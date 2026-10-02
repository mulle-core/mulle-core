//
//  ll-publish.c
//  mulle-linkedlist
//
//  Copyright (c) 2026 Nat! - Mulle kybernetiK.
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
//
// Publication ordering test for struct _mulle_concurrent_linkedlist, see
// dox/MEMORY_MODEL.md.
//
// Every entry carries a *non-atomic* payload that is written before add and
// verified after removal. The only thing that makes this well defined is the
// publication ordering of the list: the seq_cst CAS in add releases the
// payload write, the seq_cst CAS in remove_all/remove_one acquires it. If the
// head CAS were weakened, or if the speculative relaxed head read were used
// for anything that gets dereferenced, this test detects it as a torn payload
// and ThreadSanitizer reports it as a data race on the payload.
//
// The entry pool is allocated up front and freed only after all threads have
// joined. That is the quiescent-point reclamation discipline, which this list
// requires and does not provide itself.
//
// NOTE: on x86/x64 a plain run of this test proves little, the hardware is
// strongly ordered. It only becomes a real memory-model check under
// ThreadSanitizer (build with -DMULLE_THREAD_USE_PTHREADS, tsan does not
// intercept the C11 thrd_create) or on a weakly ordered CPU.
//
#include <mulle-linkedlist/mulle-linkedlist.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifndef LOOPS
# if defined( MULLE_TEST)
#  define LOOPS   200      // MULLE_THREAD_UNPLEASANT_RACE_YIELD is slow
# else
#  define LOOPS   20000
# endif
#endif

#ifndef N_THREADS
# define N_THREADS   4
#endif

#ifndef N_ENTRIES
# define N_ENTRIES   64
#endif

#define MAGIC   0xfeedfaceUL


struct demo_entry
{
   struct _mulle_concurrent_linkedlistentry   _link;

   //
   // deliberately non-atomic, ordered only by the list's publication
   // guarantee
   //
   unsigned long   magic;
   unsigned long   payload;
   unsigned long   check;
};


static struct _mulle_concurrent_linkedlist   list;
static struct demo_entry                     *pool[ N_THREADS * N_ENTRIES];
static mulle_atomic_pointer_t                n_threads;
static mulle_atomic_pointer_t                n_errors;


static void   publish( struct demo_entry *entry, unsigned long value)
{
   entry->magic   = MAGIC;
   entry->payload = value;
   entry->check   = value ^ MAGIC;
}


static void   consume( struct demo_entry *entry)
{
   if( entry->magic != MAGIC || entry->check != (entry->payload ^ MAGIC))
      _mulle_atomic_pointer_increment( &n_errors);
}


static void   wait_around( void)
{
   _mulle_atomic_pointer_decrement( &n_threads);
   while( _mulle_atomic_pointer_read( &n_threads) != 0)
      mulle_thread_yield();
}


static mulle_thread_rval_t   run_test( void *arg)
{
   struct demo_entry   *entry;
   unsigned long       i;

   MULLE_C_UNUSED( arg);

   wait_around();

   for( i = 0; i < LOOPS; i++)
   {
      //
      // alternate between single removal and bulk detach, so both consumption
      // paths are exercised
      //
      if( i & 1)
      {
         entry = (void *) _mulle_concurrent_linkedlist_remove_one( &list);
         if( ! entry)
            continue;

         assert( ! entry->_link._next);
         consume( entry);
         publish( entry, i);
         _mulle_concurrent_linkedlist_add( &list, &entry->_link);
         continue;
      }

      entry = (void *) _mulle_concurrent_linkedlist_remove_all( &list);
      while( entry)
      {
         struct demo_entry   *next;

         next = (void *) entry->_link._next;
         entry->_link._next = NULL;

         consume( entry);
         publish( entry, i);
         _mulle_concurrent_linkedlist_add( &list, &entry->_link);

         entry = next;
      }
   }

   return( (mulle_thread_rval_t) 0);
}


static int   count_callback( struct _mulle_concurrent_linkedlistentry *entry,
                             struct _mulle_concurrent_linkedlistentry *prev,
                             void *userinfo)
{
   MULLE_C_UNUSED( prev);

   consume( (struct demo_entry *) entry);
   ++*(unsigned int *) userinfo;
   return( 0);
}


static int   _main( void)
{
   mulle_thread_t       threads[ N_THREADS];
   struct demo_entry    *entry;
   unsigned int         count;
   unsigned int         i;

   _mulle_concurrent_linkedlist_init( &list);
   memset( &n_errors, 0, sizeof( n_errors));

   for( i = 0; i < N_THREADS * N_ENTRIES; i++)
   {
      entry = mulle_allocator_calloc( &mulle_allocator_default, 1, sizeof( *entry));
      publish( entry, i);
      pool[ i] = entry;
      _mulle_concurrent_linkedlist_add( &list, &entry->_link);
   }

   _mulle_atomic_pointer_nonatomic_write( &n_threads, (void *) (intptr_t) N_THREADS);

   for( i = 0; i < N_THREADS; i++)
      if( mulle_thread_create( run_test, NULL, &threads[ i]))
         return( 1);

   for( i = 0; i < N_THREADS; i++)
      if( mulle_thread_join( threads[ i]))
         return( 1);

   //
   // quiescent point: only now may the list be walked non-atomically and the
   // entries be freed
   //
   count = 0;
   _mulle_concurrent_linkedlist_walk( &list, count_callback, &count);
   if( count != N_THREADS * N_ENTRIES)
   {
      fprintf( stderr, "walk found %u entries, expected %u\n",
                       count, (unsigned int) (N_THREADS * N_ENTRIES));
      return( 1);
   }

   count = 0;
   while( (entry = (void *) _mulle_concurrent_linkedlist_remove_one( &list)))
   {
      consume( entry);
      ++count;
   }
   if( count != N_THREADS * N_ENTRIES)
   {
      fprintf( stderr, "drained %u entries, expected %u\n",
                       count, (unsigned int) (N_THREADS * N_ENTRIES));
      return( 1);
   }

   _mulle_concurrent_linkedlist_done( &list);

   if( _mulle_atomic_pointer_read( &n_errors))
   {
      fprintf( stderr, "%ld torn payloads, publication ordering is broken\n",
                       (long) (intptr_t) _mulle_atomic_pointer_read( &n_errors));
      return( 1);
   }

   for( i = 0; i < N_THREADS * N_ENTRIES; i++)
      mulle_allocator_free( &mulle_allocator_default, pool[ i]);


   return( 0);
}


#ifndef NO_MAIN
int   main( void)
{
   return( _main());
}
#endif
