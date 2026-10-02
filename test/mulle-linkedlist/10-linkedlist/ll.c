//
//  main.c
//  mulle-aba-test
//
//  Created by Nat! on 19.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
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
#include <mulle-linkedlist/mulle-linkedlist.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>

//
// when we compile with DEBUG and MULLE_TEST we have the MULLE_THREAD_UNPLEASANT_RACE_YIELD
// to catch errors, which is REALLY slow
//
#define PROGRESS     0
#define FOREVER      0

#ifndef ITERATIONS
# if defined( MULLE_TEST)
#  define LOOPS        1000   // (1 + (rand() % 100000))
#  define ITERATIONS   10
# else
#  define LOOPS        100000   // (1 + (rand() % 100000))
#  define ITERATIONS   100
# endif
#endif

#ifndef LOOPS
# define LOOPS  10
#endif
#ifndef ITERATIONS
# define ITERATIONS  1
#endif

#define MAX_THREADS  1




#pragma mark - global variables

static struct _mulle_linkedlist   list;     // common

#pragma mark - reset allocator between tests

static void   reset_memory()
{
   // use library to track allocations

   memset( &list, 0, sizeof( list));
}


struct demo_entry   
{
   struct _mulle_linkedlistentry   _link;

   void   *_pointer;
};



#pragma mark - run test

static void    run_test( void)
{
   struct demo_entry   *entry;
   unsigned long       i;

   for( i = 0; i < LOOPS; i++)
   {
      if( i % 4 == 0)
      {
         entry = (void *) _mulle_linkedlist_remove_one( &list);
         if( entry)
         {
            assert( ! entry->_link._next);
            mulle_allocator_free( &mulle_allocator_default, entry);
         }
      }

      entry = mulle_allocator_calloc( &mulle_allocator_default, 1, sizeof( *entry));

      assert( entry);
      entry->_pointer = (void *) i;

      _mulle_linkedlist_add( &list, &entry->_link);
   }

   _mulle_linkedlistentry_walk( list._head._next,
                                (mulle_linkedlistentry_walk_callback_t *)
                                mulle_allocator_free,
                                &mulle_allocator_default);
   _mulle_linkedlist_remove_all( &list);
}



#ifdef __APPLE__
#include <sys/resource.h>

MULLE_C_CONSTRUCTOR(__enable_core_dumps)
static void  __enable_core_dumps(void)
{
    struct rlimit   limit;

    limit.rlim_cur = RLIM_INFINITY;
    limit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &limit);
}
#endif


static int   _main(int argc, const char * argv[])
{
   unsigned int   i;

   srand( (unsigned int) time( NULL));

#if FOREVER
   fprintf( stderr, "This test runs forever, waiting for a crash\n");
#endif

#if FOREVER
forever:
#endif
   reset_memory();

   //
   // if there are leaks anywhere, it will assert in
   // _mulle_storage_done which is called by reset_memory
   // eventually
   //

   for( i = 0; i < ITERATIONS; i++)
   {
#if MULLE_ABA_TRACE || PROGRESS
# if MULLE_ABA_TRACE
      fprintf( stderr, "iteration %d of %d\n", i, ITERATIONS);
# else
      fprintf( stdout, "iteration %d of %d\n", i, ITERATIONS);
# endif
#endif
      run_test();
      reset_memory();
   }

#if FOREVER
   goto forever;
#endif

   return( 0);
}


#ifndef NO_MAIN
int   main(int argc, const char * argv[])
{
   return( _main( argc, argv));
}
#endif
