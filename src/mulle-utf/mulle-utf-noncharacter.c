//
//  mulle-utf-noncharacter.c
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
#include "mulle-utf-noncharacter.h"


int   mulle_utf16_is_noncharacter( mulle_utf16_t c)
{
   // a contiguous range of 32 noncharacters: U+FDD0..U+FDEF in the BMP
   if( c >= 0xFDD0 && c <= 0xFDEF)
      return( 1);

   // the last two code points of the BMP, U+FFFE and U+FFFF
   if( c >= 0xFFFE)
      return( 1);

   return( 0);
}


// Q: Which code points are noncharacters?
int   mulle_utf32_is_noncharacter( mulle_utf32_t c)
{
   // BMP noncharacters (delegate to utf16 version)
   if( c >= 0xFDD0 && c <= 0xFFFF)
      return( mulle_utf16_is_noncharacter( (mulle_utf16_t) c));

   // above 0x10FFFF is not unicode (but not a "noncharacter" per se)
   if( c < 0x10000 || c > 0x10FFFF)
      return( 0);

   // the last two code points of each of the 16 supplementary planes:
   // U+1FFFE, U+1FFFF, U+2FFFE, U+2FFFF, ... U+10FFFE, U+10FFFF
   switch( c & 0xFFFF)
   {
   case 0xFFFE :
   case 0xFFFF :
      return( 1);
   }

   return( 0);
}


//
// Every Unicode plane (0-16) contains at least two noncharacters (U+xFFFE
// and U+xFFFF). Plane 0 (BMP) additionally has U+FDD0..U+FDEF.
// Planes above 16 are entirely outside Unicode and all their code points
// are treated as noncharacters.
// This function exists for orthogonality with mulle_utf_is_privatecharacterplane.
//
int   mulle_utf_is_noncharacterplane( size_t plane)
{
   MULLE_C_UNUSED( plane);

   return( 1);
}
