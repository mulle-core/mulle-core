//
//  stress.c
//  mulle-fifo
//
//  Punishing publication + ordering test.
//
//  - 1M items through a small (32-slot) fixed FIFO and a dynamic FIFO
//  - Each item carries a 64-byte payload written by the producer
//  - Consumer validates both content integrity and FIFO ordering
//  - No sleeps: tight spin-retry to maximize contention on the counter
//  - Exercises repeated wraparound (1M / 32 = 31250 full wraps)
//
#include <mulle-fifo/mulle-fifo.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define N_ITEMS         1000000
#define DYNAMIC_SIZE    32


struct payload
{
   intptr_t   sequence;
   intptr_t   check;       // ~sequence
   intptr_t   pad[ 6];     // fill a cache line, stress publication
};


// ---- Fixed FIFO test (32 slots) ----

static struct mulle__pointerfifo32   fixed_fifo;
static struct payload                fixed_items[ N_ITEMS];


static mulle_thread_rval_t   fixed_producer( void *arg)
{
   intptr_t         i;
   struct payload   *p;
   intptr_t         n_items;

   n_items = (intptr_t) arg;

   for( i = 0; i < n_items; i++)
   {
      p = &fixed_items[ i];
      p->sequence = i;
      p->check    = ~i;
      p->pad[ 0]  = i * 3;
      p->pad[ 5]  = i * 7;

      while( _mulle__pointerfifo32_write( &fixed_fifo, p) == -1)
         mulle_thread_yield();
   }

   mulle_thread_return();
}


static int   run_fixed_test( intptr_t n_items)
{
   struct payload   *p;
   intptr_t          expect;
   intptr_t          corrupt;
   intptr_t          received;
   mulle_thread_t    thread;

   _mulle__pointerfifo32_init( &fixed_fifo);

   mulle_thread_create( fixed_producer, (void *) n_items, &thread);

   corrupt  = 0;
   expect   = 0;
   received = 0;

   while( received < n_items)
   {
      p = _mulle__pointerfifo32_read( &fixed_fifo);
      if( ! p)
      {
         mulle_thread_yield();
         continue;
      }

      if( p->sequence != expect)
         ++corrupt;
      if( p->check != ~p->sequence)
         ++corrupt;
      if( p->pad[ 0] != p->sequence * 3)
         ++corrupt;
      if( p->pad[ 5] != p->sequence * 7)
         ++corrupt;

      ++expect;
      ++received;
   }

   mulle_thread_join( thread);
   _mulle__pointerfifo32_done( &fixed_fifo);

   return( (int) corrupt);
}


// ---- Dynamic FIFO test (32 slots) ----

static struct mulle_pointerfifo   dynamic_fifo;
static struct payload             dynamic_items[ N_ITEMS];


static mulle_thread_rval_t   dynamic_producer( void *arg)
{
   intptr_t         i;
   struct payload   *p;
   intptr_t         n_items;

   n_items = (intptr_t) arg;

   for( i = 0; i < n_items; i++)
   {
      p = &dynamic_items[ i];
      p->sequence = i;
      p->check    = ~i;
      p->pad[ 0]  = i * 3;
      p->pad[ 5]  = i * 7;

      while( _mulle_pointerfifo_write( &dynamic_fifo, p) == -1)
         mulle_thread_yield();
   }

   mulle_thread_return();
}


static int   run_dynamic_test( intptr_t n_items)
{
   struct payload   *p;
   intptr_t          expect;
   intptr_t          corrupt;
   intptr_t          received;
   mulle_thread_t    thread;

   _mulle_pointerfifo_init( &dynamic_fifo, DYNAMIC_SIZE, NULL);

   mulle_thread_create( dynamic_producer,  (void *) n_items, &thread);

   corrupt  = 0;
   expect   = 0;
   received = 0;

   while( received < n_items)
   {
      p = _mulle_pointerfifo_read( &dynamic_fifo);
      if( ! p)
      {
         mulle_thread_yield();
         continue;
      }

      if( p->sequence != expect)
         ++corrupt;
      if( p->check != ~p->sequence)
         ++corrupt;
      if( p->pad[ 0] != p->sequence * 3)
         ++corrupt;
      if( p->pad[ 5] != p->sequence * 7)
         ++corrupt;

      ++expect;
      ++received;
   }

   mulle_thread_join( thread);
   _mulle_pointerfifo_done( &dynamic_fifo);

   return( (int) corrupt);
}


int   main( int argc, char *argv[])
{
   int        fixed_corrupt;
   int        dynamic_corrupt;
   intptr_t   n_items = N_ITEMS;

   if( getenv( "MULLE_TEST_VALGRIND") != NULL)
      n_items /= 1000;

   fixed_corrupt   = run_fixed_test( n_items);
   dynamic_corrupt = run_dynamic_test( n_items);

   printf( "fixed32:  %ld items, %d corrupt\n", (long) n_items, fixed_corrupt);
   printf( "dynamic%d: %ld items, %d corrupt\n", DYNAMIC_SIZE, (long) n_items, dynamic_corrupt);

   return( (fixed_corrupt || dynamic_corrupt) ? 1 : 0);
}
