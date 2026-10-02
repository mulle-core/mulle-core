#include <mulle-mmap/mulle-mmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/*  mulle_mmap_map_shared_memory() takes a preferred address. That is how a
 *  second process re-attaches a region at the address the first process saw,
 *  which is what makes the pointers inside the region -- dlmalloc's, for
 *  instance -- mean the same thing in both processes.
 *
 *  The address is asked for and not demanded, but it is not negotiable either:
 *  a request that cannot be granted fails, rather than settling for a region
 *  that was built at some other address. NULL says the OS may choose. So this
 *  test asks for every kind of address a caller can land in and prints the
 *  class of answer it got:
 *
 *     1. none                        the OS chooses
 *     2. an aligned, free address    the request can be granted
 *     3. one byte off such an address
 *     4. page aligned inside a free slot, which is granularity aligned on
 *        POSIX but not on Windows
 *     5. an address that is already mapped
 *     6. a size larger than the region
 *     7. a size smaller than the region
 *
 *  Case 2 is the promise the call exists for. Cases 3 to 5 are requests that
 *  cannot be granted: they answer NULL, and whatever was mapped at the address
 *  is left intact rather than replaced. Case 4 is the one answer that is the
 *  platform's own business, because the allocation granularity and the page size
 *  are the same number everywhere but on Windows -- so the two goldens are
 *  supposed to differ exactly there.
 *
 *  Cases 6 and 7 ask with the right address and the wrong size, which is the
 *  other way to get a mapping you cannot use. Neither mapping is read: past the
 *  end of the region there is nothing to read, and on POSIX touching it raises
 *  SIGBUS, so what the call answers is all this test can record.
 *
 *  Nothing here puts an address, a page size or a granularity into stdout:
 *  those are the platform's numbers and go to stderr. What is compared is the
 *  class of answer, which is what a caller codes against.
 */


static char   *answer( void *wanted, void *got)
{
   if( ! got)
      return( "NULL");
   if( got == wanted)
      return( "the requested address");
   return( "an address of the system's choosing");
}


static void   *alloc_aligned( size_t size)
{
   void   *p;

   p = mulle_mmap_alloc_pages( size);
   if( ! p)
      fprintf( stderr, "mulle_mmap_alloc_pages( %zu) failed: errno %d\n",
                      size, errno);
   return( p);
}


//  An address that is free, and aligned the way the platform aligns what it
//  hands out. Allocating one and giving it back leaves exactly that: the caller
//  asks for an address it cannot know in advance any other way.
//
static void   *free_aligned_address( size_t size)
{
   void   *p;

   p = alloc_aligned( size);
   if( p)
      mulle_mmap_free_pages( p, size);
   return( p);
}


//  Ask for an address and report the class of answer.
//
static void   ask( char *title, struct mulle_mmap_shared_memory *mem,
                   void *wanted, size_t length)
{
   void   *got;

   printf( "\n%s\n", title);

   if( ! wanted)
   {
      printf( "ERROR: no free address to ask for\n");
      return;
   }

   got = mulle_mmap_map_shared_memory( mem->handle, length, wanted);
   printf( "  answer: %s\n", answer( wanted, got));
   mulle_mmap_free_pages( got, length);
}


int   main( void)
{
   struct mulle_mmap_shared_memory   mem;
   size_t                            pagesize;
   size_t                            granularity;
   size_t                            size;
   char                              *busy;
   void                              *wanted;
   void                              *got;

   pagesize    = mulle_mmap_get_system_pagesize();
   granularity = mulle_mmap_get_allocation_granularity();

   fprintf( stderr, "system page size: %zu bytes\n", pagesize);
   fprintf( stderr, "allocation granularity: %zu bytes\n", granularity);

   //  one granularity of shared memory: on POSIX that is a page, on Windows 64
   //  of them. mem.size is what a caller would map at again elsewhere.
   //
   size = granularity;
   mem  = mulle_mmap_alloc_shared_memory( size);
   if( ! mem.address)
   {
      printf( "ERROR: mulle_mmap_alloc_shared_memory failed: errno %d\n", errno);
      return( 1);
   }

   printf( "=== mulle_mmap_map_shared_memory: the preferred address ===\n");

   printf( "\n1. no preferred address\n");
   got = mulle_mmap_map_shared_memory( mem.handle, size, NULL);
   printf( "  answer: %s\n", answer( NULL, got));
   mulle_mmap_free_pages( got, size);

   printf( "\n2. an aligned address that is free\n");
   wanted = free_aligned_address( size);
   if( ! wanted)
   {
      printf( "ERROR: no free address to ask for\n");
      return( 1);
   }
   got = mulle_mmap_map_shared_memory( mem.handle, size, wanted);
   printf( "  answer: %s\n", answer( wanted, got));
   if( got)
   {
      //  the second view is the same memory seen twice, which is the point of
      //  asking for the address at all
      ((char *) got)[7] = 0x5A;
      printf( "  the first view sees the write: %s\n",
              ((char *) mem.address)[7] == 0x5A ? "yes" : "no");
   }
   mulle_mmap_free_pages( got, size);

   wanted = free_aligned_address( size);
   ask( "3. that address, one byte off", &mem, (char *) wanted + 1, size);

   wanted = free_aligned_address( 2 * size);
   ask( "4. a page aligned address inside a free slot", &mem,
        (char *) wanted + pagesize, size);

   printf( "\n5. an address that is already mapped\n");
   busy = alloc_aligned( size);
   if( ! busy)
   {
      printf( "ERROR: no address to occupy\n");
      return( 1);
   }
   busy[0] = 0x3C;
   got     = mulle_mmap_map_shared_memory( mem.handle, size, busy);
   printf( "  answer: %s\n", answer( busy, got));
   printf( "  the mapping that was there: %s\n",
           busy[0] == 0x3C ? "intact" : "discarded");

   //  a granted request would mean the address holds the new mapping now, so
   //  only what the mapping did not take is freed by itself
   //
   mulle_mmap_free_pages( got, size);
   if( got != busy)
      mulle_mmap_free_pages( busy, size);

   //  the right address and the wrong size: a mapping that reaches past the end
   //  of the region is not usable either, and the two platforms need not agree
   //  about saying so
   //
   wanted = free_aligned_address( 2 * size);
   ask( "6. a size larger than the region", &mem, wanted, size + pagesize);

   wanted = free_aligned_address( size);
   ask( "7. a size smaller than the region", &mem, wanted, size / 2);

   mulle_mmap_free_shared_memory( &mem);

   printf( "\n=== Test Complete ===\n");
   return( 0);
}
