#
# Vendored constituent test suite for the amalgamated mulle-core library.
#
# The sources under test/<constituent>/... are snapshots of the constituent
# projects' behavioral tests. They compile against the amalgamated library
# (the `mulle-core` target) and are driven by cmake/RunConstituentTest.cmake,
# which follows the file conventions of `mulle-sde test` / `mulle-test`.
#
# Enable with -DBUILD_TESTING=ON (opt-in) and run `ctest`. The test tree
# is deliberately not flattened: goldens named `default.*` and `.skip.*` files
# apply per directory, and test basenames are not required to be unique.
#
# See test/README.md for the conventions and test/include.h for the shared
# umbrella header.
#

if( NOT BUILD_TESTING)
   return()
endif()

enable_testing()

# __VA_OPT__ is a C23 feature. gcc and clang also accept it before C23 as an
# extension, TinyCC 0.9.27 implements it in no mode at all. Tests that need it
# carry a `.skip.no-va-opt` marker rather than being compiled into a failure
# that reads like a library bug. Probe the capability, not __STDC_VERSION__:
# gcc answers yes in -std=c11 mode.
include( CheckCSourceCompiles)
check_c_source_compiles( "#define V( ...) 0 __VA_OPT__(+ 1)\nint a = V();\nint b = V( 1);\nint main( void) { return( a + b - 1); }"
                         MULLE_CORE_TEST_HAS_VA_OPT)

set( MULLE_CORE_TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

# The tests use `#include "include.h"` (shared umbrella header). The library's
# own usage requirements put every constituent's private `include.h` on the
# include path, so the test root must win the lookup.
include_directories( BEFORE "${MULLE_CORE_TEST_ROOT}")

# libraries the constituent tests may need in addition to mulle-core
if( NOT WIN32)
   find_package( Threads)
   set( MULLE_CORE_TEST_LIBRARIES Threads::Threads ${CMAKE_DL_LIBS} m)
   if( CMAKE_SYSTEM_NAME STREQUAL "Linux")
      list( APPEND MULLE_CORE_TEST_LIBRARIES rt)
   endif()
else()
   set( MULLE_CORE_TEST_LIBRARIES)
endif()

set( MULLE_CORE_TEST_PLATFORM "${CMAKE_SYSTEM_NAME}")
string( TOLOWER "${MULLE_CORE_TEST_PLATFORM}" MULLE_CORE_TEST_PLATFORM)
set( MULLE_CORE_TEST_ARCH "${CMAKE_SYSTEM_PROCESSOR}")

file( GLOB_RECURSE MULLE_CORE_TEST_SOURCES
      RELATIVE "${MULLE_CORE_TEST_ROOT}"
      CONFIGURE_DEPENDS
      "${MULLE_CORE_TEST_ROOT}/*.c")

list( SORT MULLE_CORE_TEST_SOURCES)

foreach( TEST_SOURCE ${MULLE_CORE_TEST_SOURCES})
   get_filename_component( TEST_DIR "${TEST_SOURCE}" DIRECTORY)
   get_filename_component( TEST_NAME "${TEST_SOURCE}" NAME_WE)

   # skip the whole directory / single test, mirroring mulle-test
   set( TEST_SKIPPED FALSE)
   set( _capability_skips)
   if( NOT MULLE_CORE_TEST_HAS_VA_OPT)
      set( _capability_skips "${TEST_NAME}.skip.no-va-opt")
   endif()
   foreach( _skip
            "no-mulle-test"
            "no-mulle-test.${MULLE_CORE_TEST_PLATFORM}"
            "${TEST_NAME}.skip"
            "${TEST_NAME}.skip.${MULLE_CORE_TEST_PLATFORM}"
            "${TEST_NAME}.skip.${MULLE_CORE_TEST_PLATFORM}.${MULLE_CORE_TEST_ARCH}"
            ${_capability_skips})
      if( EXISTS "${MULLE_CORE_TEST_ROOT}/${TEST_DIR}/${_skip}")
         set( TEST_SKIPPED TRUE)
      endif()
   endforeach()
   if( TEST_SKIPPED)
      message( STATUS "Skipping test ${TEST_SOURCE} (platform ${MULLE_CORE_TEST_PLATFORM})")
      continue()
   endif()

   # compile-failure tests (mulle-test `.ccdiag`) are not runnable executables;
   # the driver does not implement compiler-diagnostic matching yet
   if( EXISTS "${MULLE_CORE_TEST_ROOT}/${TEST_DIR}/${TEST_NAME}.ccdiag")
      message( STATUS "Skipping compile-failure test ${TEST_SOURCE}")
      continue()
   endif()

   string( MAKE_C_IDENTIFIER "test_${TEST_SOURCE}" TEST_TARGET)

   add_executable( "${TEST_TARGET}" "${MULLE_CORE_TEST_ROOT}/${TEST_SOURCE}")
   target_link_libraries( "${TEST_TARGET}" PRIVATE mulle-core ${MULLE_CORE_TEST_LIBRARIES})
   set_target_properties( "${TEST_TARGET}" PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
      C_STANDARD 11
      C_STANDARD_REQUIRED ON)

   # mulle-test links tests with the executable's symbols in the dynamic
   # symbol table, so tests may use dlsym( RTLD_DEFAULT, ...).
   if( WIN32)
      target_link_options( "${TEST_TARGET}" PRIVATE "-Wl,--export-all-symbols")
   elseif( NOT APPLE)
      target_link_options( "${TEST_TARGET}" PRIVATE "-Wl,--export-dynamic")
   endif()

   add_test( NAME "${TEST_SOURCE}"
      COMMAND "${CMAKE_COMMAND}"
         "-DTEST_SOURCE=${MULLE_CORE_TEST_ROOT}/${TEST_SOURCE}"
         "-DTEST_BINARY=$<TARGET_FILE:${TEST_TARGET}>"
         "-DTEST_SOURCE_DIR=${MULLE_CORE_TEST_ROOT}/${TEST_DIR}"
         "-DTEST_OUTPUT_DIR=${CMAKE_CURRENT_BINARY_DIR}/output"
         "-DTEST_PLATFORM=${MULLE_CORE_TEST_PLATFORM}"
         "-DTEST_ARCH=${MULLE_CORE_TEST_ARCH}"
         "-DTEST_CONFIG=$<CONFIG>"
         -P "${MULLE_CORE_TEST_ROOT}/cmake/RunConstituentTest.cmake")

   set_tests_properties( "${TEST_SOURCE}" PROPERTIES
      TIMEOUT 120
      WORKING_DIRECTORY "${MULLE_CORE_TEST_ROOT}/${TEST_DIR}")

   # tests that sleep or start many threads produce different but equivalent
   # output depending on scheduling and load; keep them from running next to
   # other tests. These also spin hard (pointerset/stress.c burns minutes of
   # CPU), so they get a larger budget than the 120s default.
   if( TEST_DIR MATCHES "multithreaded" OR TEST_DIR MATCHES "stress"
       OR TEST_DIR MATCHES "relativetime" OR TEST_DIR MATCHES "sleep"
       OR TEST_DIR MATCHES "bench" OR TEST_NAME MATCHES "stress")
      set_tests_properties( "${TEST_SOURCE}" PROPERTIES
         RUN_SERIAL TRUE
         TIMEOUT 300)
   endif()
endforeach()