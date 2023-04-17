//
//  mulle_utf-type.h
//  mulle-utf
//
//  Created by Nat! on 18.03.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
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
#ifndef mulle_utf_type_h__
#define mulle_utf_type_h__

#include "include.h"

#include <stdint.h>
#include <stddef.h>


typedef uintptr_t  mulle_char5_t;
typedef uintptr_t  mulle_char7_t;

typedef uint8_t    mulle_utf8_t;
typedef uint16_t   mulle_utf16_t;

// this is signed, so any function returning an error can return -1
typedef int32_t   mulle_utf32_t;  //  0 - 0x10FFFF.


enum
{
   mulle_utf32_max = 0x10FFFF
};

//
// keeping mulle_utf32char_t signed, makes it easier
//

struct mulle_utf_information
{
   size_t   utf8len;
   size_t   utf16len;
   size_t   utf32len;
   void     *start;          // behind BOM if bommed, otherwise start
   void     *invalid;        // first fail char
   int      has_bom;
   int      is_ascii;
   int      is_char5;
   int      is_utf15;
   int      has_terminating_zero;
};


static inline int   mulle_utf_information_is_valid( struct mulle_utf_information *info)
{
   return( info->invalid == NULL);
}


enum mulle_utf_scan_return
{
   mulle_utf_has_overflown           = -2,
   mulle_utf_is_invalid              = -1,
   mulle_utf_is_valid                = 0x0,
   mulle_utf_is_too_large_for_signed = 0x1, // too large for a signed number
   mulle_utf_has_trailing_garbage    = 0x2
};

// void * is a user pointer, supplied else
typedef void   (*mulle_utf_add_bytes_function_t)( void *userinfo, void *bytes, size_t length);


enum mulle_utf_charinfo
{
   mulle_utf_is_not_char5_or_char7 = 0,
   mulle_utf_is_char5              = 1,
   mulle_utf_is_char7              = 2
};

#endif
