# This file will be regenerated by `mulle-match-to-cmake` via
# `mulle-sde reflect` and any edits will be lost.
#
# This file will be included by cmake/share/Headers.cmake
#
if( MULLE_TRACE_INCLUDE)
   MESSAGE( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# contents are derived from the file locations

set( INCLUDE_DIRS
src
src/generic
src/mulle-aba
src/mulle-allocator
src/mulle-buffer
src/mulle-c11
src/mulle-concurrent
src/mulle-container
src/mulle-data
src/mulle-dlfcn
src/mulle-fifo
src/mulle-fprintf
src/mulle-http
src/mulle-mmap
src/mulle-multififo
src/mulle-slug
src/mulle-sprintf
src/mulle-stacktrace
src/mulle-thread
src/mulle-time
src/mulle-unicode
src/mulle-url
src/mulle-utf
src/mulle-vararg
src/reflect
)

#
# contents selected with patternfile ??-header--private-generated-headers
#
set( PRIVATE_GENERATED_HEADERS
src/reflect/_mulle-core-include-private.h
)

#
# contents selected with patternfile ??-header--private-generic-headers
#
set( PRIVATE_GENERIC_HEADERS
src/generic/include-private.h
src/mulle-aba/include-private.h
src/mulle-buffer/include-private.h
src/mulle-concurrent/include-private.h
src/mulle-container/include-private.h
src/mulle-dlfcn/include-private.h
src/mulle-fifo/include-private.h
src/mulle-fprintf/include-private.h
src/mulle-http/include-private.h
src/mulle-mmap/include-private.h
src/mulle-multififo/include-private.h
src/mulle-slug/include-private.h
src/mulle-sprintf/include-private.h
src/mulle-stacktrace/include-private.h
src/mulle-thread/include-private.h
src/mulle-unicode/include-private.h
src/mulle-url/include-private.h
src/mulle-utf/include-private.h
)

#
# contents selected with patternfile ??-header--private-headers
#
set( PRIVATE_HEADERS
src/mulle-aba/_mulle-aba-include-private.h
src/mulle-buffer/_mulle-buffer-include-private.h
src/mulle-concurrent/_mulle-concurrent-include-private.h
src/mulle-container/_mulle-container-include-private.h
src/mulle-dlfcn/_mulle-dlfcn-include-private.h
src/mulle-fifo/_mulle-fifo-include-private.h
src/mulle-fprintf/_mulle-fprintf-include-private.h
src/mulle-http/_mulle-http-include-private.h
src/mulle-mmap/_mulle-mmap-include-private.h
src/mulle-multififo/_mulle-multififo-include-private.h
src/mulle-slug/_mulle-slug-include-private.h
src/mulle-sprintf/_mulle-sprintf-include-private.h
src/mulle-stacktrace/_mulle-stacktrace-include-private.h
src/mulle-thread/_mulle-thread-include-private.h
src/mulle-unicode/_mulle-unicode-include-private.h
src/mulle-url/_mulle-url-include-private.h
src/mulle-utf/_mulle-utf-include-private.h
)

#
# contents selected with patternfile ??-header--public-generated-headers
#
set( PUBLIC_GENERATED_HEADERS
src/reflect/_mulle-core-include.h
)

#
# contents selected with patternfile ??-header--public-generic-headers
#
set( PUBLIC_GENERIC_HEADERS
src/generic/include.h
src/mulle-aba/include.h
src/mulle-buffer/include.h
src/mulle-concurrent/include.h
src/mulle-container/include.h
src/mulle-data/include.h
src/mulle-dlfcn/include.h
src/mulle-fifo/include.h
src/mulle-fprintf/include.h
src/mulle-http/include.h
src/mulle-mmap/include.h
src/mulle-multififo/include.h
src/mulle-slug/include.h
src/mulle-sprintf/include.h
src/mulle-stacktrace/include.h
src/mulle-thread/include.h
src/mulle-unicode/include.h
src/mulle-url/include.h
src/mulle-utf/include.h
)

#
# contents selected with patternfile ??-header--public-headers
#
set( PUBLIC_HEADERS
src/mulle-aba/mulle-aba-defines.h
src/mulle-aba/mulle-aba.h
src/mulle-aba/_mulle-aba-include.h
src/mulle-aba/mulle-aba-linkedlist.h
src/mulle-aba/_mulle-aba-provide.h
src/mulle-aba/mulle-aba-storage.h
src/mulle-aba/_mulle-aba-versioncheck.h
src/mulle-allocator/mulle-allocator.h
src/mulle-allocator/mulle-allocator-struct.h
src/mulle-allocator/_mulle-allocator-versioncheck.h
src/mulle-buffer/mulle--buffer.h
src/mulle-buffer/mulle-buffer.h
src/mulle-buffer/_mulle-buffer-include.h
src/mulle-buffer/_mulle-buffer-versioncheck.h
src/mulle-buffer/mulle-flexbuffer.h
src/mulle-c11/mulle-c11-eval.h
src/mulle-c11/mulle-c11.h
src/mulle-concurrent/mulle-concurrent.h
src/mulle-concurrent/mulle-concurrent-hashmap.h
src/mulle-concurrent/_mulle-concurrent-include.h
src/mulle-concurrent/mulle-concurrent-pointerarray.h
src/mulle-concurrent/_mulle-concurrent-provide.h
src/mulle-concurrent/mulle-concurrent-types.h
src/mulle-concurrent/_mulle-concurrent-versioncheck.h
src/mulle-container/mulle--array.h
src/mulle-container/mulle-array.h
src/mulle-container/mulle-container-callback-global.h
src/mulle-container/mulle-container-callback.h
src/mulle-container/mulle-container-compiler-attributes.h
src/mulle-container/mulle-container.h
src/mulle-container/_mulle-container-include.h
src/mulle-container/mulle-container-math.h
src/mulle-container/mulle-container-operation.h
src/mulle-container/_mulle-container-provide.h
src/mulle-container/_mulle-container-versioncheck.h
src/mulle-container/mulle-flexarray.h
src/mulle-container/mulle--map.h
src/mulle-container/mulle-map.h
src/mulle-container/mulle--pointerarray.h
src/mulle-container/mulle-pointerarray.h
src/mulle-container/mulle--pointermap-generic.h
src/mulle-container/mulle--pointermap.h
src/mulle-container/mulle--pointermap-struct.h
src/mulle-container/mulle--pointerpairarray.h
src/mulle-container/mulle-pointerpairarray.h
src/mulle-container/mulle-pointerpair.h
src/mulle-container/mulle--pointerqueue.h
src/mulle-container/mulle-pointerqueue.h
src/mulle-container/mulle--pointerset-generic.h
src/mulle-container/mulle--pointerset.h
src/mulle-container/mulle--pointerset-struct.h
src/mulle-container/mulle--rangeset.h
src/mulle-container/mulle--set.h
src/mulle-container/mulle-set.h
src/mulle-container/mulle--structarray.h
src/mulle-container/mulle-structarray.h
src/mulle-container/mulle--uniquepointerarray.h
src/mulle-container/mulle-uniquepointerarray.h
src/mulle-core.h
src/mulle-data/farmhash.h
src/mulle-data/largeint.h
src/mulle-data/mulle-data.h
src/mulle-data/_mulle-data-versioncheck.h
src/mulle-data/mulle-fnv1a.h
src/mulle-data/mulle-hash.h
src/mulle-data/mulle-prime.h
src/mulle-data/mulle-range.h
src/mulle-dlfcn/mulle-dlfcn.h
src/mulle-dlfcn/_mulle-dlfcn-include.h
src/mulle-dlfcn/_mulle-dlfcn-versioncheck.h
src/mulle-fifo/mulle-fifo.h
src/mulle-fifo/_mulle-fifo-include.h
src/mulle-fifo/_mulle-fifo-include-public.h
src/mulle-fifo/_mulle-fifo-provide.h
src/mulle-fifo/_mulle-fifo-versioncheck.h
src/mulle-fifo/mulle--pointerfifo1024.h
src/mulle-fifo/mulle--pointerfifo128.h
src/mulle-fifo/mulle--pointerfifo16.h
src/mulle-fifo/mulle--pointerfifo2048.h
src/mulle-fifo/mulle--pointerfifo256.h
src/mulle-fifo/mulle--pointerfifo32.h
src/mulle-fifo/mulle--pointerfifo4096.h
src/mulle-fifo/mulle--pointerfifo4.h
src/mulle-fifo/mulle--pointerfifo512.h
src/mulle-fifo/mulle--pointerfifo64.h
src/mulle-fifo/mulle--pointerfifo8192.h
src/mulle-fifo/mulle--pointerfifo8.h
src/mulle-fifo/mulle-pointerfifo.h
src/mulle-fprintf/mulle-fprintf.h
src/mulle-fprintf/_mulle-fprintf-include.h
src/mulle-fprintf/_mulle-fprintf-include-public.h
src/mulle-fprintf/_mulle-fprintf-provide.h
src/mulle-fprintf/_mulle-fprintf-versioncheck.h
src/mulle-http/http_parser.h
src/mulle-http/mulle-http.h
src/mulle-http/_mulle-http-include.h
src/mulle-http/_mulle-http-provide.h
src/mulle-http/_mulle-http-versioncheck.h
src/mulle-mmap/mulle-mmap.h
src/mulle-mmap/_mulle-mmap-include.h
src/mulle-mmap/_mulle-mmap-versioncheck.h
src/mulle-multififo/mulle-multififo.h
src/mulle-multififo/_mulle-multififo-include.h
src/mulle-multififo/_mulle-multififo-include-public.h
src/mulle-multififo/_mulle-multififo-provide.h
src/mulle-multififo/_mulle-multififo-versioncheck.h
src/mulle-multififo/mulle-pointermultififo.h
src/mulle-slug/mulle-slug.h
src/mulle-slug/_mulle-slug-include.h
src/mulle-sprintf/mulle-sprintf-character.h
src/mulle-sprintf/mulle-sprintf-escape.h
src/mulle-sprintf/mulle-sprintf-fp.h
src/mulle-sprintf/mulle-sprintf-function.h
src/mulle-sprintf/mulle-sprintf.h
src/mulle-sprintf/_mulle-sprintf-include.h
src/mulle-sprintf/mulle-sprintf-integer.h
src/mulle-sprintf/mulle-sprintf-pointer.h
src/mulle-sprintf/mulle-sprintf-return.h
src/mulle-sprintf/mulle-sprintf-string.h
src/mulle-sprintf/_mulle-sprintf-versioncheck.h
src/mulle-stacktrace/mulle-stacktrace.h
src/mulle-stacktrace/_mulle-stacktrace-include.h
src/mulle-stacktrace/_mulle-stacktrace-versioncheck.h
src/mulle-thread/mulle-atomic-c11.h
src/mulle-thread/mulle-atomic.h
src/mulle-thread/mulle-atomic-mintomic.h
src/mulle-thread/mulle-thread-c11.h
src/mulle-thread/mulle-thread.h
src/mulle-thread/_mulle-thread-include.h
src/mulle-thread/_mulle-thread-provide.h
src/mulle-thread/mulle-thread-pthreads.h
src/mulle-thread/_mulle-thread-versioncheck.h
src/mulle-thread/mulle-thread-windows.h
src/mulle-time/mulle-absolutetime.h
src/mulle-time/mulle-calendartime.h
src/mulle-time/mulle-relativetime.h
src/mulle-time/mulle-time.h
src/mulle-time/mulle-timespec.h
src/mulle-time/mulle-timetype.h
src/mulle-time/mulle-timeval.h
src/mulle-unicode/miniplane.h
src/mulle-unicode/mulle-unicode-ctype.h
src/mulle-unicode/mulle-unicode.h
src/mulle-unicode/_mulle-unicode-include.h
src/mulle-unicode/mulle-unicode-is-alphanumeric.h
src/mulle-unicode/mulle-unicode-is-capitalized.h
src/mulle-unicode/mulle-unicode-is-control.h
src/mulle-unicode/mulle-unicode-is-decimaldigit.h
src/mulle-unicode/mulle-unicode-is-decomposable.h
src/mulle-unicode/mulle-unicode-is-legalcharacter.h
src/mulle-unicode/mulle-unicode-is-letter.h
src/mulle-unicode/mulle-unicode-is-lowercase.h
src/mulle-unicode/mulle-unicode-is-newline.h
src/mulle-unicode/mulle-unicode-is-nonbase.h
src/mulle-unicode/mulle-unicode-is-noncharacter.h
src/mulle-unicode/mulle-unicode-is-punctuation.h
src/mulle-unicode/mulle-unicode-is-symbol.h
src/mulle-unicode/mulle-unicode-is-uppercase.h
src/mulle-unicode/mulle-unicode-is-whitespace.h
src/mulle-unicode/mulle-unicode-is-whitespaceornewline.h
src/mulle-unicode/mulle-unicode-is-zerodigit.h
src/mulle-unicode/mulle-unicode-tolower.h
src/mulle-unicode/mulle-unicode-toupper.h
src/mulle-unicode/_mulle-unicode-versioncheck.h
src/mulle-url/mulle-unicode-is-nonpercentescape.h
src/mulle-url/mulle-unicode-is-validurlfragment.h
src/mulle-url/mulle-unicode-is-validurlhost.h
src/mulle-url/mulle-unicode-is-validurlpassword.h
src/mulle-url/mulle-unicode-is-validurlpath.h
src/mulle-url/mulle-unicode-is-validurlquery.h
src/mulle-url/mulle-unicode-is-validurlscheme.h
src/mulle-url/mulle-unicode-is-validurluser.h
src/mulle-url/mulle-url.h
src/mulle-url/_mulle-url-include.h
src/mulle-url/_mulle-url-provide.h
src/mulle-url/_mulle-url-versioncheck.h
src/mulle-utf/mulle-ascii.h
src/mulle-utf/mulle-char5.h
src/mulle-utf/mulle-char7.h
src/mulle-utf/mulle-utf16.h
src/mulle-utf/mulle-utf16-string.h
src/mulle-utf/mulle-utf32.h
src/mulle-utf/mulle-utf32-string.h
src/mulle-utf/mulle-utf8.h
src/mulle-utf/mulle-utf-convenience.h
src/mulle-utf/mulle-utf.h
src/mulle-utf/_mulle-utf-include.h
src/mulle-utf/mulle-utf-noncharacter.h
src/mulle-utf/mulle-utf-privatecharacter.h
src/mulle-utf/_mulle-utf-provide.h
src/mulle-utf/mulle-utf-rover.h
src/mulle-utf/mulle-utf-scan.h
src/mulle-utf/mulle-utf-type.h
src/mulle-utf/_mulle-utf-versioncheck.h
src/mulle-vararg/mulle-align.h
src/mulle-vararg/mulle-vararg-builder.h
src/mulle-vararg/mulle-vararg.h
)
