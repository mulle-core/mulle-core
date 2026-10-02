# Toolchain file for cross-compiling to FreeBSD with clang + lld.
#
# Expects a local FreeBSD development root (created by
# freebsd-devroot.sh) containing:
#
#    <root>/VERSION
#    <root>/sysroot/                  (FreeBSD base.txz)
#    <root>/bin/freebsd-clang         (plain C)
#    <root>/bin/freebsd-mulle-clang   (Objective-C / mulle-objc)
#
# The root is taken from MULLE_CROSS_COMPILER_ROOT, which mulle-make exports
# from MULLE_CRAFT_CROSS_COMPILER_ROOT__FREEBSD (set with
# `mulle-sde platform set freebsd root <dir>`).

set(CMAKE_SYSTEM_NAME FreeBSD)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

if(NOT DEFINED FREEBSD_ROOT)
    if(DEFINED ENV{MULLE_CROSS_COMPILER_ROOT})
        set(FREEBSD_ROOT "$ENV{MULLE_CROSS_COMPILER_ROOT}")
    elseif(DEFINED ENV{FREEBSD_DEV_ROOT})
        set(FREEBSD_ROOT "$ENV{FREEBSD_DEV_ROOT}")
    else()
        set(FREEBSD_ROOT "$ENV{HOME}/.mulle/freebsd/15.1-RELEASE")
    endif()
endif()

message(STATUS "Using FreeBSD root: ${FREEBSD_ROOT}")

if(EXISTS "${FREEBSD_ROOT}/VERSION")
    file(READ "${FREEBSD_ROOT}/VERSION" FREEBSD_VERSION)
    string(STRIP "${FREEBSD_VERSION}" FREEBSD_VERSION)
else()
    set(FREEBSD_VERSION "15.1-RELEASE")
endif()

string(REGEX REPLACE "-RELEASE$" "" FREEBSD_VERSION_NUMBER "${FREEBSD_VERSION}")
set(FREEBSD_TRIPLE "x86_64-unknown-freebsd${FREEBSD_VERSION_NUMBER}")
set(FREEBSD_SYSROOT "${FREEBSD_ROOT}/sysroot")

if(NOT EXISTS "${FREEBSD_SYSROOT}")
    message(FATAL_ERROR
        "No FreeBSD sysroot at \"${FREEBSD_SYSROOT}\". "
        "Set MULLE_CRAFT_CROSS_COMPILER_ROOT__FREEBSD to a root created by freebsd-devroot.sh")
endif()

# Objective-C (mulle-objc) must compile with mulle-clang, which carries the
# mulle-objc runtime. Plain C uses stock clang.
if("$ENV{PROJECT_DIALECT}" STREQUAL "objc" OR "$ENV{PROJECT_DIALECT}" STREQUAL "objcpp")
    set(FREEBSD_COMPILER_SUFFIX "mulle-clang")
    set(FREEBSD_MULLE_OBJC ON)
else()
    set(FREEBSD_COMPILER_SUFFIX "clang")
    set(FREEBSD_MULLE_OBJC OFF)
endif()

if(EXISTS "${FREEBSD_ROOT}/bin/freebsd-${FREEBSD_COMPILER_SUFFIX}")
    set(CMAKE_C_COMPILER   "${FREEBSD_ROOT}/bin/freebsd-${FREEBSD_COMPILER_SUFFIX}"   CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "${FREEBSD_ROOT}/bin/freebsd-${FREEBSD_COMPILER_SUFFIX}++" CACHE FILEPATH "CXX compiler" FORCE)
else()
    set(CMAKE_C_COMPILER   clang   CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER clang++ CACHE FILEPATH "CXX compiler" FORCE)
endif()

set(CMAKE_C_COMPILER_TARGET   "${FREEBSD_TRIPLE}")
set(CMAKE_CXX_COMPILER_TARGET "${FREEBSD_TRIPLE}")

# -fuse-ld=lld belongs on the link line only; putting it in the compile flags
# just produces "argument unused during compilation" warnings.
set(CMAKE_C_FLAGS_INIT   "--sysroot=${FREEBSD_SYSROOT}")
set(CMAKE_CXX_FLAGS_INIT "--sysroot=${FREEBSD_SYSROOT}")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "--sysroot=${FREEBSD_SYSROOT} -fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "--sysroot=${FREEBSD_SYSROOT} -fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "--sysroot=${FREEBSD_SYSROOT} -fuse-ld=lld")

# mulle-cmake detects the compiler from its name; a wrapper named
# "freebsd-mulle-clang" would be seen as plain clang, so state it explicitly
# (same as the Windows toolchain does).
if(FREEBSD_MULLE_OBJC)
    set(MULLE_C_COMPILER_ID   "MULLECLANG")
    set(MULLE_CXX_COMPILER_ID "MULLECLANG")
endif()

if(EXISTS "${FREEBSD_ROOT}/bin/freebsd-ar")
    set(CMAKE_AR     "${FREEBSD_ROOT}/bin/freebsd-ar"     CACHE FILEPATH "archiver" FORCE)
    set(CMAKE_RANLIB "${FREEBSD_ROOT}/bin/freebsd-ranlib" CACHE FILEPATH "ranlib" FORCE)
endif()

set(CMAKE_SYSROOT "${FREEBSD_SYSROOT}")

# Search programs on the host, libraries/headers on the target. BOTH keeps the
# dependency SDK usable.
set(CMAKE_FIND_ROOT_PATH
    "${FREEBSD_SYSROOT}"
)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# FreeBSD uses .so for shared libraries like Linux
set(CMAKE_STATIC_LIBRARY_SUFFIX ".a" CACHE STRING "static library suffix")
