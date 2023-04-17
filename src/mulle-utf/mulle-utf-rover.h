//
//  mulle-utf-rover.h
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
#ifndef mulle_utf32_rover_h__
#define mulle_utf32_rover_h__

#include "mulle-utf-type.h"


struct mulle_utf_rover
{
   void           *s;
   void           *sentinel;
   mulle_utf32_t  (*next)( struct mulle_utf_rover *rover);
   void           (*dialback)( struct mulle_utf_rover *rover);
};


static inline void   *_mulle_utf_rover_get_current( struct mulle_utf_rover *rover)
{
   return( rover->s);
}


static inline int   _mulle_utf_rover_has_character( struct mulle_utf_rover *rover)
{
   return( (char *) rover->s < (char *) rover->sentinel);
}


static inline int   _mulle_utf_rover_next_character( struct mulle_utf_rover *rover)
{
   return( (*rover->next)( rover));
}


static inline void   _mulle_utf_rover_dial_back( struct mulle_utf_rover *rover)
{
   (*rover->dialback)( rover);
}


MULLE_UTF_GLOBAL
void   _mulle_utf32_rover_init( struct mulle_utf_rover *rover,
                                mulle_utf32_t *s,
                                size_t len);
MULLE_UTF_GLOBAL
void   _mulle_utf16_rover_init( struct mulle_utf_rover *rover,
                                mulle_utf16_t *s,
                                size_t len);
MULLE_UTF_GLOBAL
void   _mulle_utf8_rover_init( struct mulle_utf_rover *rover,
                               mulle_utf8_t *s,
                               size_t len);

#endif

