//
//  mulle-concurrent-linkedlist.c
//  mulle-linkedlist
//
//  Created by Nat! on 01.07.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#ifndef mulle_concurrent_linkedlist_h__
#define mulle_concurrent_linkedlist_h__

#include "include.h"

#include <string.h>
#include <assert.h>


struct _mulle_concurrent_linkedlistentry
{
   struct _mulle_concurrent_linkedlistentry   *_next;
};


//
// since the mulle_atomic_pointer_t is opaque, I use this union
// to make it easier to debug
//
union _mulle_concurrent_atomiclinkedlistentry
{
   struct _mulle_concurrent_linkedlistentry  *entry;  // never read it except in the debugger
   mulle_atomic_pointer_t                    pointer;
};


struct _mulle_concurrent_linkedlist
{
   union _mulle_concurrent_atomiclinkedlistentry   _head;
};


MULLE_C_NONNULL_FIRST
static inline void    
   _mulle_concurrent_linkedlist_init( struct _mulle_concurrent_linkedlist *p)
{
   memset( p, 0, sizeof( *p));
}

MULLE_C_NONNULL_FIRST
static inline void   
   _mulle_concurrent_linkedlist_done( struct _mulle_concurrent_linkedlist *p)
{
   assert( ! _mulle_atomic_pointer_nonatomic_read( &p->_head.pointer));
}


//
// limited functionality, add to back (single)
// remove all
//
//
// retrieves the current head pointer and sets it to NULL in one atomic
// operation
//
MULLE_C_NONNULL_FIRST
static inline struct _mulle_concurrent_linkedlistentry  *
   _mulle_concurrent_linkedlist_remove_all( struct _mulle_concurrent_linkedlist *list)
{
   struct _mulle_concurrent_linkedlistentry  *head;

   assert( list);

   do
   {
      head = _mulle_atomic_pointer_read( &list->_head.pointer);
      if( ! head)
         break;
   }
   while( ! _mulle_atomic_pointer_weakcas( &list->_head.pointer, NULL, head));

   return( head);
}


MULLE_C_NONNULL_FIRST_SECOND
static inline void  
   _mulle_concurrent_linkedlist_add( struct _mulle_concurrent_linkedlist *list,
                                     struct _mulle_concurrent_linkedlistentry  *entry)
{
   struct _mulle_concurrent_linkedlistentry  *head;

   assert( list);
   assert( entry);
   assert( ! entry->_next);

   do
   {
      head = _mulle_atomic_pointer_read( &list->_head.pointer);
      assert( head != entry);

      //MULLE_THREAD_UNPLEASANT_RACE_YIELD();
      entry->_next = head;
   }
   while( ! _mulle_atomic_pointer_cas( &list->_head.pointer, entry, head));
}



// based on remove all, removes all then adds back
MULLE__LINKEDLIST_GLOBAL MULLE_C_NONNULL_FIRST
struct _mulle_concurrent_linkedlistentry  *
   _mulle_concurrent_linkedlist_remove_one( struct _mulle_concurrent_linkedlist *list);

//
// NOT THREADSAFE AT ALL
//
MULLE__LINKEDLIST_GLOBAL MULLE_C_NONNULL_FIRST_SECOND
int   _mulle_concurrent_linkedlist_walk( struct _mulle_concurrent_linkedlist *list,
                                         int (*callback)( struct _mulle_concurrent_linkedlistentry *,
                                                          struct _mulle_concurrent_linkedlistentry *,
                                                         void *),
                                         void *userinfo);

#endif
