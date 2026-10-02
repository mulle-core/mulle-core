#include <mulle-mmap/mulle-mmap.h>

#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>


static int   count_open_descriptors( void)
{
   struct rlimit   rl;
   int             fd;
   int             max;
   int             n;

   max = 256;
   if( getrlimit( RLIMIT_NOFILE, &rl) == 0)
   {
      if( rl.rlim_cur == RLIM_INFINITY)
         max = 65536;
      else
         max = (int) rl.rlim_cur;
   }
   if( max > 65536)
      max = 65536;

   n = 0;
   for( fd = 0; fd < max; fd++)
   {
      if( fcntl( fd, F_GETFD) != -1)
         n++;
   }
   return( n);
}


static int   test_directory_mapping_leak( void)
{
   struct mulle_mmap   info;
   int                 after;
   int                 attempts;
   int                 before;
   int                 failures;
   int                 i;
   int                 rval;

   attempts = 64;
   failures = 0;
   before   = count_open_descriptors();

   for( i = 0; i < attempts; i++)
   {
      _mulle_mmap_init( &info, mulle_mmap_read);
      rval = _mulle_mmap_map_file_range( &info, ".", 0, (size_t) -1);
      if( rval)
         failures++;
      _mulle_mmap_done( &info);
   }

   after = count_open_descriptors();

   printf( "Test 1: failed directory mapping does not leak descriptors\n");
   printf( "  mapping attempts  : %d\n", attempts);
   printf( "  failed mappings   : %d\n", failures);
   printf( "  descriptors leaked: %d\n", after - before);

   if( failures != attempts)
   {
      printf( "  FAIL: expected every directory mapping to fail\n");
      return( 1);
   }
   if( after != before)
   {
      printf( "  FAIL: descriptor leak detected\n");
      return( 1);
   }
   printf( "  PASS\n");
   return( 0);
}


static int   test_public_api_round_trip( void)
{
   mulle_mmap_file_t   handle;
   int64_t             size;
   int                 after;
   int                 before;
   int                 failed;

   failed = 0;
   before = count_open_descriptors();

   printf( "Test 2: file_open/query_size/file_close round trip does not leak descriptors\n");

   handle = mulle_mmap_file_open( "empty.txt", mulle_mmap_read);
   if( handle == MULLE_MMAP_INVALID_HANDLE)
   {
      printf( "  FAIL: could not open empty.txt\n");
      failed = 1;
   }
   else
   {
      size = mulle_mmap_file_query_size( handle);
      printf( "  empty.txt size    : %lld\n", (long long) size);
      if( size != 0)
      {
         printf( "  FAIL: empty.txt should be empty\n");
         failed = 1;
      }
      mulle_mmap_file_close( handle);
   }

   handle = mulle_mmap_file_open( "small.txt", mulle_mmap_read);
   if( handle == MULLE_MMAP_INVALID_HANDLE)
   {
      printf( "  FAIL: could not open small.txt\n");
      failed = 1;
   }
   else
   {
      size = mulle_mmap_file_query_size( handle);
      printf( "  small.txt size    : %lld\n", (long long) size);
      if( size != 84)
      {
         printf( "  FAIL: small.txt should be 84 bytes\n");
         failed = 1;
      }
      mulle_mmap_file_close( handle);
   }

   after = count_open_descriptors();
   printf( "  descriptors leaked: %d\n", after - before);

   if( after != before)
   {
      printf( "  FAIL: descriptor leak detected\n");
      failed = 1;
   }
   if( ! failed)
      printf( "  PASS\n");
   return( failed);
}


int main( void)
{
   int   failed;

   printf( "=== Descriptor Leak Regression Tests ===\n\n");

   failed  = test_directory_mapping_leak();
   printf( "\n");
   failed |= test_public_api_round_trip();

   printf( "\n=== All Descriptor Leak Tests Completed ===\n");
   return( failed ? 1 : 0);
}
