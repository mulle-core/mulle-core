//
//  mulle_utf_noncharacter.c
//  mulle-utf
//
//  Created by Nat! on 15.04.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#include "mulle-utf-noncharacter.h"


int   mulle_utf16_is_noncharacter( mulle_utf16_t c)
{
   //
   // Die Codebereiche von U+D800 bis U+DBFF (High-Surrogates) und von U+DC00 bis U+DFFF (Low-Surrogates)
   // sind speziell für solche UTF-16-Ersatzzeichen reserviert und enthalten daher keine eigenständigen Zeichen.)
   // surrogates in utf32 ? it's a non-character IMO
   //
   if( c >= 0xD800 && c < 0xE000)
      return( 1);
         // a contiguous range of 32 noncharacters: U+FDD0..U+FDEF in the BMP

   // the last two code points of the BMP, U+FFFE and U+FFFF
   // the last two code points of each of the 16 supplementary planes: U+1FFFE, U+1FFFF, U+2FFFE, U+2FFFF, ... U+10FFFE, U+10FFFF

   if( c >= 0xFDD0 && c <= 0xFDEF)
      return( 1);

   // It's important to note that code points 0xFFFE and 0xFFFF are not
   // considered invalid; they are known as the anti-BOM (U+FFFE) and the
   // Private Use Area (PUA) (U+FFFF) reddit.com. (hmm)
   if( c >= 0xfffe /*&& c <= 0xffff*/)
      return( 1);
   return( 0);
}


// Q: Which code points are noncharacters?
int   mulle_utf32_is_noncharacter( mulle_utf32_t c)
{
   if( c < 0x10000)
   {
      if( c < 0x0FFF)
         return( 0);
      if( mulle_utf16_is_noncharacter( (mulle_utf16_t) c))
         return( 1);
   }

   // above 0x10FFFF is not unicode
   if( c > 0x10FFFF)
      return( 1);

   // the last two code points of the BMP, U+FFFE and U+FFFF
   // the last two code points of each of the 16 supplementary planes: U+1FFFE, U+1FFFF, U+2FFFE, U+2FFFF, ... U+10FFFE, U+10FFFF
   switch( c & 0xffff)
   {
   case 0xfffe  :
   case 0xffff  :
      return( 1);
   }

   return( 0);
}


int   mulle_utf_is_noncharacterplane( size_t plane)
{
   MULLE_C_UNUSED( plane);

   return( 1);
}
