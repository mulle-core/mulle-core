#include <mulle-fifo/mulle-fifo.h>


//
// checks the documented contract of the dynamic fifo:
// - NULL fifo is tolerated by the safe wrappers
// - NULL pointers can not be stored (-2)
// - a full fifo fails with -1 and keeps its contents
// - reads come out in write order, across storage wraparound
// - an empty fifo reads NULL
// - done is idempotent
//
int  main( int argc, char *argv[])
{
   struct mulle_pointerfifo   fifo;
   void                       *pointer;
   int                        failures;
   unsigned int               i;
   int                        rval;
   failures = 0;
   /* safe wrappers tolerate a NULL fifo */
   mulle_pointerfifo_init( NULL, 4, NULL);
   mulle_pointerfifo_done( NULL);
   if( mulle_pointerfifo_write( NULL, (void *) (intptr_t) 1) != -1)
      ++failures;
   if( mulle_pointerfifo_read( NULL) != NULL)
      ++failures;
   if( mulle_pointerfifo_get_count( NULL) != 0)
      ++failures;

   _mulle_pointerfifo_init( &fifo, 4, NULL);

   /* NULL pointers are rejected, the count does not change */
   rval = _mulle_pointerfifo_write( &fifo, NULL);
   printf( "write NULL: %d (count %u)\n", rval, _mulle_pointerfifo_get_count( &fifo));
   if( rval != -2)
      ++failures;
   if( _mulle_pointerfifo_get_count( &fifo) != 0)
      ++failures;

   /* fill to capacity */
   for( i = 1; i <= 4; i++)
   {
      rval = _mulle_pointerfifo_write( &fifo, (void *) (intptr_t) i);
      if( rval != 0)
         ++failures;
   }
   printf( "full count: %u\n", _mulle_pointerfifo_get_count( &fifo));
   if( _mulle_pointerfifo_get_count( &fifo) != 4)
      ++failures;

   /* one past capacity fails with -1 and keeps the count */
   rval = _mulle_pointerfifo_write( &fifo, (void *) (intptr_t) 5);
   printf( "write full: %d (count %u)\n", rval, _mulle_pointerfifo_get_count( &fifo));
   if( rval != -1)
      ++failures;
   if( _mulle_pointerfifo_get_count( &fifo) != 4)
      ++failures;

   /* read one, then write past the end of the storage to force wraparound */
   pointer = _mulle_pointerfifo_read( &fifo);
   if( (intptr_t) pointer != 1)
      ++failures;
   rval = _mulle_pointerfifo_write( &fifo, (void *) (intptr_t) 5);
   if( rval != 0)
      ++failures;

   /* drain: must come out in write order 2, 3, 4, 5 */
   for( i = 2; i <= 5; i++)
   {
      pointer = _mulle_pointerfifo_read( &fifo);
      if( (intptr_t) pointer != (intptr_t) i)
         ++failures;
   }

   /* empty now */
   pointer = _mulle_pointerfifo_read( &fifo);
   printf( "read empty: %ld (count %u)\n",
           (long) (intptr_t) pointer,
           _mulle_pointerfifo_get_count( &fifo));
   if( pointer != NULL)
      ++failures;
   if( _mulle_pointerfifo_get_count( &fifo) != 0)
      ++failures;

   /* interleaved write/read, indices wrap many times */
   for( i = 1; i <= 100; i++)
   {
      rval = _mulle_pointerfifo_write( &fifo, (void *) (intptr_t) i);
      if( rval != 0)
         ++failures;
      pointer = _mulle_pointerfifo_read( &fifo);
      if( (intptr_t) pointer != (intptr_t) i)
         ++failures;
   }
   printf( "interleave count: %u\n", _mulle_pointerfifo_get_count( &fifo));
   if( _mulle_pointerfifo_get_count( &fifo) != 0)
      ++failures;

   /* done is idempotent */
   _mulle_pointerfifo_done( &fifo);
   _mulle_pointerfifo_done( &fifo);

   printf( "%s\n", failures ? "FAIL" : "PASS");

   return( failures ? 1 : 0);
}
