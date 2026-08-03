//
//  mulle-utf-privatecharacter.c
//  mulle-utf
//
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//
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
#include "mulle-utf-privatecharacter.h"


int   mulle_utf16_is_privatecharacter( uint16_t c)
{
   // quick check for most
   if( c < 0xE000)
      return( 0);

   if( c <= 0xF8FF)
      return( 1);

   return( 0);
}

int   mulle_utf32_is_privatecharacter( int32_t c)
{
   if( c <= 0xFFFF)
      return( mulle_utf16_is_privatecharacter( (uint16_t) c));

   if( c >= 0xF0000 && c <= 0xFFFFD)
      return( 1);
   if( c >= 0x100000 && c <= 0x10FFFD)
      return( 1);

   return( 0);
}


int   mulle_utf_is_privatecharacterplane( size_t plane)
{
   switch( plane)
   {
   case 0x00 :
   case 0x0F :
   case 0x10 :
      return( 1);
   }
   return( 0);
}

