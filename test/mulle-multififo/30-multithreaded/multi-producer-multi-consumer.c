#include <mulle-multififo/mulle-multififo.h>

#include <time.h>
#include <stdlib.h>


#if 1
# define N_PRODUCERS     3
# define N_CONSUMERS     3
# define N_ITEMS         1024
# define N_QUEUE_ITEMS   32
#else
# define N_PRODUCERS     2
# define N_CONSUMERS     2
# define N_ITEMS         1024
# define N_QUEUE_ITEMS   2
#endif


static mulle_thread_rval_t   producer( void *arg)
{
   struct mulle_pointermultififo  *fifo = arg;
   struct timespec                 delay;
   int                             i;
   int                             warned;

   fprintf( stderr, "producer 0x%tx starts\n", (intptr_t) mulle_thread_id());

   warned = 0;
   i      = 1;
   for(;;)
   {
      switch( rand() % 8)
      {
      case 0 :
      case 1 :
      case 2 :
      case 3 :
         if( _mulle_pointermultififo_write( fifo, (void *) (intptr_t) i) == -1)
         {
            if( ! warned)
            {
               fprintf( stderr, "producer 0x%tx full\n", (intptr_t) mulle_thread_id());
               warned = 1;
            }
         }
         else
         {
            if( warned)
            {
               fprintf( stderr, "producer 0x%tx continues\n", (intptr_t) mulle_thread_id());
               warned = 0;
            }

            if( i == N_ITEMS)
               goto done;
            ++i;
         }
         break;

      case 4 :
      case 5 :
      case 6 :
      case 7 :
         delay.tv_sec  = 0;
         delay.tv_nsec = rand() % 10000;
         nanosleep( &delay, NULL);
      }
   }

done:
   fprintf( stderr, "producer 0x%tx quits\n", (intptr_t) mulle_thread_id());
   mulle_thread_return();
}


static mulle_thread_rval_t   consumer( void *arg)
{
   struct mulle_pointermultififo   *fifo = arg;
   struct timespec                 delay;
   void                            *pointer;
   int                             warned;

   fprintf( stderr, "consumer 0x%tx starts\n", (intptr_t) mulle_thread_id());

   warned = 0;
   for(;;)
   {
      switch( rand() % 8)
      {
      case 0 :
      case 1 :
      case 2 :
      case 3 :
         pointer = _mulle_pointermultififo_read_barrier( fifo);
         if( ! pointer)
         {
            if( ! warned)
            {
               fprintf( stderr, "consumer 0x%tx empty\n", (intptr_t) mulle_thread_id());
               warned = 1;
            }
            continue;
         }

         if( warned)
         {
            fprintf( stderr, "consumer 0x%tx continues\n", (intptr_t) mulle_thread_id());
            warned = 0;
         }

         printf( "%ld\n", (long) (intptr_t) pointer); fflush( stdout);
         if( pointer == (void *) N_ITEMS)
         {
            goto done;
         }
         break;

      case 4 :
      case 5 :
      case 6 :
      case 7 :
         delay.tv_sec  = 0;
         delay.tv_nsec = rand() % (10000 / 5);
         nanosleep( &delay, NULL);
      }
   }

done:
   fprintf( stderr, "consumer 0x%tx quits\n", (intptr_t) mulle_thread_id());
   mulle_thread_return();
}



int  main( int argc, char *argv[])
{
   struct mulle_pointermultififo  fifo;
   mulle_thread_t                 producer_thread[ N_PRODUCERS];
   mulle_thread_t                 consumer_thread[ N_CONSUMERS];
   int                            i;

   _mulle_pointermultififo_init( &fifo, N_QUEUE_ITEMS, NULL);

   for( i = 0; i < N_PRODUCERS; i++)
      mulle_thread_create( producer, &fifo, &producer_thread[ i]);

   for( i = 0; i < N_CONSUMERS; i++)
      mulle_thread_create( consumer, &fifo, &consumer_thread[ i]);

   for( i = 0; i < N_PRODUCERS; i++)
      mulle_thread_join( producer_thread[ i]);

   for( i = 0; i < N_CONSUMERS; i++)
      mulle_thread_join( consumer_thread[ i]);

   _mulle_pointermultififo_done( &fifo);

   return( 0);
}