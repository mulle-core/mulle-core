# Toolchain file for cross-compiling to Android via the Android NDK.
#
# Configure with:
#   ANDROID_NDK=/path/to/ndk mulle-sde craft --platform android
#
# ANDROID_ABI and ANDROID_PLATFORM may be supplied through the environment or
# as CMake cache variables. Defaults target current 64-bit Android devices.

set(CMAKE_SYSTEM_NAME Android)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

if(NOT DEFINED ANDROID_NDK OR "${ANDROID_NDK}" STREQUAL "")
   if(DEFINED ENV{ANDROID_NDK})
      set(ANDROID_NDK "$ENV{ANDROID_NDK}" CACHE PATH "Android NDK root")
   elseif(DEFINED ENV{MULLE_CRAFT_CROSS_COMPILER_ROOT__ANDROID})
      set(ANDROID_NDK "$ENV{MULLE_CRAFT_CROSS_COMPILER_ROOT__ANDROID}" CACHE PATH "Android NDK root")
   else()
      message(FATAL_ERROR "ANDROID_NDK is required for the Android toolchain")
   endif()
endif()

if(NOT EXISTS "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
   message(FATAL_ERROR "Invalid Android NDK: ${ANDROID_NDK}")
endif()

set(ANDROID_ABI "${ANDROID_ABI}" CACHE STRING "Android ABI")
if("${ANDROID_ABI}" STREQUAL "")
   set(ANDROID_ABI arm64-v8a CACHE STRING "Android ABI" FORCE)
endif()

set(ANDROID_PLATFORM "${ANDROID_PLATFORM}" CACHE STRING "Android API level")
if("${ANDROID_PLATFORM}" STREQUAL "")
   set(ANDROID_PLATFORM android-24 CACHE STRING "Android API level" FORCE)
endif()

message(STATUS "Using Android NDK: ${ANDROID_NDK}")
message(STATUS "Android ABI: ${ANDROID_ABI}")
message(STATUS "Android platform: ${ANDROID_PLATFORM}")

include("${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
