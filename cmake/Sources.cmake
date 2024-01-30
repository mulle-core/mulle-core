### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# The following includes include definitions generated
# during `mulle-sde reflect`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Sources OPTIONAL)

# === MULLE-SDE END ===
#
#
# If you don't like the "automatic" way of generating _Sources
#
# MULLE_MATCH_TO_CMAKE_SOURCES_FILE="DISABLE" # or NONE
#

# add dlfcn-win32, if needed
#
if( WIN32)
   LIST( APPEND SOURCES "dlfcn-win32/dlfcn.c")
endif()

