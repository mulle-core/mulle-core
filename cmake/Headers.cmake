### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

option( RESOLVE_INSTALLABLE_HEADER_SYMLINKS "Resolve PROJECT_INSTALLABLE_HEADERS symlinks" OFF)
message( STATUS "RESOLVE_INSTALLABLE_HEADER_SYMLINKS is ${RESOLVE_INSTALLABLE_HEADER_SYMLINKS}")


#
# The following includes include definitions generated
# during `mulle-sde reflect`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Headers OPTIONAL)

# === MULLE-SDE END ===
#

#
# If you don't like the "automatic" way of generating _Headers
#
# MULLE_MATCH_TO_CMAKE_HEADERS_FILE="DISABLE" # or NONE
#


function( ResolveFileSymlinksIfNeeded listname outputname)
   unset( list)
   if( RESOLVE_INSTALLABLE_HEADER_SYMLINKS)
      foreach( TMP_HEADER ${${listname}})
         file( REAL_PATH "${TMP_HEADER}" TMP_RESOLVED_HEADER)
         list( APPEND list "${TMP_RESOLVED_HEADER}")
      endforeach()
      message( STATUS "Resolved symlinks of ${outputname}=${list}")
   else()
      set( list ${${listname}})
   endif()
   set( ${outputname} ${list} PARENT_SCOPE)
endfunction()


#
# PROJECT_INSTALLABLE_HEADERS
# INSTALL_PUBLIC_HEADERS
# INSTALL_PRIVATE_HEADERS
#

# keep headers to install separate to make last minute changes
set( TMP_HEADERS ${PUBLIC_HEADERS}
                 ${PUBLIC_GENERIC_HEADERS}
                 ${PUBLIC_GENERATED_HEADERS}
)
ResolveFileSymlinksIfNeeded( TMP_HEADERS INSTALL_PUBLIC_HEADERS)

set( TMP_HEADERS ${PRIVATE_HEADERS})
if( TMP_HEADERS)
   list( REMOVE_ITEM TMP_HEADERS "include-private.h")
endif()
ResolveFileSymlinksIfNeeded( TMP_HEADERS INSTALL_PRIVATE_HEADERS)

# let's not cache headers, as they are bound to fluctuate. when we change
# dependencies we expect a clean
set( PROJECT_INSTALLABLE_HEADERS
   ${INSTALL_PUBLIC_HEADERS}
   ${INSTALL_PRIVATE_HEADERS}
)

#
# You can put more source and resource file definitions here.
#
#

set( MINTOMIC_PUBLIC_HEADERS
mintomic/core.h
mintomic/mintomic.h
mintomic/platform_detect.h)


set( MINTOMIC_PRIVATE_HEADERS
mintomic/core_gcc.h
mintomic/core_msvc.h
mintomic/mintomic_gcc_arm.h
mintomic/mintomic_gcc_x86-64.h
mintomic/mintomic_msvc.h
mintomic/mintomic_stdint.h
)

include_directories(
AFTER SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}"
)

set( MINTOMIC_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mintomic/private")
unset( MINTOMIC_PRIVATE_HEADERS_COPIES)

# Create the destination directory if it doesn't exist
if( NOT EXISTS ${MINTOMIC_PRIVATE_DIR})
    file( MAKE_DIRECTORY ${MINTOMIC_PRIVATE_DIR})

   # Create a custom target for each file
   foreach( TMP_FILE ${MINTOMIC_PRIVATE_HEADERS})
       get_filename_component( TMP_FILENAME ${TMP_FILE} NAME)
       set( TMP_DESTINATION_PATH "${MINTOMIC_PRIVATE_DIR}/${TMP_FILENAME}")
       add_custom_command(
           OUTPUT ${TMP_DESTINATION_PATH}
           COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${TMP_FILE} ${TMP_DESTINATION_PATH}
           COMMENT "Copying ${TMP_FILE} to private headers..."
       )
       # Add a custom target for this file
       list( APPEND MINTOMIC_PRIVATE_HEADERS_COPIES ${TMP_DESTINATION_PATH})
    endforeach()

    add_custom_target( RearrangeMintomicPrivateHeaders ALL DEPENDS ${MINTOMIC_PRIVATE_HEADERS_COPIES})
endif()


#
# add dlfcn-win32, if needed
#
# if( WIN32)
#    set( DLFCN_WIN32_PUBLIC_HEADERS
#       dlfcn-win32/dlfcn.h
#    )
# 
#    include_directories(
#       AFTER SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/dlfcn-win32"
#    )
# endif()

