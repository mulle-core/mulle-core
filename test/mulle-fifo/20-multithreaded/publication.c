#include <mulle-fifo/mulle-fifo.h>

#include <time.h>


#define N_ITEMS   1024

struct item
{
   intptr_t   value;
   intptr_t   check;
};


//
// the producer fills in the item contents and then hands the pointer
// to the consumer. The consumer validates both the contents (visibility)
// and the sequence (fifo order). Output is deterministic.
//
static struct item               items[ N_ITEMS];
static struct mulle_pointerfifo  fifo;


static mulle_thread_rval_t   producer( void *arg)
{
   intptr_t         i;
   struct timespec  delay;

   for( i = 0; i < N_ITEMS; i++)
   {
      items[ i].value = i;
      items[ i].check = ~i;
      while( _mulle_pointerfifo_write( &fifo, &items[ i]) == -1)
      {
         delay.tv_sec  = 0;
         delay.tv_nsec = 1000;
         nanosleep( &delay, NULL);
      }
   }

   mulle_thread_return();
}


int  main( int argc, char *argv[])
{
   struct item       *item;
   intptr_t          corrupt;
   intptr_t          expect;
   struct timespec   delay;
   int               received;
   mulle_thread_t    producer_thread;

   _mulle_pointerfifo_init( &fifo, 256, NULL);

   mulle_thread_create( producer, NULL, &producer_thread);

   corrupt  = 0;
   expect   = 0;
   received = 0;
   while( received < N_ITEMS)
   {
      item = _mulle_pointerfifo_read( &fifo);
      if( ! item)
      {
         delay.tv_sec  = 0;
         delay.tv_nsec = 1000;
         nanosleep( &delay, NULL);
         continue;
      }

      /* contents written before the write must be visible after the read */
      if( item->check != ~item->value)
         ++corrupt;

      /* items must arrive in the order they were written */
      if( item->value != expect)
         ++corrupt;
      else
         ++expect;

      ++received;
   }

   mulle_thread_join( producer_thread);
   _mulle_pointerfifo_done( &fifo);

   printf( "received %d items, %ld corrupt\n", received, (long) corrupt);

   return( corrupt ? 1 : 0);
}
