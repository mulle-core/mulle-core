//
//  mulle_unicode_is_whitespaceornewline.h
//  mulle-unicode
//
//  Created by Nat! on 2016-04-15 17:24:35 +0200.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_unicode_is_whitespaceornewline_h__
#define mulle_unicode_is_whitespaceornewline_h__

#include "include.h"
#include <stdint.h>

MULLE__UNICODE_GLOBAL
int   mulle_unicode16_is_whitespaceornewline( uint16_t c);
MULLE__UNICODE_GLOBAL
int   mulle_unicode_is_whitespaceornewline( int32_t c);
MULLE__UNICODE_GLOBAL
int   mulle_unicode_is_whitespaceornewlineplane( unsigned int plane);

#endif /* mulle_unicode_is_whitespaceornewline_h */
