//
//  timebudget.h
//
//  Wall clock budget for the stress tests.
//
//  A stress test with a fixed iteration count encodes the speed of the machine
//  it was tuned on: the same loop is seconds with one compiler and times out
//  with another (tcc, valgrind, a 32-bit build, a loaded CI runner). A time
//  budget says "hammer for N seconds" instead: a fast build gets more
//  iterations, a slow build fewer, and the test can never overrun its budget.
//  The mulle-sprintf benchmarks calibrate the same way, by doubling the work
//  until their budget is spent.
//
//  The budget defaults to MULLE_TEST_BUDGET_DEFAULT seconds. Don't overrun the
//  ctest TIMEOUT (120s) - the budget needs headroom so a loaded machine does
//  not turn a correct test into a timeout.
//
//  The budget can be shortened for quick runs with the MULLE_TEST_BUDGET
//  environment variable (in seconds). Under valgrind it is cut down
//  automatically, because there the threads are serialized and every operation
//  is orders of magnitude slower.
//
//  Usage: start the budget once, then let every worker poll it.
//
//     struct mulle_test_budget   budget;
//     unsigned long              i;
//
//     mulle_test_budget_start( &budget, mulle_test_budget_seconds());
//     for( i = 0; ! mulle_test_budget_expired_at( &budget, i); i++)
//        do_one_stressful_thing();
//     mulle_test_budget_report( &budget, "lookup_race", i);
//
//  The budget struct is written once before the worker threads are created and
//  only read afterwards, so no locking is needed. `_expired_at` reads the clock
//  once per MULLE_TEST_BUDGET_GRANULARITY iterations, so the clock does not
//  dominate the loop.
//
//  Only put the budget on loops that do a bounded amount of work per
//  iteration: a test that allocates a new key per iteration would otherwise
//  grow without limit for as long as the budget lasts.
//

#ifndef mulle_test_timebudget_h__
#define mulle_test_timebudget_h__

#include <mulle-time/mulle-time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MULLE_TEST_BUDGET_DEFAULT       60.0
#define MULLE_TEST_BUDGET_VALGRIND       5.0
#define MULLE_TEST_BUDGET_GRANULARITY    1024   // iterations between clock reads

#if defined( __GNUC__) || defined( __clang__)
# define MULLE_TEST_BUDGET_UNUSED  __attribute__( ( unused))
#else
# define MULLE_TEST_BUDGET_UNUSED
#endif


struct mulle_test_budget
{
   double   start;
   double   deadline;
};


//
// Detect valgrind once: all threads are serialized there, so the budget and
// the thread counts must shrink. Detected via the environment (CI) or the
// vgpreload library in the process maps (Linux).
//
static int   mulle_test_budget_is_valgrind( void) MULLE_TEST_BUDGET_UNUSED;

static int   mulle_test_budget_is_valgrind( void)
{
   static int   is_valgrind = -1;

   if( is_valgrind < 0)
   {
      is_valgrind = 0;
      if( getenv( "MULLE_TEST_VALGRIND"))
         is_valgrind = 1;
#ifdef __linux__
      else
      {
         FILE   *f;
         char   line[ 256];

         f = fopen( "/proc/self/maps", "r");
         if( f)
         {
            while( fgets( line, sizeof( line), f))
               if( strstr( line, "vgpreload"))
               {
                  is_valgrind = 1;
                  break;
               }
            fclose( f);
         }
      }
#endif
   }

   return( is_valgrind);
}


//
// Total budget in seconds for one test executable. MULLE_TEST_BUDGET wins,
// then the valgrind default, then the regular default.
//
static double   mulle_test_budget_seconds( void) MULLE_TEST_BUDGET_UNUSED;

static double   mulle_test_budget_seconds( void)
{
   char     *s;
   double   seconds;

   s = getenv( "MULLE_TEST_BUDGET");
   if( s && *s)
   {
      seconds = atof( s);
      if( seconds > 0.0)
         return( seconds);
   }

   if( mulle_test_budget_is_valgrind())
      return( MULLE_TEST_BUDGET_VALGRIND);

   return( MULLE_TEST_BUDGET_DEFAULT);
}


static void   mulle_test_budget_start( struct mulle_test_budget *budget,
                                       double seconds) MULLE_TEST_BUDGET_UNUSED;

static void   mulle_test_budget_start( struct mulle_test_budget *budget,
                                       double seconds)
{
   budget->start    = mulle_relativetime_now();
   budget->deadline = budget->start + seconds;
}


static double   mulle_test_budget_elapsed( struct mulle_test_budget *budget) MULLE_TEST_BUDGET_UNUSED;

static double   mulle_test_budget_elapsed( struct mulle_test_budget *budget)
{
   return( mulle_relativetime_now() - budget->start);
}


static int   mulle_test_budget_expired( struct mulle_test_budget *budget) MULLE_TEST_BUDGET_UNUSED;

static int   mulle_test_budget_expired( struct mulle_test_budget *budget)
{
   return( mulle_relativetime_now() >= budget->deadline);
}


//
// Loop guard: clock read only every MULLE_TEST_BUDGET_GRANULARITY iterations,
// so overshoot is bounded by that many iterations.
//
static int   mulle_test_budget_expired_at( struct mulle_test_budget *budget,
                                           unsigned long i) MULLE_TEST_BUDGET_UNUSED;

static int   mulle_test_budget_expired_at( struct mulle_test_budget *budget,
                                           unsigned long i)
{
   if( i % MULLE_TEST_BUDGET_GRANULARITY)
      return( 0);

   return( mulle_test_budget_expired( budget));
}


//
// Print the outcome to stderr: stdout is compared against a golden file and
// must stay deterministic, the iteration count is not.
//
static void   mulle_test_budget_report( struct mulle_test_budget *budget,
                                        char *label,
                                        unsigned long iterations) MULLE_TEST_BUDGET_UNUSED;

static void   mulle_test_budget_report( struct mulle_test_budget *budget,
                                        char *label,
                                        unsigned long iterations)
{
   fprintf( stderr, "%s: %.1fs, %lu iterations (%.0f/s)\n",
            label,
            mulle_test_budget_elapsed( budget),
            iterations,
            mulle_test_budget_elapsed( budget) > 0.0
               ? (double) iterations / mulle_test_budget_elapsed( budget)
               : 0.0);
}


#endif /* mulle_test_timebudget_h__ */
