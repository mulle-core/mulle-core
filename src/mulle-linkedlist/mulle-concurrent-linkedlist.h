//
//  mulle-concurrent-linkedlist.h
//  mulle-linkedlist
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
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


//
// Entries are intrusive and caller-owned. An entry must be unlinked and
// exclusively owned by the caller before it is added.
//
// The concurrent operations provide atomic head manipulation, but they do not
// provide memory reclamation. Removing an entry does not make it safe to free
// or reuse while another thread may still hold a pointer to it. Callers must
// provide the required ownership, quiescence, or reclamation discipline.
//
//
// MEMORY MODEL (see dox/MEMORY_MODEL.md for the full rationale)
//
// mulle-thread offers exactly two orderings: the default operations are
// sequentially consistent and the `_relaxed` suffixed operations are relaxed.
// There is no acquire-only or release-only variant. This list therefore uses:
//
//    default (seq_cst)  for every operation that publishes or consumes an
//                       entry, i.e. all CAS operations on `_head`
//    relaxed            only for the speculative `_head` load at the top of a
//                       CAS retry loop, whose value is never dereferenced and
//                       only used as the CAS `expect` operand
//    nonatomic          for `_next` traversal of a chain that the calling
//                       thread has already detached (and therefore owns), and
//                       for `init`/`done`/`walk`, which require exclusive
//                       access by contract
//
// The key invariant that makes the relaxed loads sound: *every* store to
// `_head` after `init` is a read-modify-write (a CAS). The modification order
// of `_head` is therefore an unbroken release sequence, so a thread that
// acquires any `_head` value also synchronizes with all publications that
// precede it in that order. Do not introduce a plain `_write` or
// `_write_relaxed` on `_head`, that would cut the chain.
//
// Requires mulle-thread 4.9.0 or better for the `_relaxed` API.
//
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


//
// Non-atomic by contract: the list must not be reachable by another thread
// yet. Publishing the list itself (or the object containing it) is the
// caller's job and needs the caller's own release operation.
//
MULLE_C_NONNULL_FIRST
static inline void    
   _mulle_concurrent_linkedlist_init( struct _mulle_concurrent_linkedlist *p)
{
   memset( p, 0, sizeof( *p));
}


//
// Non-atomic by contract: all other threads must have quiesced.
//
MULLE_C_NONNULL_FIRST
static inline void   
   _mulle_concurrent_linkedlist_done( struct _mulle_concurrent_linkedlist *p)
{
   assert( ! _mulle_atomic_pointer_nonatomic_read( &p->_head.pointer));

   MULLE_C_UNUSED( p);
}


//
// limited functionality, prepend to head (concurrent)
// remove all
//
//
// retrieves the current head pointer and sets it to NULL in one atomic
// operation
//
// Memory model: the speculative read is relaxed, because its value is not
// dereferenced here, it is only the CAS `expect` operand. A stale value makes
// the CAS fail and we retry. The acquire that lets the *caller* dereference
// the returned chain comes from the successful seq_cst CAS, which reads the
// very value it returns.
//
// A NULL return means "empty at some point during this call", not "empty
// now". That is inherent to a concurrent list and not a consequence of the
// relaxed read.
//
MULLE_C_NONNULL_FIRST
static inline struct _mulle_concurrent_linkedlistentry  *
   _mulle_concurrent_linkedlist_remove_all( struct _mulle_concurrent_linkedlist *list)
{
   struct _mulle_concurrent_linkedlistentry  *head;

   assert( list);

   do
   {
      head = _mulle_atomic_pointer_read_relaxed( &list->_head.pointer);
      if( ! head)
         break;
   }
   while( ! _mulle_atomic_pointer_weakcas( &list->_head.pointer, NULL, head));

   return( head);
}


//
// Memory model: the speculative read is relaxed, `head` is only stored into
// `entry->_next` and used as the CAS `expect` operand, it is never
// dereferenced. The publishing CAS is seq_cst, so the write to `entry->_next`
// and the caller's initialization of the surrounding object are visible to any
// thread that later obtains `entry` from `_head`.
//
// Therefore: the caller must complete all writes to the entry *before*
// calling add, and must not touch the entry afterwards.
//
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
      head = _mulle_atomic_pointer_read_relaxed( &list->_head.pointer);
      assert( head != entry);

      //MULLE_THREAD_UNPLEASANT_RACE_YIELD();
      entry->_next = head;
   }
   while( ! _mulle_atomic_pointer_cas( &list->_head.pointer, entry, head));
}



// based on remove all, removes all then adds back. This is O(n) in the
// retained chain length.
MULLE__LINKEDLIST_GLOBAL MULLE_C_NONNULL_FIRST
struct _mulle_concurrent_linkedlistentry  *
   _mulle_concurrent_linkedlist_remove_one( struct _mulle_concurrent_linkedlist *list);


static inline struct _mulle_concurrent_linkedlistentry  *
   mulle_concurrent_linkedlist_remove_one( struct _mulle_concurrent_linkedlist *list)
{
   if( ! list)
      return( NULL);
   return( _mulle_concurrent_linkedlist_remove_one( list));
}


//
// NOT THREADSAFE AT ALL. The caller must ensure that entries remain valid for
// the complete walk and must not free or reuse them concurrently.
//
// Memory model: reads `_head` non-atomically and traverses `_next` with plain
// loads. There is no synchronization whatsoever, the caller must have
// exclusive access to the list.
//
MULLE__LINKEDLIST_GLOBAL MULLE_C_NONNULL_FIRST_SECOND
int   _mulle_concurrent_linkedlist_walk( struct _mulle_concurrent_linkedlist *list,
                                         int (*callback)( struct _mulle_concurrent_linkedlistentry *,
                                                          struct _mulle_concurrent_linkedlistentry *,
                                                         void *),
                                         void *userinfo);


static inline int
   mulle_concurrent_linkedlist_walk( struct _mulle_concurrent_linkedlist *list,
                                     int (*callback)( struct _mulle_concurrent_linkedlistentry *,
                                                      struct _mulle_concurrent_linkedlistentry *,
                                                      void *),
                                     void *userinfo)
{
   if( ! list)
      return( 0);
   return( _mulle_concurrent_linkedlist_walk( list, callback, userinfo));
}


#endif
