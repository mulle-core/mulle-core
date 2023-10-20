//
//  mulle-linkedlist.c
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

#ifndef mulle_linkedlist_h__
#define mulle_linkedlist_h__

#include "include.h"

#include <string.h>
#include <assert.h>

#define MULLE__LINKEDLIST_VERSION  ((0 << 20) | (0 << 8) | 2)


static inline unsigned int   mulle_linkedlist_get_version_major( void)
{
   return( MULLE__LINKEDLIST_VERSION >> 20);
}


static inline unsigned int   mulle_linkedlist_get_version_minor( void)
{
   return( (MULLE__LINKEDLIST_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_linkedlist_get_version_patch( void)
{
   return( MULLE__LINKEDLIST_VERSION & 0xFF);
}


MULLE__LINKEDLIST_GLOBAL
uint32_t   mulle_linkedlist_get_version( void);


//
// ** NOT multi-threaded**
//
// This is a basic linked list. You place struct _mulle_linkedlistentry
// as the top element of your struct.
//
// Useful if you prefer locking or can do without multi-thread access.
//
struct _mulle_linkedlistentry
{
   struct _mulle_linkedlistentry   *_next;
};


MULLE_C_NONNULL_FIRST_SECOND
static inline void
   _mulle_linkedlistentry_chain( struct _mulle_linkedlistentry **head,
                                 struct _mulle_linkedlistentry *entry)
{
   assert( head);
   assert( entry);

   entry->_next = *head;
   *head = entry;
}


MULLE_C_NONNULL_FIRST
static inline struct _mulle_linkedlistentry *
   _mulle_linkedlistentry_unchain( struct _mulle_linkedlistentry **head)
{
   struct _mulle_linkedlistentry  *entry;

   assert( head);

   entry = *head;
   if( entry)
   {
      *head = entry->_next;
      entry->_next = NULL;
   }
   return( entry);
}


typedef void  mulle_linkedlistentry_walk_callback_t( void *, struct _mulle_linkedlistentry *);

//
// thinly disguised routine to call mulle_allocator_free on all entries
//
MULLE_C_NONNULL_FIRST_SECOND
static inline void
   _mulle_linkedlistentry_walk( struct _mulle_linkedlistentry *next,
                                mulle_linkedlistentry_walk_callback_t  *callback,
                                void *userinfo)
{
   struct _mulle_linkedlistentry *entry;

   MULLE_C_ASSERT( sizeof( struct _mulle_linkedlistentry *) == sizeof( void *));

   assert( next);
   assert( callback);

   while( (entry = next))
   {
      next = entry->_next;
      (*callback)( userinfo, entry);
   }
}


MULLE_C_NONNULL_SECOND
static inline void
   mulle_linkedlistentry_walk( struct _mulle_linkedlistentry *next,
                                mulle_linkedlistentry_walk_callback_t  *callback,
                                void *userinfo)
{
   if( next)
      _mulle_linkedlistentry_walk( next, callback, userinfo);
}



//
// ** NOT multi-threaded**
//
//
// This is a single threaded version of mulle_concurrent_linkedlist.
// Useful if you prefer locking or can do without multi-thread access.
// It trys to mimic mulle_concurrent_linkedlist, although it could be
// written a bit simpler.
//
struct _mulle_linkedlist
{
   struct _mulle_linkedlistentry   _head;
};


MULLE_C_NONNULL_FIRST
static inline void    
   _mulle_linkedlist_init( struct _mulle_linkedlist *p)
{
   memset( p, 0, sizeof( *p));
}


MULLE_C_NONNULL_FIRST
static inline void   
   _mulle_linkedlist_done( struct _mulle_linkedlist *p)
{
   assert( ! p->_head._next);
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
static inline struct _mulle_linkedlistentry  *
   _mulle_linkedlist_remove_all( struct _mulle_linkedlist *list)
{
   struct _mulle_linkedlistentry  *head;

   assert( list);

   head = list->_head._next;
   list->_head._next = NULL;
   return( head);
}


MULLE_C_NONNULL_FIRST
static inline struct _mulle_linkedlistentry  *
   _mulle_linkedlist_remove_one( struct _mulle_linkedlist *list)
{
   struct _mulle_linkedlistentry  *head;

   assert( list);

   head = list->_head._next;
   if( head)
   {
      list->_head._next = head->_next;
      head->_next = NULL;
   }
   return( head);
}


static inline struct _mulle_linkedlistentry  *
   mulle_linkedlist_remove_one( struct _mulle_linkedlist *list)
{
   if( ! list)
      return( NULL);
   return( _mulle_linkedlist_remove_one( list));
}



MULLE_C_NONNULL_FIRST_SECOND
static inline void
   _mulle_linkedlist_add( struct _mulle_linkedlist *list,
                          struct _mulle_linkedlistentry *entry)
{
   struct _mulle_linkedlistentry  *next;

   assert( list);
   assert( entry);
   assert( ! entry->_next);

   next = list->_head._next;
   assert( next != entry);
   entry->_next = next;
   list->_head._next = entry;
}


typedef int   mulle_linkedlist_walk_callback_t( struct _mulle_linkedlistentry *,
                                                struct _mulle_linkedlistentry *,
                                                void *);


MULLE_C_NONNULL_FIRST_SECOND
static inline int
   _mulle_linkedlist_walk( struct _mulle_linkedlist *list,
                           mulle_linkedlist_walk_callback_t *callback,
                           void *userinfo)
{
   struct _mulle_linkedlistentry   *entry;
   struct _mulle_linkedlistentry   *prev;
   struct _mulle_linkedlistentry   *next;
   int                             rval;

   assert( list);
   assert( callback);

   entry = list->_head._next;
   prev  = NULL;
   rval  = 0;

   while( entry)
   {
      next = entry->_next; // do this now, so callback can remove it
      rval = (*callback)( entry, prev, userinfo);
      if( rval)
         break;

      prev  = entry;
      entry = next;
   }

   return( rval);
}


MULLE_C_NONNULL_SECOND
static inline int
   mulle_linkedlist_walk( struct _mulle_linkedlist *list,
                           mulle_linkedlist_walk_callback_t *callback,
                           void *userinfo)
{
   if( ! list)
      return( 0);
   return( _mulle_linkedlist_walk( list, callback, userinfo));
}


#include "mulle-concurrent-linkedlist.h"

/*
 * The versioncheck header can be generated with
 * mulle-project-dependency-versions, but it is optional.
 */
#ifdef __has_include
# if __has_include( "_mulle-linkedlist-versioncheck.h")
#  include "_mulle-linkedlist-versioncheck.h"
# endif
#endif

#endif
