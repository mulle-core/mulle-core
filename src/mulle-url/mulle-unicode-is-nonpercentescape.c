//
//  mulle_unicode_isnonpercentescape.c
//  mulle-unicode
//
//  Created by Nat! on 24.04.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#include "mulle-unicode-is-nonpercentescape.h"


int   mulle_unicode16_is_nonpercentescape( uint16_t c)
{
   if( c >= 'A' && c <= 'Z')
      return( 1);
   if( c >= 'a' && c <= 'z')
      return( 1);
   if( c >= '0' && c <= '9')
      return( 1);

   switch( c)
   {
   case '-' :
   case '_' :
   case '.' :
   case '~' :
      return( 1);
   }
   return( 0);

}


int   mulle_unicode_is_nonpercentescape( int32_t c)
{
   if( c <= 0xFFFF)
      return( mulle_unicode16_is_nonpercentescape( (uint16_t) c));
   return( 0);
}


int   mulle_unicode_is_nonpercentescapeplane( unsigned int plane)
{
   switch( plane)
   {
   case 0 :
      return( 1);
   }
   return( 0);
}
