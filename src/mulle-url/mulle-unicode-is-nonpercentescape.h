//
//  mulle_unicode_isnonpercentescape.h
//  mulle-unicode
//
//  Created by Nat! on 24.04.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_url_isnonpercentescape_h__
#define mulle_url_isnonpercentescape_h__

#include "include.h"
#include <stdint.h>

MULLE__URL_GLOBAL
int   mulle_unicode16_is_nonpercentescape( uint16_t c);
MULLE__URL_GLOBAL
int   mulle_unicode_is_nonpercentescape( int32_t c);
MULLE__URL_GLOBAL
int   mulle_unicode_is_nonpercentescapeplane( unsigned int plane);


#endif /* mulle_unicode_isnonpercentescape_h */
