//
//  mulle_utf_privatecharacter.c
//  mulle-utf
//
//  Created by Nat! on 15.04.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//

#ifndef mulle_utf_privatecharacter_h__
#define mulle_utf_privatecharacter_h__

#include "mulle-utf-type.h"
#include <assert.h>


MULLE__UTF_GLOBAL
int   mulle_utf16_is_privatecharacter( uint16_t c);
MULLE__UTF_GLOBAL
int   mulle_utf32_is_privatecharacter( int32_t c);
MULLE__UTF_GLOBAL
int   mulle_utf_is_privatecharacterplane( size_t plane);

#endif
