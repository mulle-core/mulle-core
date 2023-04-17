//
//  mulle_url8_is_validurlscheme.h
//  mulle-url
//
//  Created by Nat! on 16.05.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_unicode_is_validurlscheme_h__
#define mulle_unicode_is_validurlscheme_h__

#include "include.h"
#include <stdint.h>

MULLE_URL_GLOBAL
int   mulle_unicode16_is_validurlscheme( uint16_t c);
MULLE_URL_GLOBAL
int   mulle_unicode_is_validurlscheme( int32_t c);
MULLE_URL_GLOBAL
int   mulle_unicode_is_validurlschemeplane( unsigned int plane);

#endif /* mulle_url8_is_validurlscheme_h */
