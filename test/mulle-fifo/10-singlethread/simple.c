#include <mulle-fifo/mulle-fifo.h>



int  main( int argc, char *argv[])
{
   struct mulle_pointerfifo   fifo;
   void                        *pointer;
   unsigned int                i;
   int                         rval;

   _mulle_pointerfifo_init( &fifo, 100, NULL);

   printf( "WRITE:\n");
   for( i = 0; i < 101; i++)
   {
      rval = _mulle_pointerfifo_write( &fifo, (void *) (intptr_t) i + 1);
      printf( "%u: %d\n", _mulle_pointerfifo_get_count( &fifo), rval);
   }

   printf( "READ:\n");
   for( i = 0; i < 101; i++)
   {
      pointer = _mulle_pointerfifo_read( &fifo);
      printf( "%ld\n", (long) (intptr_t) pointer);
   }

   _mulle_pointerfifo_done( &fifo);

   return( 0);
}