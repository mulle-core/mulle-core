#
# You rarely need to edit this file. To add or remove files in your
# project edit the "src" folders contents and run `mulle-sde reflect`
#
#
# A convenient way to add a new source file is:
#    mulle-sde add src/foo.c
#
cmake_minimum_required( VERSION 3.19...99.99) # for REAL_PATH

project( mulle-core VERSION 0.7.0 LANGUAGES C)

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

# use LIBRARY_LINK_TARGET for linker options
if( LINK_PHASE)
   target_link_options( "${LIBRARY_LINK_TARGET}" PUBLIC)
endif()

include( InstallLibrary)

if( LINK_PHASE)
   include( InstallCMakeInclude)
   include( InstallCMakePackage)
endif()

include( FinalOutput OPTIONAL)


######
#
# must be after CPACK_PACKAGE_VERSION
include( CMakePackage.cmake) # need extension
include( CPack)
