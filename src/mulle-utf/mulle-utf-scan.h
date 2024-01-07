//
//  mulle-utf32-scan.h
//  mulle-utf
//
//  Copyright (C) 2019 Nat!, Mulle kybernetiK.
//  Copyright (c) 2019 Codeon GmbH.
//  All rights reserved.
//
//  Coded by Nat!
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#ifndef mulle_utf_scan_h__
#define mulle_utf_scan_h__

#include "mulle-utf-type.h"


//
// These routines only scan for ASCII 0-9 possibly prefixed with either +
// or -
//
// will return scanned position in p_s (+1)
// will not scan more than len
// will return value in p_value
//
// Return values:
//    -2 : has overflown
//    -1 : is not valid
//    0  : is long long
//    1  : is unsigned long long
//    2  : is long long but has trailing garbage
//    3  : is unsigned long long but has trailing garbage
//
MULLE__UTF_GLOBAL
int   _mulle_utf8_scan_longlong_decimal( char **p_s,
                                         size_t len,
                                         long long *p_value);

MULLE__UTF_GLOBAL
int   _mulle_utf16_scan_longlong_decimal( mulle_utf16_t **p_s,
                                          unsigned int len,
                                          long long *p_value);

MULLE__UTF_GLOBAL
int   _mulle_utf32_scan_longlong_decimal( mulle_utf32_t **p_s,
                                          unsigned int len,
                                          long long *p_value);

#endif
