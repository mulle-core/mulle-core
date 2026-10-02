#include <mulle-mmap/mulle-mmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


//  the checked-in fixture next to this test
#define LARGE_TXT_SIZE  16384


static void test_unaligned_offsets( void)
{
   struct mulle_mmap   info;
   size_t              page_size;
   size_t              test_offsets[] = {1, 3, 17, 63, 127, 255, 511, 1023, 2047, 4095};
   size_t              num_offsets = sizeof(test_offsets) / sizeof(test_offsets[0]);
   size_t              i;
   int                 rval;

   printf( "Testing unaligned offset mappings:\n");
   
   page_size = mulle_mmap_get_system_pagesize();
   //  The page size and the granularity are the platform's numbers, not this
   //  test's: they go to stderr, where a reader sees them without either of them
   //  becoming something a golden has to agree about.
   fprintf( stderr, "  System page size: %zu bytes\n", page_size);

   for( i = 0; i < num_offsets; i++)
   {
      size_t offset = test_offsets[i];
      
      // Skip offsets that are too large for our large file
      if( offset >= LARGE_TXT_SIZE)
         continue;
         
      printf( "  Testing offset %zu:\n", offset);
      
      _mulle_mmap_init( &info, mulle_mmap_read);
      
      rval = _mulle_mmap_map_file_range( &info, "large.txt", offset, 1024);
      if( rval)
      {
         printf( "    Failed to map at offset %zu: %s\n", offset, strerror( errno));
         _mulle_mmap_done( &info);
         continue;
      }
      
      size_t actual_length = _mulle_mmap_get_length( &info);
      size_t mapped_length = _mulle_mmap_get_mapped_length( &info);
      size_t mapping_offset = _mulle_mmap_get_mapping_offset( &info);
      void *data = _mulle_mmap_get_bytes( &info);
      
      printf( "    SUCCESS - Length: %zu, Mapped: %zu, MapOffset: %zu\n",
              actual_length, mapped_length, mapping_offset);
      
      // Verify we can read the data
      if( actual_length > 0 && data != NULL)
      {
         unsigned char first_byte = *((unsigned char *) data);
         printf( "    First byte at offset %zu: 0x%02x\n", offset, first_byte);
      }
      
      _mulle_mmap_done( &info);
   }
   
   printf( "  Unaligned offset tests completed\n\n");
}


static void test_length_variations( void)
{
   struct mulle_mmap   info;
   size_t              test_lengths[] = {1, 2, 3, 4, 5, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191};
   size_t              num_lengths = sizeof(test_lengths) / sizeof(test_lengths[0]);
   size_t              i;
   int                 rval;

   printf( "Testing various length parameters:\n");

   for( i = 0; i < num_lengths; i++)
   {
      size_t length = test_lengths[i];
      
      printf( "  Testing length %zu:\n", length);
      
      _mulle_mmap_init( &info, mulle_mmap_read);
      
      rval = _mulle_mmap_map_file_range( &info, "large.txt", 0, length);
      if( rval)
      {
         printf( "    Failed to map length %zu: %s\n", length, strerror( errno));
         _mulle_mmap_done( &info);
         continue;
      }
      
      size_t actual_length = _mulle_mmap_get_length( &info);
      size_t mapped_length = _mulle_mmap_get_mapped_length( &info);
      
      printf( "    SUCCESS - Requested: %zu, Actual: %zu, Mapped: %zu\n", 
              length, actual_length, mapped_length);
      
      _mulle_mmap_done( &info);
   }
   
   // Test SIZE_MAX
   printf( "  Testing SIZE_MAX length:\n");
   _mulle_mmap_init( &info, mulle_mmap_read);
   
   rval = _mulle_mmap_map_file_range( &info, "large.txt", 0, SIZE_MAX);
   if( rval)
   {
      printf( "    Failed to map with SIZE_MAX: %s\n", strerror( errno));
   }
   else
   {
      size_t actual_length = _mulle_mmap_get_length( &info);
      printf( "    SUCCESS - SIZE_MAX mapped to actual file size: %zu\n", actual_length);
   }
   
   _mulle_mmap_done( &info);
   printf( "  Length variation tests completed\n\n");
}


static void test_boundary_conditions( void)
{
   struct mulle_mmap   info;
   int                 rval;
   size_t              file_size = 16384; // large.txt size

   printf( "Testing boundary conditions:\n");

   // Test 1: Offset at end of file
   printf( "  Test 1: Mapping at end of file (offset = file size)\n");
   _mulle_mmap_init( &info, mulle_mmap_read);
   rval = _mulle_mmap_map_file_range( &info, "large.txt", file_size, 100);
   if( rval)
   {
      printf( "    SUCCESS: Failed as expected for offset at end of file (error code: %d)\n", rval);
   }
   else
   {
      printf( "    INFO: Mapping at end of file succeeded, length: %zu\n", _mulle_mmap_get_length( &info));
   }
   _mulle_mmap_done( &info);

   // Test 2: Offset beyond file
   printf( "  Test 2: Mapping beyond end of file (offset > file size)\n");
   _mulle_mmap_init( &info, mulle_mmap_read);
   rval = _mulle_mmap_map_file_range( &info, "large.txt", file_size + 1000, 100);
   if( rval)
   {
      printf( "    SUCCESS: Failed as expected for offset beyond file (error code: %d)\n", rval);
   }
   else
   {
      printf( "    ERROR: Should have failed for offset beyond file\n");
   }
   _mulle_mmap_done( &info);

   // Test 3: Length extending beyond file
   printf( "  Test 3: Length extending beyond file end\n");
   _mulle_mmap_init( &info, mulle_mmap_read);
   rval = _mulle_mmap_map_file_range( &info, "large.txt", file_size - 1000, 2000);
   if( rval)
   {
      printf( "    Failed with error code: %d\n", rval);
   }
   else
   {
      size_t actual_length = _mulle_mmap_get_length( &info);
      printf( "    SUCCESS: Length truncated to file boundary, actual length: %zu\n", actual_length);
   }
   _mulle_mmap_done( &info);

   // Test 4: Single byte at very end of file
   printf( "  Test 4: Single byte at end of file\n");
   _mulle_mmap_init( &info, mulle_mmap_read);
   rval = _mulle_mmap_map_file_range( &info, "large.txt", file_size - 1, 1);
   if( rval)
   {
      printf( "    Failed with error code: %d\n", rval);
   }
   else
   {
      size_t actual_length = _mulle_mmap_get_length( &info);
      void *data = _mulle_mmap_get_bytes( &info);
      printf( "    SUCCESS: Mapped last byte, length: %zu\n", actual_length);
      if( data != NULL)
      {
         unsigned char last_byte = *((unsigned char *) data);
         printf( "    Last byte value: 0x%02x\n", last_byte);
      }
   }
   _mulle_mmap_done( &info);

   printf( "  Boundary condition tests completed\n\n");
}


static void test_page_boundary_alignments( void)
{
   struct mulle_mmap   info;
   size_t              page_size;
   size_t              granularity;
   size_t              offsets_around_granularity[8];
   size_t              i;
   size_t              skipped;
   int                 rval;

   printf( "Testing page boundary alignments:\n");
   
   page_size   = mulle_mmap_get_system_pagesize();
   granularity = mulle_mmap_get_allocation_granularity();
   //  The page size and the granularity are the platform's numbers, not this
   //  test's: they go to stderr, where a reader sees them without either of them
   //  becoming something a golden has to agree about.
   fprintf( stderr, "  System page size: %zu bytes\n", page_size);
   fprintf( stderr, "  Mapping granularity: %zu bytes\n", granularity);

   // An *offset* keeps the mapping granularity, not the page size. On POSIX the
   // two are the same number, on Windows the offset one is the bigger: an offset
   // of a page is not one a view may sit at there.
   offsets_around_granularity[0] = granularity - 4;
   offsets_around_granularity[1] = granularity - 2;
   offsets_around_granularity[2] = granularity - 1;
   offsets_around_granularity[3] = granularity;
   offsets_around_granularity[4] = granularity + 1;
   offsets_around_granularity[5] = granularity + 2;
   offsets_around_granularity[6] = granularity + 4;
   offsets_around_granularity[7] = granularity * 2;

   skipped = 0;
   for( i = 0; i < 8; i++)
   {
      size_t offset = offsets_around_granularity[i];
      
      //  An offset needs the mapping length to still fit into the fixture.
      //  On Windows the granularity is 64 KiB, so every offset here misses that
      //  and the block below does not run: a reader has to be told, because a
      //  silent skip looks exactly like a pass. Crossing the granularity
      //  boundary is exercised by the unaligned offsets above on every platform,
      //  what this block adds is the crossing *into the next* granularity block.
      if( offset + page_size > LARGE_TXT_SIZE)
      {
         skipped++;
         continue;
      }
         
      printf( "  Testing granularity-relative offset %zu (granularity %s %zu):\n", 
              offset, 
              (offset < granularity) ? "-" : "+", 
              (offset < granularity) ? (granularity - offset) : (offset - granularity));
      
      _mulle_mmap_init( &info, mulle_mmap_read);
      
      rval = _mulle_mmap_map_file_range( &info, "large.txt", offset, page_size);
      if( rval)
      {
         printf( "    Failed to map at page-relative offset %zu: %s\n", offset, strerror( errno));
         _mulle_mmap_done( &info);
         continue;
      }
      
      size_t actual_length = _mulle_mmap_get_length( &info);
      size_t mapped_length = _mulle_mmap_get_mapped_length( &info);
      size_t mapping_offset = _mulle_mmap_get_mapping_offset( &info);
      void *data = _mulle_mmap_get_bytes( &info);
      
      printf( "    SUCCESS - Length: %zu, Mapped: %zu, MapOffset: %zu\n", 
              actual_length, mapped_length, mapping_offset);
      printf( "    Data aligned to %zu: %s\n",
              page_size,
              ((uintptr_t) data % page_size == 0) ? "YES" : "NO");
      
      _mulle_mmap_done( &info);
   }
   
   if( skipped)
   {
      fprintf( stderr, "  NOTE: %zu of 8 granularity-boundary offsets skipped: "
                       "granularity %zu with a %d byte fixture\n",
                       skipped, granularity, (int) LARGE_TXT_SIZE);
   }
   
   printf( "  Page boundary alignment tests completed\n\n");
}


int main( void)
{
   printf( "=== Edge Cases and Alignment Tests ===\n\n");

   // Test unaligned offsets
   test_unaligned_offsets();
   
   // Test various lengths
   test_length_variations();
   
   // Test boundary conditions
   test_boundary_conditions();
   
   // Test page boundary alignments
   test_page_boundary_alignments();

   printf( "=== All Edge Case and Alignment Tests Completed ===\n");
   return( 0);
}
