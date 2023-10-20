//
//  mulle_unicode_is_validurlfragment.h
//  mulle-url
//
//  Created by Nat! on 2016-05-16 16:34:06 +0200.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_unicode_is_validurlfragment_h__
#define mulle_unicode_is_validurlfragment_h__

#include "include.h"
#include <stdint.h>

MULLE__URL_GLOBAL
int   mulle_unicode16_is_validurlfragment( uint16_t c);
MULLE__URL_GLOBAL
int   mulle_unicode_is_validurlfragment( int32_t c);
MULLE__URL_GLOBAL
int   mulle_unicode_is_validurlfragmentplane( unsigned int plane);

#endif /* mulle_unicode_is_validurlfragment_h */
