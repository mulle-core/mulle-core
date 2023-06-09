//
//  mulle_unicode_is_validurlscheme.c
//  mulle-url
//
//  Created by Nat! on 16.05.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
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
