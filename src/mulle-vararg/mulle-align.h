//
//  mulle_align.h
//  mulle-objc-runtime
//
//  Created by Nat! on 07.04.16.
//  Copyright © 2016 Mulle kybernetiK. All rights reserved.
//

#ifndef mulle_align_h__
#define mulle_align_h__

#include <mulle-c11/mulle-c11.h>

#include <stdint.h>
#include <stddef.h>


MULLE_C_CONST_RETURN
static inline uintptr_t   mulle_address_align( uintptr_t p, unsigned int alignment)
{
   intptr_t   misalignment;

   if( alignment)
   {
      misalignment = p % alignment;
      if( misalignment)
         p += alignment - misalignment;
   }
   return( p);
}


MULLE_C_CONST_RETURN
static inline void   *mulle_pointer_align( void *p, unsigned int alignment)
{
   return( (void *) mulle_address_align( (uintptr_t) p, alignment));
}


#endif /* mulle_align_h */
