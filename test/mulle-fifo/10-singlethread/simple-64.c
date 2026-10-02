#include <mulle-fifo/mulle-fifo.h>



int  main( int argc, char *argv[])
{
   struct mulle__pointerfifo64  fifo;
   void                         *pointer;
   unsigned int                 i;
   int                          rval;

   _mulle__pointerfifo64_init( &fifo);

   printf( "WRITE:\n");
   for( i = 0; i < 65; i++)
   {
      rval = _mulle__pointerfifo64_write( &fifo, (void *) (intptr_t) i + 1);
      printf( "%u: %d\n", _mulle__pointerfifo64_get_count( &fifo), rval);
   }

   printf( "READ:\n");
   for( i = 0; i < 65; i++)
   {
      pointer = _mulle__pointerfifo64_read( &fifo);
      printf( "%ld\n", (long) (intptr_t) pointer);
   }

   _mulle__pointerfifo64_done( &fifo);

   return( 0);
}