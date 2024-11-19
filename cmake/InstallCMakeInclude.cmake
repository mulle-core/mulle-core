### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __INSTALL_CMAKE_INCLUDE__CMAKE__)
   set( __INSTALL_CMAKE_INCLUDE__CMAKE__ ON)


   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # convenient way to figure out, if we are added as a subdirectory
   # if we aren't, there is no point in doing this or ?
   #
   if( NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)

      list( APPEND INSTALL_TMP_HEADERS ${INSTALL_PUBLIC_HEADERS} ${INSTALL_PRIVATE_HEADERS})

      #
      # we copy our headers into a build directory, so we don't have to gum
      # up our project with a root /include, that duplicates the headers (as
      # we did in the code below)
      #
      if( INSTALL_TMP_HEADERS)
         add_custom_target( ${LIBRARY_NAME}-headers ALL
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             SOURCES ${INSTALL_TMP_HEADERS}
         )

         foreach( HEADER ${INSTALL_TMP_HEADERS})
             # Get the relative path of the header file
             string( REGEX REPLACE "^[^/]+/(.*)" "\\1" HEADER_REL_PATH ${HEADER})

             # Get the directory part of the relative path
             get_filename_component( HEADER_REL_DIR ${HEADER_REL_PATH} DIRECTORY)

             # Get just the filename part
             get_filename_component( HEADER_FILENAME ${HEADER_REL_PATH} NAME)

             # Check if its an amalgamation header was directly in src/
             if( HEADER_REL_DIR MATCHES "^mulle-")
                 set( DEST_DIR "${CMAKE_BINARY_DIR}/include/${HEADER_REL_DIR}")
             else()
                 set( DEST_DIR "${CMAKE_BINARY_DIR}/include/${LIBRARY_NAME}")
             endif()

             # Add a custom command for each header
             add_custom_command(
                 TARGET ${LIBRARY_NAME}-headers
                 COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_DIR}"
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${HEADER} "${DEST_DIR}/${HEADER_FILENAME}"
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 DEPENDS ${HEADER}
             )
         endforeach()

         add_dependencies( ${LIBRARY_NAME} ${LIBRARY_NAME}-headers)

         target_include_directories(${LIBRARY_NAME} INTERFACE
             $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
             $<INSTALL_INTERFACE:include>
         )
      endif()

    endif()

#   function( unprotect_cmake_include_directory dir)
#       if( NOT EXISTS "${dir}")
#          file( MAKE_DIRECTORY "${dir}")
#       else()
#          if( WIN32)
#              execute_process(COMMAND attrib -R "${dir}\\*.*" /S)
#          else()
#              execute_process(COMMAND chmod -R ugo+w "${dir}")
#          endif()
#       endif()
#   endfunction()
#
#   function( protect_cmake_include_directory dir)
#       if( WIN32)
#           execute_process(COMMAND attrib +R "${dir}\\*.*" /S)
#       else()
#           execute_process(COMMAND chmod -R ugo-w "${dir}")
#       endif()
#   endfunction()
#
#   #
#   # This can run at anytime really, targets must not rely on these headers
#   #
#   if( INSTALL_CMAKE_INCLUDE_ENABLED)
#      set( SOURCE_ROOT_INCLUDE_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/include")
#      set( SOURCE_PUBLIC_INCLUDE_DIR  "${SOURCE_ROOT_INCLUDE_DIR}/${PROJECT_NAME}")
#      set( SOURCE_PRIVATE_INCLUDE_DIR "${SOURCE_ROOT_INCLUDE_DIR}/${PROJECT_NAME}")
##
## noone's gonna look there anyway
##      set( SOURCE_CMAKE_INCLUDE_DIR   "${SOURCE_ROOT_INCLUDE_DIR}/${PROJECT_NAME}/cmake")
#
#      unprotect_cmake_include_directory( "${SOURCE_ROOT_INCLUDE_DIR}")
#
#      file( MAKE_DIRECTORY "${SOURCE_PUBLIC_INCLUDE_DIR}")
#      file( MAKE_DIRECTORY "${SOURCE_PRIVATE_INCLUDE_DIR}")
##      file( MAKE_DIRECTORY "${SOURCE_CMAKE_INCLUDE_DIR}")
#
#      # Copy public headers
#      foreach( HEADER ${INSTALL_PUBLIC_HEADERS})
#          file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}"
#               DESTINATION "${SOURCE_PUBLIC_INCLUDE_DIR}")
#      endforeach()
#
#      # Copy private headers
#      foreach( HEADER ${INSTALL_PRIVATE_HEADERS})
#          file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}"
#               DESTINATION "${SOURCE_PRIVATE_INCLUDE_DIR}")
#      endforeach()
##
##      # Copy cmake headers
##      foreach( HEADER ${INSTALL_CMAKE_INCLUDES})
##          file( COPY "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}"
##                DESTINATION "${SOURCE_CMAKE_INCLUDE_DIR}")
##      endforeach()
#
#      protect_cmake_include_directory( "${SOURCE_ROOT_INCLUDE_DIR}")
#   endif()

endif()
