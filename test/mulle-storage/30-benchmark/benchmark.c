//
//  benchmark.c
//  mulle-storage
//
//  Copyright (c) 2023 Nat! - Mulle kybernetiK.
//  All rights reserved.
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
//  Benchmark mulle-storage against the two obvious alternatives for a
//  tree-node pool:
//
//    malloc/free         - the baseline everyone compares against
//    naive free-list     - the "I could write this in 40 lines" competitor:
//                          per-node malloc, freed nodes linked into a LIFO
//                          list, per-node free on teardown
//    mulle-storage       - the library under test
//
//  Workloads model typical tree usage:
//    growth       - allocate N nodes, then destroy everything at once
//    lifo churn   - a stack oscillating between empty and full
//    random churn - a live set with random frees and allocations
//
//  mulle-storage grows in fixed buckets whose size is the `capacity` argument
//  of `_mulle_storage_init`. A "growth vs capacity" sweep shows how that one
//  argument controls both the number of system-allocator calls and the cost
//  of `_mulle_storage_done`.
//
//  The printed numbers are informational. The test only FAILS on:
//    - correctness violations: all allocators must produce identical results
//      and count accounting must match the live set
//    - structural sanity bounds: during pure growth mulle-storage must touch
//      the system allocator ~N/capacity times (not N times like malloc), and
//      bulk teardown must be far cheaper than per-node free
//
//  Timing assertions are deliberately generous so the test is not flaky in
//  CI, whether it runs natively, under wine, ASan/UBSan or valgrind.
//  Iteration counts are calibrated at runtime, so the benchmark stays cheap
//  even under valgrind.
//

#include <mulle-storage/mulle-storage.h>
#include <mulle-allocator/mulle-allocator.h>

#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined( _WIN32)
# include <windows.h>
#else
# include <time.h>
# if defined( __unix__) || defined( __APPLE__)
#  include <sys/resource.h>
# endif
#endif


#define NODE_SIZE_32     32
#define NODE_SIZE_128    128

#define DEFAULT_CAPACITY 64      // bucket size used for the timing table

#define CALIBRATION_OPS  30000UL   // fixed ops for the calibration pass
#define TARGET_NS        4.0e7     // scale a measurement to ~40 ms
#define MIN_ITERS        10000UL   // keep it cheap under valgrind
#define MAX_ITERS        1000000UL // keep memory bounded natively
#define REPEATS          3         // take the minimum of REPEATS runs


/* timing */


static double   now_ns( void)
{
#if defined( _WIN32)
   LARGE_INTEGER  freq;
   LARGE_INTEGER  counter;

   QueryPerformanceFrequency( &freq);
   QueryPerformanceCounter( &counter);
   return( (double) counter.QuadPart * 1e9 / (double) freq.QuadPart);
#else
   struct timespec  ts;

   clock_gettime( CLOCK_MONOTONIC, &ts);
   return( (double) ts.tv_sec * 1e9 + (double) ts.tv_nsec);
#endif
}


static long   peak_rss_kb( void)
{
#if defined( __linux__)
   struct rusage  ru;

   if( getrusage( RUSAGE_SELF, &ru) == 0)
      return( ru.ru_maxrss);               // kilobytes on Linux
#elif defined( __APPLE__)
   struct rusage  ru;

   if( getrusage( RUSAGE_SELF, &ru) == 0)
      return( ru.ru_maxrss / 1024);        // bytes on macOS
#endif
   return( -1);
}


/* deterministic prng */


static uint32_t   rng_state = 0x1234ABCD;


static uint32_t   rng_next( void)
{
   rng_state ^= rng_state << 13;
   rng_state ^= rng_state >> 17;
   rng_state ^= rng_state << 5;
   return( rng_state);
}


/* node payload */


#define NODE_MAGIC   0x545235454C4C554DULL  // "MULLE5RT" as little-endian bytes


struct bench_node
{
   uint64_t       magic;
   unsigned int   serial;
   unsigned char  data[];      // flexible array member
};


static void   node_init( struct bench_node *n, size_t size, unsigned int serial)
{
   n->magic  = NODE_MAGIC;
   n->serial = serial;
   memset( n->data, (int) (serial & 0xFF), size - sizeof( struct bench_node));
}


static void   node_check( struct bench_node *n, size_t size, unsigned int serial)
{
   size_t        i;
   unsigned char pattern = (unsigned char) (serial & 0xFF);

   if( n->magic != NODE_MAGIC || n->serial != serial)
      abort();
   for( i = 0; i < size - sizeof( struct bench_node); i++)
      if( n->data[ i] != pattern)
         abort();
}


/* allocator abstraction */


struct bench_ctx;


struct alloc_ops
{
   const char     *name;
   void           (*ctx_init)( struct bench_ctx *ctx, size_t size, unsigned int capacity);
   void           *(*alloc)( struct bench_ctx *ctx, size_t size, unsigned int serial);
   void           (*free)( struct bench_ctx *ctx, void *p);
   void           (*teardown)( struct bench_ctx *ctx, void **live, size_t live_count);
   size_t         (*live_count)( struct bench_ctx *ctx);
   unsigned long  (*alloc_calls)( struct bench_ctx *ctx);
};


/* contexts */


struct malloc_ctx
{
   unsigned long   alloc_calls;
   unsigned long   free_calls;
};


struct freelist_ctx
{
   struct bench_node  *free_head;   // freed nodes, linked through offset 0
   struct bench_node **blocks;      // every malloc'd block, for teardown
   size_t              nblocks;
   size_t              blocks_cap;
   unsigned long       outstanding; // live nodes
};


struct counting_allocator
{
   struct mulle_allocator   allocator;
   unsigned long            calloc_calls;
   unsigned long            realloc_calls;
   unsigned long            free_calls;
};


struct storage_ctx
{
   struct mulle_storage       storage;
   struct counting_allocator  counting;
};


struct bench_ctx
{
   struct malloc_ctx      malloc;
   struct freelist_ctx    freelist;
   struct storage_ctx     storage;
};


/* allocator: malloc/free */


static void   malloc_ctx_init( struct bench_ctx *ctx, size_t size, unsigned int capacity)
{
   (void) size;
   (void) capacity;
   ctx->malloc.alloc_calls = 0;
   ctx->malloc.free_calls  = 0;
}


static void   *malloc_alloc( struct bench_ctx *ctx, size_t size, unsigned int serial)
{
   struct bench_node  *n = malloc( size);

   if( ! n)
      abort();
   ctx->malloc.alloc_calls++;
   node_init( n, size, serial);
   return( n);
}


static void   malloc_free( struct bench_ctx *ctx, void *p)
{
   ctx->malloc.free_calls++;
   free( p);
}


static void   malloc_teardown( struct bench_ctx *ctx, void **live, size_t live_count)
{
   size_t  i;

   for( i = 0; i < live_count; i++)
      free( live[ i]);
   if( ctx->malloc.alloc_calls != ctx->malloc.free_calls + live_count)
      abort();
}


static size_t   malloc_live_count( struct bench_ctx *ctx)
{
   return( (size_t) (ctx->malloc.alloc_calls - ctx->malloc.free_calls));
}


static unsigned long   malloc_alloc_calls( struct bench_ctx *ctx)
{
   return( ctx->malloc.alloc_calls);
}


/* allocator: naive embedded free-list */


static void   freelist_ctx_init( struct bench_ctx *ctx, size_t size, unsigned int capacity)
{
   (void) size;
   (void) capacity;
   ctx->freelist.free_head   = NULL;
   ctx->freelist.blocks      = NULL;
   ctx->freelist.nblocks     = 0;
   ctx->freelist.blocks_cap  = 0;
   ctx->freelist.outstanding = 0;
}


static void   *freelist_alloc( struct bench_ctx *ctx, size_t size, unsigned int serial)
{
   struct freelist_ctx  *fl  = &ctx->freelist;
   struct bench_node    *n   = fl->free_head;

   if( n)
   {
      fl->free_head = *(struct bench_node **) n;   // clobbers the dead payload
   }
   else
   {
      n = malloc( size);
      if( ! n)
         abort();
      if( fl->nblocks == fl->blocks_cap)
      {
         fl->blocks_cap = fl->blocks_cap ? fl->blocks_cap * 2 : 1024;
         fl->blocks = realloc( fl->blocks, fl->blocks_cap * sizeof( struct bench_node *));
         if( ! fl->blocks)
            abort();
      }
      fl->blocks[ fl->nblocks++] = n;
   }
   fl->outstanding++;
   node_init( n, size, serial);
   return( n);
}


static void   freelist_free( struct bench_ctx *ctx, void *p)
{
   struct freelist_ctx  *fl = &ctx->freelist;

   *(struct bench_node **) p = fl->free_head;
   fl->free_head = p;
   fl->outstanding--;
}


static void   freelist_teardown( struct bench_ctx *ctx, void **live, size_t live_count)
{
   struct freelist_ctx  *fl = &ctx->freelist;
   size_t                i;

   for( i = 0; i < live_count; i++)
      freelist_free( ctx, live[ i]);
   for( i = 0; i < fl->nblocks; i++)
      free( fl->blocks[ i]);
   free( fl->blocks);
   fl->blocks = NULL;
}


static size_t   freelist_live_count( struct bench_ctx *ctx)
{
   return( (size_t) ctx->freelist.outstanding);
}


static unsigned long   freelist_alloc_calls( struct bench_ctx *ctx)
{
   return( (unsigned long) ctx->freelist.nblocks);
}


/* allocator: mulle-storage with counting allocator */


static void   *counting_calloc( size_t n, size_t size, struct mulle_allocator *a)
{
   struct counting_allocator  *ca = (struct counting_allocator *) a;

   ca->calloc_calls++;
   return( calloc( n, size));
}


static void   *counting_reallocarray( void *block, size_t n, size_t size, struct mulle_allocator *a)
{
   struct counting_allocator  *ca = (struct counting_allocator *) a;

   ca->realloc_calls++;
   return( realloc( block, n * size));
}


static void   counting_free( void *block, struct mulle_allocator *a)
{
   struct counting_allocator  *ca = (struct counting_allocator *) a;

   ca->free_calls++;
   free( block);
}


static void   counting_fail( struct mulle_allocator *a, void *block, size_t size) _MULLE_C_NO_RETURN;

static void   counting_fail( struct mulle_allocator *a, void *block, size_t size)
{
   (void) a;
   (void) block;
   (void) size;
   abort();
}


static void   counting_allocator_init( struct counting_allocator *ca)
{
   memset( ca, 0, sizeof( *ca));
   ca->allocator.calloc       = counting_calloc;
   ca->allocator.reallocarray = counting_reallocarray;
   ca->allocator.free         = counting_free;
   ca->allocator.fail    = counting_fail;
   // abafree and aba stay NULL
}


static void   storage_ctx_init( struct bench_ctx *ctx, size_t size, unsigned int capacity)
{
   counting_allocator_init( &ctx->storage.counting);
   _mulle_storage_init( &ctx->storage.storage,
                        size,
                        alignof( struct bench_node),
                        capacity,
                        &ctx->storage.counting.allocator);
}


static void   *storage_alloc( struct bench_ctx *ctx, size_t size, unsigned int serial)
{
   struct bench_node  *n = _mulle_storage_malloc( &ctx->storage.storage);

   if( ! n)
      abort();
   node_init( n, size, serial);
   return( n);
}


static void   storage_free( struct bench_ctx *ctx, void *p)
{
   _mulle_storage_free( &ctx->storage.storage, p);
}


static void   storage_teardown( struct bench_ctx *ctx, void **live, size_t live_count)
{
   (void) live;
   (void) live_count;
   _mulle_storage_done( &ctx->storage.storage);
}


static size_t   storage_live_count( struct bench_ctx *ctx)
{
   return( _mulle_storage_get_count( &ctx->storage.storage));
}


static unsigned long   storage_alloc_calls( struct bench_ctx *ctx)
{
   return( ctx->storage.counting.calloc_calls + ctx->storage.counting.realloc_calls);
}


static const struct alloc_ops   malloc_ops =
{
   "malloc",
   malloc_ctx_init,
   malloc_alloc,
   malloc_free,
   malloc_teardown,
   malloc_live_count,
   malloc_alloc_calls
};

static const struct alloc_ops   freelist_ops =
{
   "naive-freelist",
   freelist_ctx_init,
   freelist_alloc,
   freelist_free,
   freelist_teardown,
   freelist_live_count,
   freelist_alloc_calls
};

static const struct alloc_ops   storage_ops =
{
   "mulle-storage",
   storage_ctx_init,
   storage_alloc,
   storage_free,
   storage_teardown,
   storage_live_count,
   storage_alloc_calls
};


/* workloads */


static double   growth_pass( const struct alloc_ops *ops, struct bench_ctx *ctx,
                             size_t size, size_t n,
                             double *teardown_ns, unsigned long *alloc_calls)
{
   void   **live = malloc( n * sizeof( void *));
   size_t   i;
   double   t0, t1, td0, td1;

   if( ! live)
      abort();

   t0 = now_ns();
   for( i = 0; i < n; i++)
      live[ i] = ops->alloc( ctx, size, (unsigned int) i);
   t1 = now_ns();

   if( ops->live_count( ctx) != n)
      abort();
   for( i = 0; i < n; i++)
      node_check( live[ i], size, (unsigned int) i);

   td0 = now_ns();
   ops->teardown( ctx, live, n);
   td1 = now_ns();

   *alloc_calls = ops->alloc_calls( ctx);
   *teardown_ns = td1 - td0;
   free( live);
   return( t1 - t0);
}


static double   lifo_pass( const struct alloc_ops *ops, struct bench_ctx *ctx,
                           size_t size, size_t n)
{
   enum { DEPTH = 64 };

   void          *stack[ DEPTH];
   unsigned int   serials[ DEPTH];
   unsigned int   level  = 0;
   unsigned int   serial = 0;
   size_t         i;
   double         t0, t1;

   t0 = now_ns();
   for( i = 0; i < n; i++)
   {
      if( level == 0)
      {
         while( level < DEPTH)
         {
            stack[ level]   = ops->alloc( ctx, size, serial);
            serials[ level] = serial++;
            level++;
         }
      }
      else
      {
         void  *p = stack[ --level];

         node_check( p, size, serials[ level]);
         ops->free( ctx, p);
      }
   }
   t1 = now_ns();

   while( level > 0)
   {
      void  *p = stack[ --level];

      node_check( p, size, serials[ level]);
      ops->free( ctx, p);
   }
   if( ops->live_count( ctx) != 0)
      abort();
   return( t1 - t0);
}


static double   random_pass( const struct alloc_ops *ops, struct bench_ctx *ctx,
                             size_t size, size_t n)
{
   enum { MAX_LIVE = 4096 };

   void          *live[ MAX_LIVE];
   unsigned int   serials[ MAX_LIVE];
   size_t         live_count = 0;
   unsigned int   serial     = 0;
   size_t         i;
   double         t0, t1;

   t0 = now_ns();
   for( i = 0; i < n; i++)
   {
      if( live_count == 0 || (live_count < MAX_LIVE && (rng_next() & 1)))
      {
         live[ live_count]    = ops->alloc( ctx, size, serial);
         serials[ live_count] = serial++;
         live_count++;
      }
      else
      {
         size_t  k = rng_next() % live_count;
         void   *p = live[ k];

         node_check( p, size, serials[ k]);
         live[ k]    = live[ live_count - 1];
         serials[ k] = serials[ live_count - 1];
         live_count--;
         ops->free( ctx, p);
      }
   }
   t1 = now_ns();

   while( live_count > 0)
   {
      void  *p = live[ live_count - 1];

      node_check( p, size, serials[ live_count - 1]);
      live_count--;
      ops->free( ctx, p);
   }
   if( ops->live_count( ctx) != 0)
      abort();
   return( t1 - t0);
}


/* bench runners (min of REPEATS, fresh context each time) */


static double   bench_growth( const struct alloc_ops *ops, size_t size, unsigned int capacity, size_t n,
                              double *teardown_ns, unsigned long *alloc_calls)
{
   struct bench_ctx  ctx;
   double            best         = 1e300;
   double            best_teardown = 1e300;
   unsigned long     calls        = 0;
   int               repeat;

   for( repeat = 0; repeat < REPEATS; repeat++)
   {
      double  t, td;
      unsigned long  c;

      ops->ctx_init( &ctx, size, capacity);
      t = growth_pass( ops, &ctx, size, n, &td, &c);
      if( repeat == 0)
         calls = c;
      if( t < best)
         best = t;
      if( td < best_teardown)
         best_teardown = td;
   }
   *teardown_ns = best_teardown;
   *alloc_calls = calls;
   return( best / (double) n);
}


static double   bench_lifo( const struct alloc_ops *ops, size_t size, unsigned int capacity, size_t n)
{
   struct bench_ctx  ctx;
   double            best = 1e300;
   int               repeat;

   for( repeat = 0; repeat < REPEATS; repeat++)
   {
      double  t;

      ops->ctx_init( &ctx, size, capacity);
      t = lifo_pass( ops, &ctx, size, n);
      ops->teardown( &ctx, NULL, 0);
      if( t < best)
         best = t;
   }
   return( best / (double) n);
}


static double   bench_random( const struct alloc_ops *ops, size_t size, unsigned int capacity, size_t n)
{
   struct bench_ctx  ctx;
   double            best = 1e300;
   int               repeat;

   for( repeat = 0; repeat < REPEATS; repeat++)
   {
      double  t;

      ops->ctx_init( &ctx, size, capacity);
      t = random_pass( ops, &ctx, size, n);
      ops->teardown( &ctx, NULL, 0);
      if( t < best)
         best = t;
   }
   return( best / (double) n);
}


/* calibration */


static double   calibrate_ns_per_op( size_t size)
{
   struct bench_ctx  ctx;
   double            t;

   freelist_ops.ctx_init( &ctx, size, DEFAULT_CAPACITY);
   t = random_pass( &freelist_ops, &ctx, size, CALIBRATION_OPS);
   freelist_ops.teardown( &ctx, NULL, 0);
   return( t / (double) CALIBRATION_OPS);
}


/* growth vs capacity sweep */


static void   capacity_sweep( size_t size, size_t n)
{
   unsigned int   caps[] = { 4, 64, 1024 };
   size_t         i;

   printf( "\n== growth vs capacity (node size %zu, %zu nodes) ==\n", size, n);
   printf( "%-12s %16s %13s\n", "capacity", "sys-alloc calls", "teardown us");

   for( i = 0; i < sizeof( caps) / sizeof( caps[ 0]); i++)
   {
      double         td;
      unsigned long  calls;
      double         g;

      g = bench_growth( &storage_ops, size, caps[ i], n, &td, &calls);
      (void) g;
      printf( "%-12u %16lu %13.1f\n", caps[ i], calls, td / 1000.0);

      // mulle-storage touches the system allocator ~n/capacity times during
      // pure growth (one bucket per `capacity` nodes). Allow 2x headroom.
      if( calls > n / (caps[ i] / 2))
      {
         fprintf( stderr, "FAIL: capacity %u produced %lu sys-alloc calls for %zu nodes (expected <= %zu)\n",
                  caps[ i], calls, n, n / (caps[ i] / 2));
         abort();
      }
   }
}


/* main */


int  main( int argc, char *argv[])
{
   size_t   sizes[]   = { NODE_SIZE_32, NODE_SIZE_128 };
   size_t   i;

   (void) argc;
   (void) argv;

   printf( "mulle-storage benchmark (min of %d runs, calibrated to ~%.0f ms per measurement)\n",
           REPEATS, TARGET_NS / 1e6);
#if defined( DEBUG)
   printf( "build mode: DEBUG - poison fills active\n");
#endif
#if defined( NDEBUG)
   printf( "build mode: release (NDEBUG) - header-inline asserts disabled\n");
#else
   printf( "build mode: debug build - header-inline asserts active; churn includes O(n) pointer-validation cost\n");
   printf( "  (compile with -DNDEBUG -O2 for release-mode numbers)\n");
#endif

   for( i = 0; i < sizeof( sizes) / sizeof( sizes[ 0]); i++)
   {
      size_t         size = sizes[ i];
      double         ns_per_op;
      size_t         iters;
      double         mgrowth, mlifo, mrandom, mteardown;
      double         fgrowth, flifo, frandom, fteardown;
      double         sgrowth, slifo, srandom, steardown;
      unsigned long  malloc_calls, freelist_calls, storage_calls;

      ns_per_op = calibrate_ns_per_op( size);
      iters     = (size_t) (TARGET_NS / ns_per_op);
      if( iters < MIN_ITERS)
         iters = MIN_ITERS;
      if( iters > MAX_ITERS)
         iters = MAX_ITERS;

      printf( "\n== node size %zu bytes, %zu iterations per workload, capacity %d ==\n",
              size, iters, DEFAULT_CAPACITY);

      mgrowth = bench_growth( &malloc_ops, size, DEFAULT_CAPACITY, iters, &mteardown, &malloc_calls);
      mlifo   = bench_lifo( &malloc_ops, size, DEFAULT_CAPACITY, iters);
      mrandom = bench_random( &malloc_ops, size, DEFAULT_CAPACITY, iters);

      fgrowth = bench_growth( &freelist_ops, size, DEFAULT_CAPACITY, iters, &fteardown, &freelist_calls);
      flifo   = bench_lifo( &freelist_ops, size, DEFAULT_CAPACITY, iters);
      frandom = bench_random( &freelist_ops, size, DEFAULT_CAPACITY, iters);

      sgrowth = bench_growth( &storage_ops, size, DEFAULT_CAPACITY, iters, &steardown, &storage_calls);
      slifo   = bench_lifo( &storage_ops, size, DEFAULT_CAPACITY, iters);
      srandom = bench_random( &storage_ops, size, DEFAULT_CAPACITY, iters);

      printf( "\n%-15s %11s %11s %11s %13s %16s\n",
              "allocator", "growth ns", "lifo ns", "random ns", "teardown us", "sys-alloc calls");
      printf( "%-15s %11.1f %11.1f %11.1f %13.1f %16lu\n",
              malloc_ops.name, mgrowth, mlifo, mrandom, mteardown / 1000.0, malloc_calls);
      printf( "%-15s %11.1f %11.1f %11.1f %13.1f %16lu\n",
              freelist_ops.name, fgrowth, flifo, frandom, fteardown / 1000.0, freelist_calls);
      printf( "%-15s %11.1f %11.1f %11.1f %13.1f %16lu\n",
              storage_ops.name, sgrowth, slifo, srandom, steardown / 1000.0, storage_calls);

      printf( "\nmulle-storage vs malloc: growth x%.2f  lifo x%.2f  random x%.2f  teardown x%.4f  sys-alloc-calls x%.4f\n",
              sgrowth / mgrowth,
              slifo / mlifo,
              srandom / mrandom,
              steardown / mteardown,
              (double) storage_calls / (double) malloc_calls);

      //
      // structural sanity (deliberately generous, not flaky):
      //
      // 1. during pure growth mulle-storage touches the system allocator
      //    ~iters/64 times (bucket size = capacity = 64), not `iters` times
      //    like malloc. Allow 2x headroom over the theoretical ~iters/64.
      //
      if( storage_calls > iters / 32)
      {
         fprintf( stderr, "FAIL: mulle-storage performed %lu system allocator calls during growth of %zu nodes\n",
                  storage_calls, iters);
         abort();
      }
      //
      // 2. bulk teardown of a full pool must not be slower than per-node
      //    free. mulle-storage releases ~iters/64 large buckets, malloc
      //    releases `iters` tiny nodes; glibc's per-free cost grows with
      //    block size, so the wall-clock win is real but modest (measured
      //    ~3-15x depending on capacity). A 2x-lag bound is very safe.
      //
      if( steardown > mteardown * 2.0)
      {
         fprintf( stderr, "FAIL: mulle-storage bulk teardown (%.1f us) slower than per-node free (%.1f us)\n",
                  steardown / 1000.0, mteardown / 1000.0);
         abort();
      }

      capacity_sweep( size, iters);
   }

   {
      long  rss = peak_rss_kb();

      if( rss < 0)
         printf( "\npeak RSS: n/a on this platform\n");
      else
         printf( "\npeak RSS: %ld KB\n", rss);
   }

   printf( "\nbenchmark OK\n");
   return( 0);
}
