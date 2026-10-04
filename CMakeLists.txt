#
# You rarely need to edit this file. To add or remove files in your
# project edit the "mulle-core" folders contents and run `mulle-sde reflect`
#
#
# A convenient way to add a new source file is:
#    mulle-sde add mulle-core/foo.c
#
cmake_minimum_required( VERSION 3.19...99.99) # for REAL_PATH

project( mulle-core VERSION 0.10.0 LANGUAGES C)

#
# The vendored constituent tests in test/ are opt-in. mulle-core is normally
# crafted as a dependency of other mulle-sde projects, so building (let alone
# running) the ~1800 constituent test executables by default is unwanted.
# `include(CTest)` would declare BUILD_TESTING with a default of ON, so declare
# it here as OFF instead. The tests are registered near the end of this file.
#
# A plain cmake checkout opts in with -DBUILD_TESTING=ON and runs `ctest`.
# `mulle-sde test` is unaffected: test/ is its own mulle-sde project and drives
# the same sources through mulle-test, not through this build.
#
option( BUILD_TESTING "Build the vendored constituent test suite (ctest)" OFF)

# Common compiler flags for all platforms and compilers
set(COMMON_C_FLAGS "")

### AI generated

# Add suppression flags based on compiler
if(CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "GNU")
  # GCC and Clang suppression flags
  add_compile_options(
    -Wno-unknown-pragmas
  )
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
  # MSVC suppression flags
  add_compile_options(
    /wd4068  # Unknown pragma warning
  )
  # MSVC secure function warnings suppression
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Your existing CMake configuration continues here...


###


set( MULLE_PROJECT_TYPE "Library")

### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake/reflect")
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include( Environment)

include_directories( src)

include( Files)

# Don't have this as default
# include( IDESupport OPTIONAL)

unset( SUB_PROJECT_NAMES)
file( GLOB SUB_PROJECT_PATHS src/mulle-*)

foreach( SUB_PROJECT_PATH ${SUB_PROJECT_PATHS})
   if( IS_DIRECTORY "${SUB_PROJECT_PATH}")
      get_filename_component( SUB_PROJECT_NAME "${SUB_PROJECT_PATH}" NAME)
      list( APPEND SUB_PROJECT_NAMES "${SUB_PROJECT_NAME}")
      message( STATUS "SUB_PROJECT_NAME = ${SUB_PROJECT_NAME}")
   endif()
endforeach()
set( SUB_PROJECT_NAMES ${SUB_PROJECT_NAMES} CACHE INTERNAL "sub-project names")


### Library

if( HEADER_PHASE)

   #
   # need to install like the embedded subprojects would
   #
   unset(REMAINING_HEADER)
   foreach( TMP_HEADER ${PROJECT_INSTALLABLE_HEADERS})
      get_filename_component( TMP_PROJECT_NAME "${TMP_HEADER}" DIRECTORY)
      get_filename_component( TMP_PROJECT_NAME "${TMP_PROJECT_NAME}" NAME)
      list( FIND SUB_PROJECT_NAMES "${TMP_PROJECT_NAME}" TMP_INDEX)
      # can't stay as a symlink, otherwise symlinks get installed which
      # fail, as they are relative
      file( REAL_PATH "${TMP_HEADER}" TMP_HEADER)
      if( NOT( "${TMP_INDEX}" EQUAL -1))
         install( FILES ${TMP_HEADER} DESTINATION "include/${TMP_PROJECT_NAME}")
      else()
         list( APPEND REMAINING_HEADER "${TMP_HEADER}")
      endif()
   endforeach()

   install( FILES ${REMAINING_HEADER} DESTINATION "include/${PROJECT_NAME}")
   # install( FILES ${INSTALL_CMAKE_INCLUDES} DESTINATION "include/${PROJECT_NAME}/cmake")

   # manually maintained!
   install( FILES ${MINTOMIC_PUBLIC_HEADERS}  DESTINATION "include/mintomic")
   # NOTE: it's really private subdir!
   install( FILES ${MINTOMIC_PRIVATE_HEADERS} DESTINATION "include/mintomic/private")

   # manually maintained!
   install( FILES ${DLFCN_WIN32_PUBLIC_HEADERS}  DESTINATION "include")

   # short cut out
   if( NOT COMPILE_PHASE AND NOT LINK_PHASE)
      return()
   endif()
endif()

if( LINK_PHASE)
   include( Dependencies OPTIONAL)
endif()

include( Library)

# because clib flattens the headers, put we need "private" in the local
# build, we gotta reorganize
if( MINTOMIC_PRIVATE_HEADERS_COPIES)
   add_dependencies( ${LIBRARY_COMPILE_TARGET} RearrangeMintomicPrivateHeaders)
endif()

# use LIBRARY_COMPILE_TARGET for compiler options
target_compile_definitions( "${LIBRARY_COMPILE_TARGET}" PUBLIC)
target_compile_options( "${LIBRARY_COMPILE_TARGET}" PUBLIC)
if( NOT MSVC)
   target_compile_options( "${LIBRARY_COMPILE_TARGET}" PUBLIC -Wunused)
endif()

#
# The sub-projects above are amalgamated: their sources are compiled straight
# into this library. Each one marks its exported symbols with
# <SUB_PROJECT>_GLOBAL / <SUB_PROJECT>_GLOBAL_VAR, and those expand to
# dllexport only while the sub-project's own <SUB_PROJECT>_BUILD is defined.
#
# A standalone build of a sub-project defines it (Library.cmake derives it from
# the project name), but an amalgamation compiles the vendored sources without
# it, so their headers declared the globals as dllimport inside the very
# library that defines them. On Windows such a reference goes through an import
# thunk, the DLL ends up importing symbols it also defines, and llvm-mingw
# refuses to export those: they vanish from both the export table and the
# .dll.a, and consumers fail to link with
#
#    undefined symbol: __declspec(dllimport) NSFileTypeDirectory
#
# Some sub-projects already cover their amalgamation by naming it in their
# include.h guard, which is why this went unnoticed. Deriving the identifier
# here with the same snakeCaseString() rule the sub-project's own
# .mulle/etc/env/environment-project.sh uses fixes the rest without having to
# edit every sub-project or maintain a list.
#
if( LIBRARY_COMPILE_TARGET)
   foreach( SUB_PROJECT_NAME ${SUB_PROJECT_NAMES})
      snakeCaseString( "${SUB_PROJECT_NAME}" SUB_PROJECT_IDENTIFIER)
      string( TOUPPER "${SUB_PROJECT_IDENTIFIER}" SUB_PROJECT_UPCASE_IDENTIFIER)
      if( NOT "${SUB_PROJECT_UPCASE_IDENTIFIER}" STREQUAL "${LIBRARY_UPCASE_IDENTIFIER}")
         message( STATUS "Amalgamating sub-project ${SUB_PROJECT_NAME} as ${SUB_PROJECT_UPCASE_IDENTIFIER}_BUILD")
         target_compile_definitions( "${LIBRARY_COMPILE_TARGET}" PRIVATE "${SUB_PROJECT_UPCASE_IDENTIFIER}_BUILD")
         if( LIBRARY_STAGE2_TARGET)
            target_compile_definitions( "${LIBRARY_STAGE2_TARGET}" PRIVATE "${SUB_PROJECT_UPCASE_IDENTIFIER}_BUILD")
         endif()
      endif()
   endforeach()
   unset( SUB_PROJECT_IDENTIFIER)
   unset( SUB_PROJECT_UPCASE_IDENTIFIER)
endif()

# use LIBRARY_LINK_TARGET for linker options
if( LINK_PHASE)
   target_link_options( "${LIBRARY_LINK_TARGET}" PUBLIC)

   #
   # CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS (DLL_EXPORT_ALL) is a no-op for the
   # llvm-mingw toolchain: it is only implemented for MS linkers. The MinGW
   # linker exports every defined symbol by default, but it stops doing so as
   # soon as anything is explicitly dllexport'ed, which the block above now
   # does. Ask for the full export surface explicitly, so that symbols which
   # are still declared bare keep their export.
   #
   if( (WIN32 OR MINGW) AND NOT MSVC AND BUILD_SHARED_LIBS)
      target_link_options( "${LIBRARY_LINK_TARGET}" PRIVATE "-Wl,--export-all-symbols")
   endif()
endif()

# Install the API TOC of each constituent library.
#
# clib flattens paths to their basename, so a constituent's
# "asset/dox/api/toc/index.md" arrives here as "src/<name>/index.md" and the
# legacy "asset/dox/TOC.md" arrives as "src/<name>/TOC.md". Accept both, but
# prefer "index.md".
#
# Always install as "share/<name>/dox/api/toc/index.md", which is where
# `mulle-sde api` looks. Note that DESTINATION is a directory, the filename is
# set with RENAME.
#
if( LINK_PHASE)
   foreach( SUB_PROJECT_NAME ${SUB_PROJECT_NAMES})
      set( SUB_ASSETS_TOC "${CMAKE_CURRENT_SOURCE_DIR}/mulle-core/${SUB_PROJECT_NAME}/index.md")
      if( NOT EXISTS "${SUB_ASSETS_TOC}")
         set( SUB_ASSETS_TOC "${CMAKE_CURRENT_SOURCE_DIR}/mulle-core/${SUB_PROJECT_NAME}/TOC.md")
      endif()
      if( EXISTS "${SUB_ASSETS_TOC}")
         message( STATUS "Installing API TOC from ${SUB_PROJECT_NAME}")
         install( FILES "${SUB_ASSETS_TOC}"
                  DESTINATION "share/${SUB_PROJECT_NAME}/dox/api/toc"
                  RENAME "index.md"
                  PERMISSIONS OWNER_WRITE OWNER_READ
                              GROUP_READ
                              WORLD_READ)
      endif()
   endforeach()
endif()

include( InstallLibrary)

if( LINK_PHASE)
   include( AddSubdirectorySupport)
   include( FindPackageSupport)
endif()



include( FinalOutput OPTIONAL)


# Vendored constituent tests, runnable with `ctest`. They compile against the
# amalgamated `mulle-core` target, so they can only be added once it exists.
#
# Never register them as an add_subdirectory() subproject: the parent project's
# ctest would then run our tests, and a parent that sets BUILD_TESTING=ON would
# otherwise drag the whole suite into a consumer's build.
if( BUILD_TESTING AND NOT IS_CMAKE_SUBPROJECT AND LINK_PHASE
    AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test/CMakeLists.txt")
   enable_testing()
   add_subdirectory( test)
endif()


######
#
# must be after CPACK_PACKAGE_VERSION
include( CMakePackage.cmake) # need extension
include( CPack)
