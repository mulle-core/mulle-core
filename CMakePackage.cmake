#
# CPack and project specific stuff
#
######

set( CPACK_PACKAGE_NAME "${PROJECT_NAME}")
execute_process( COMMAND mulle-project-version -s
                 OUTPUT_VARIABLE PROJECT_VERSION
                 WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
message( STATUS "PROJECT_VERSION=${PROJECT_VERSION}")

set( CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set( CPACK_PACKAGE_CONTACT "nat@mulle-kybernetik.de")
set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "🌋 Almagamated library of mulle-core + mulle-concurrent + mulle-c")
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set( CPACK_STRIP_FILES false)


set( CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/mulle-core/${PROJECT_NAME}")
set( CPACK_DEBIAN_PACKAGE_DEPENDS)
set( CPACK_RPM_PACKAGE_VENDOR "mulle-core")
