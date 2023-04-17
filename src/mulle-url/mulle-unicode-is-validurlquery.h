//
//  mulle_unicode_is_validurlquery.h
//  mulle-url
//
//  Created by Nat! on 2016-05-16 16:34:06 +0200.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_unicode_is_validurlquery_h__
#define mulle_unicode_is_validurlquery_h__

#include "include.h"
#include <stdint.h>

MULLE_URL_GLOBAL
int   mulle_unicode16_is_validurlquery( uint16_t c);
MULLE_URL_GLOBAL
int   mulle_unicode_is_validurlquery( int32_t c);
MULLE_URL_GLOBAL
int   mulle_unicode_is_validurlqueryplane( unsigned int plane);

#endif /* mulle_unicode_is_validurlquery_h */
