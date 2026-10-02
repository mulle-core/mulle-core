#include <mulle-multififo/mulle-multififo.h>



#define N_ITEMS  100


int  main( int argc, char *argv[])
{
   struct mulle_pointermultififo   fifo;
   void                        *pointer;
   unsigned int                i;
   int                         rval;

   _mulle_pointermultififo_init( &fifo, N_ITEMS, NULL);

   printf( "WRITE:\n");
   for( i = 0; i < N_ITEMS + 1; i++)
   {
      rval = _mulle_pointermultififo_write( &fifo, (void *) (intptr_t) i + 1);
      printf( "%u: %d\n", i + 1, rval);
   }

   printf( "READ:\n");
   for( i = 0; i < N_ITEMS + 1; i++)
   {
      pointer = _mulle_pointermultififo_read_barrier( &fifo);
      printf( "%ld\n", (long) (intptr_t) pointer);
   }

   _mulle_pointermultififo_done( &fifo);

   return( 0);
}