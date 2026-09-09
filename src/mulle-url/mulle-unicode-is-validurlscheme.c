//
//  mulle-unicode-is-validurlscheme.c
//  mulle-url
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
#include "mulle-unicode-is-validurlscheme.h"


int   mulle_unicode16_is_validurlscheme( uint16_t c)
{
   // scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

   if( c < 0x002b)
      return( 0);

   if( c > 0x007a)
      return( 0);

   switch( c)
   {
   case 0x002c :
   case 0x002f :
   case 0x003a :
   case 0x003b :
   case 0x003c :
   case 0x003d :
   case 0x003e :
   case 0x003f :
   case 0x0040 :
   case 0x005b :
   case 0x005c :
   case 0x005d :
   case 0x005e :
   case 0x005f :
   case 0x0060 :
      return( 0);
   }
   return( 1);
}


int   mulle_unicode_is_validurlscheme( int32_t c)
{
   if( c <= 0xFFFF)
      return( mulle_unicode16_is_validurlscheme( (uint16_t) c));

   return( 0);
}


int   mulle_unicode_is_validurlschemeplane( unsigned int plane)
{
   switch( plane)
   {
   case 0 :
      return( 1);
   }
   return( 0);
}
