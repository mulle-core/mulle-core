# Run a vendored constituent test against the amalgamated mulle-core library.
#
# This implements the subset of mulle-test semantics that the vendored tests
# rely on. It is intentionally a CMake script so that `ctest` works on every
# platform CMake supports, without requiring mulle-sde or a POSIX shell.
#
# Conventions (a test is <name>.c in <dir>):
#
#   <name>.stdout      exact expected standard output (default: not checked)
#   <name>.stderr      expected standard error, whitespace-insensitive
#   <name>.errors      each non-comment line is an `grep -E` pattern; its
#                      presence means a non-zero exit is expected
#   <name>.stdin       fed to the test's standard input
#   <name>.args        arguments (one per line, `#` comments allowed)
#   <name>.environment `export KEY=VALUE` / `KEY=VALUE` lines
#   <name>.skip*       handled at configure time, not here
#
# Any of these may be written as `default.<ext>` to be shared by all tests in
# the directory. Platform and architecture variants are looked up first, e.g.
# `<name>.stdout.windows` or `<name>.stdout.<platform>.<arch>`.
#
# Whitespace-only differences in stdout are a warning, not a failure, matching
# `mulle-test`. Different stderr is always a failure.

foreach( _required TEST_SOURCE TEST_BINARY TEST_SOURCE_DIR TEST_OUTPUT_DIR)
   if( NOT DEFINED ${_required})
      message( FATAL_ERROR "RunConstituentTest: ${_required} must be defined")
   endif()
endforeach()

if( NOT DEFINED TEST_PLATFORM OR "${TEST_PLATFORM}" STREQUAL "")
   set( TEST_PLATFORM "linux")
endif()
if( NOT DEFINED TEST_ARCH OR "${TEST_ARCH}" STREQUAL "")
   set( TEST_ARCH "unknown")
endif()
if( NOT DEFINED TEST_CONFIG OR "${TEST_CONFIG}" STREQUAL "")
   set( TEST_CONFIG "Debug")
endif()

get_filename_component( TEST_NAME "${TEST_SOURCE}" NAME_WE)

# _test_resolve( <outvar> <varname> <fallback> )
#
# Searches for <name>.<varname>[.<variant>] and default.<varname>[.<variant>]
# in the test's directory and stores the first hit, or <fallback>.
function( _test_resolve outvar varname fallback)
   # NOTE: a trailing empty element would be dropped by foreach(), so the
   # unsuffixed name is tried explicitly after the suffixed variants.
   set( _suffixes
      "${TEST_PLATFORM}-${TEST_CONFIG}"
      "${TEST_PLATFORM}"
      "${TEST_CONFIG}"
      "${TEST_ARCH}")
   foreach( _first "${TEST_NAME}" "default")
      foreach( _suffix ${_suffixes})
         set( _candidates
            "${_first}.${varname}.${_suffix}.${TEST_ARCH}"
            "${_first}.${varname}.${_suffix}")
         foreach( _candidate ${_candidates})
            if( EXISTS "${TEST_SOURCE_DIR}/${_candidate}")
               set( ${outvar} "${TEST_SOURCE_DIR}/${_candidate}" PARENT_SCOPE)
               return()
            endif()
         endforeach()
      endforeach()
      if( EXISTS "${TEST_SOURCE_DIR}/${_first}.${varname}")
         set( ${outvar} "${TEST_SOURCE_DIR}/${_first}.${varname}" PARENT_SCOPE)
         return()
      endif()
   endforeach()
   set( ${outvar} "${fallback}" PARENT_SCOPE)
endfunction()

_test_resolve( TEST_STDIN  "stdin"  "")
_test_resolve( TEST_STDOUT "stdout" "")
_test_resolve( TEST_STDERR "stderr" "")
_test_resolve( TEST_ERRORS "errors" "")
_test_resolve( TEST_ARGS_FILE "args" "")
_test_resolve( TEST_CAT "cat" "")
_test_resolve( TEST_DIFF "diff" "")

file( MAKE_DIRECTORY "${TEST_OUTPUT_DIR}")

if( NOT TEST_STDIN)
   set( TEST_STDIN "${TEST_OUTPUT_DIR}/empty.stdin")
   file( WRITE "${TEST_STDIN}" "")
endif()

# arguments
set( _test_args)
if( TEST_ARGS_FILE)
   file( STRINGS "${TEST_ARGS_FILE}" _arg_lines)
   foreach( _line ${_arg_lines})
      if( NOT "${_line}" MATCHES "^[ \t]*#")
         separate_arguments( _line_args UNIX_COMMAND "${_line}")
         list( APPEND _test_args ${_line_args})
      endif()
   endforeach()
endif()

# environment
_test_resolve( TEST_ENVIRONMENT_FILE "environment" "")
if( TEST_ENVIRONMENT_FILE)
   file( STRINGS "${TEST_ENVIRONMENT_FILE}" _env_lines)
   foreach( _line ${_env_lines})
      if( "${_line}" MATCHES "^[ \t]*(export[ \t]+)?([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*(.*)$")
         set( _key "${CMAKE_MATCH_2}")
         set( _value "${CMAKE_MATCH_3}")
         string( REGEX REPLACE "^\"(.*)\"$" "\\1" _value "${_value}")
         string( REGEX REPLACE "^'(.*)'$" "\\1" _value "${_value}")
         set( ENV{${_key}} "${_value}")
      endif()
   endforeach()
endif()

# A unique identifier per test: test basenames are only unique per directory
# (e.g. several `search.c`), so output and work paths must not use TEST_NAME.
string( MAKE_C_IDENTIFIER "${TEST_SOURCE}" _test_ident)

set( _actual_stdout "${TEST_OUTPUT_DIR}/${_test_ident}.actual.stdout")
set( _actual_stderr "${TEST_OUTPUT_DIR}/${_test_ident}.actual.stderr")

# Run every test in a private copy of its directory. Several tests create,
# chmod or unlink files next to themselves (the mmap persistence tests, for
# example), and they must not mutate the vendored snapshot in the source tree.
set( _workdir "${TEST_OUTPUT_DIR}/work/${_test_ident}")
file( REMOVE_RECURSE "${_workdir}")
file( MAKE_DIRECTORY "${_workdir}")
file( COPY "${TEST_SOURCE_DIR}/" DESTINATION "${_workdir}")

execute_process(
   COMMAND "${TEST_BINARY}" ${_test_args}
   WORKING_DIRECTORY "${_workdir}"
   INPUT_FILE "${TEST_STDIN}"
   OUTPUT_FILE "${_actual_stdout}"
   ERROR_FILE "${_actual_stderr}"
   RESULT_VARIABLE _rc
   TIMEOUT 120 )

if( NOT "${_rc}" STREQUAL "0")
   set( _failed_rv "${_rc}" )
else()
   set( _failed_rv "" )
endif()

# exit code / errors file handling
if( NOT "${_failed_rv}" STREQUAL "" AND NOT TEST_ERRORS)
   file( READ "${_actual_stdout}" _o)
   file( READ "${_actual_stderr}" _e)
   message( "stdout:\n${_o}")
   message( "stderr:\n${_e}")
   message( FATAL_ERROR "Test ${TEST_NAME} failed (exit ${_failed_rv})")
endif()

if( "${_failed_rv}" STREQUAL "" AND TEST_ERRORS)
   message( FATAL_ERROR "Test ${TEST_NAME} succeeded but ${TEST_ERRORS} expects a failure")
endif()

if( NOT "${_failed_rv}" STREQUAL "" AND TEST_ERRORS)
   file( READ "${_actual_stderr}" _actual_errors)
   file( STRINGS "${TEST_ERRORS}" _error_patterns)
   foreach( _pattern ${_error_patterns})
      if( NOT "${_pattern}" MATCHES "^[ \t]*#")
         if( NOT "${_actual_errors}" MATCHES "${_pattern}")
            message( "stderr:\n${_actual_errors}")
            message( FATAL_ERROR "Test ${TEST_NAME}: stderr did not match \"${_pattern}\"")
         endif()
      endif()
   endforeach()
   return()
endif()

# stdout: exact, with whitespace-only difference downgraded to a warning.
# A <name>.cat script massages the actual output; a <name>.diff script decides
# equality itself (this is how multithreaded tests normalize their ordering).
if( TEST_STDOUT)
   if( TEST_CAT)
      set( _massaged "${TEST_OUTPUT_DIR}/${_test_ident}.massaged.stdout")
      execute_process(
         COMMAND "${TEST_CAT}" "${_actual_stdout}"
         OUTPUT_FILE "${_massaged}"
         RESULT_VARIABLE _cat_rc)
      if( "${_cat_rc}" STREQUAL "0")
         set( _actual_stdout "${_massaged}")
      endif()
   endif()

   if( TEST_DIFF)
      execute_process(
         COMMAND "${TEST_DIFF}" "${TEST_STDOUT}" "${_actual_stdout}"
         RESULT_VARIABLE _diff_rc
         OUTPUT_VARIABLE _diff_out
         ERROR_VARIABLE _diff_err)
      if( NOT "${_diff_rc}" STREQUAL "0")
         message( "diff:\n${_diff_out}${_diff_err}")
         message( FATAL_ERROR "Test ${TEST_NAME} diff script reported differences")
      endif()
   else()
      file( READ "${TEST_STDOUT}" _expected_stdout)
      file( READ "${_actual_stdout}" _actual_stdout_text)
      if( NOT "${_actual_stdout_text}" STREQUAL "${_expected_stdout}")
         string( REGEX REPLACE "[ \t\r\n]+" "" _a "${_actual_stdout_text}")
         string( REGEX REPLACE "[ \t\r\n]+" "" _e "${_expected_stdout}")
         if( "${_a}" STREQUAL "${_e}")
            message( "WARNING: ${TEST_NAME} stdout differs only in whitespace from ${TEST_STDOUT}")
         else()
            message( "actual stdout:\n${_actual_stdout_text}")
            message( "expected stdout (${TEST_STDOUT}):\n${_expected_stdout}")
            message( FATAL_ERROR "Test ${TEST_NAME} produced different stdout")
         endif()
      endif()
   endif()
endif()

# stderr: whitespace-insensitive
if( TEST_STDERR)
   file( READ "${TEST_STDERR}" _expected_stderr)
   file( READ "${_actual_stderr}" _actual_stderr_text)
   string( REGEX REPLACE "[ \t\r\n]+" "" _a "${_actual_stderr_text}")
   string( REGEX REPLACE "[ \t\r\n]+" "" _e "${_expected_stderr}")
   if( NOT "${_a}" STREQUAL "${_e}")
      message( "actual stderr:\n${_actual_stderr_text}")
      message( "expected stderr (${TEST_STDERR}):\n${_expected_stderr}")
      message( FATAL_ERROR "Test ${TEST_NAME} produced different stderr")
   endif()
endif()