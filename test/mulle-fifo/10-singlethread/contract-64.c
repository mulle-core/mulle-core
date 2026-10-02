#include <mulle-fifo/mulle-fifo.h>


//
// checks the documented contract of the fixed size fifo:
// - NULL pointers can not be stored (-2)
// - a full fifo fails with -1 and keeps its contents
// - reads come out in write order, across storage wraparound
// - an empty fifo reads NULL
//
int  main( int argc, char *argv[])
{
   struct mulle__pointerfifo64   fifo;
   void                          *pointer;
   int                           failures;
   unsigned int                  i;
   int                           rval;

   failures = 0;

   _mulle__pointerfifo64_init( &fifo);

   /* NULL pointers are rejected, the count does not change */
   rval = _mulle__pointerfifo64_write( &fifo, NULL);
   printf( "write NULL: %d (count %u)\n", rval, _mulle__pointerfifo64_get_count( &fifo));
   if( rval != -2)
      ++failures;
   if( _mulle__pointerfifo64_get_count( &fifo) != 0)
      ++failures;

   /* fill to capacity */
   for( i = 1; i <= 64; i++)
   {
      rval = _mulle__pointerfifo64_write( &fifo, (void *) (intptr_t) i);
      if( rval != 0)
         ++failures;
   }
   printf( "full count: %u\n", _mulle__pointerfifo64_get_count( &fifo));
   if( _mulle__pointerfifo64_get_count( &fifo) != 64)
      ++failures;

   /* one past capacity fails with -1 and keeps the count */
   rval = _mulle__pointerfifo64_write( &fifo, (void *) (intptr_t) 65);
   printf( "write full: %d (count %u)\n", rval, _mulle__pointerfifo64_get_count( &fifo));
   if( rval != -1)
      ++failures;
   if( _mulle__pointerfifo64_get_count( &fifo) != 64)
      ++failures;

   /* read one, then write past the end of the storage to force wraparound */
   pointer = _mulle__pointerfifo64_read( &fifo);
   if( (intptr_t) pointer != 1)
      ++failures;
   rval = _mulle__pointerfifo64_write( &fifo, (void *) (intptr_t) 65);
   if( rval != 0)
      ++failures;

   /* drain: must come out in write order 2 .. 65 */
   for( i = 2; i <= 65; i++)
   {
      pointer = _mulle__pointerfifo64_read( &fifo);
      if( (intptr_t) pointer != (intptr_t) i)
         ++failures;
   }

   /* empty now */
   pointer = _mulle__pointerfifo64_read( &fifo);
   printf( "read empty: %ld (count %u)\n",
           (long) (intptr_t) pointer,
           _mulle__pointerfifo64_get_count( &fifo));
   if( pointer != NULL)
      ++failures;

   /* read observes the same values in the same order (interleaved) */
   for( i = 1; i <= 100; i++)
   {
      rval = _mulle__pointerfifo64_write( &fifo, (void *) (intptr_t) i);
      if( rval != 0)
         ++failures;
      pointer = _mulle__pointerfifo64_read( &fifo);
      if( (intptr_t) pointer != (intptr_t) i)
         ++failures;
   }
   printf( "interleave count: %u\n", _mulle__pointerfifo64_get_count( &fifo));
   if( _mulle__pointerfifo64_get_count( &fifo) != 0)
      ++failures;

   _mulle__pointerfifo64_done( &fifo);

   printf( "%s\n", failures ? "FAIL" : "PASS");

   return( failures ? 1 : 0);
}
