/* Does everything mulle_dtostr prints parse back to the identical bit
 * pattern with mulle_strtod ? That is the whole point of having our own
 * strtod, so it gets its own test with a wide sample.
 *
 * The sample is deterministic, the random number generator is a fixed seed
 * xorshift, so the output is stable across runs and platforms.
 */

#include <mulle-dtostr/mulle-dtostr.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>


static uint64_t   bits_of( double d)
{
   uint64_t   bits;

   memcpy( &bits, &d, sizeof( bits));
   return( bits);
}


static double   double_of( uint64_t bits)
{
   double   d;

   memcpy( &d, &bits, sizeof( d));
   return( d);
}


static uint64_t   rng = 0xF00DBABE12345678ull;

static uint64_t   next_rand( void)
{
   rng ^= rng << 13;
   rng ^= rng >> 7;
   rng ^= rng << 17;
   return( rng);
}


static int   n_fail;
static int   n_shown;
static int   n_total;


static void   check( uint64_t bits)
{
   char       buffer[ MULLE__DTOSTR_BUFFER_SIZE];
   double     back;
   double     value;

   /* inf and nan are not decimals, they are tested separately */
   if( (bits & 0x7FF0000000000000ull) == 0x7FF0000000000000ull)
      return;

   value = double_of( bits);
   mulle_dtostr( value, buffer);
   back = mulle_strtod( buffer, (char **) 0);
   n_total++;

   if( bits_of( back) == bits)
      return;

   n_fail++;
   if( n_shown < 12)
   {
      printf( "   0x%016llx -> \"%s\" -> 0x%016llx\n",
              (unsigned long long) bits,
              buffer,
              (unsigned long long) bits_of( back));
      n_shown++;
   }
}


static void   report( char *label, int *p_before_fail, int *p_before_total)
{
   printf( "%-28s %7d tested %7d failed\n",
           label,
           n_total - *p_before_total,
           n_fail - *p_before_fail);
   *p_before_fail  = n_fail;
   *p_before_total = n_total;
}


int   main( void)
{
   uint64_t   base;
   uint64_t   bits;
   double     d;
   int        i;
   int        mark_fail;
   int        mark_total;

   mark_fail  = 0;
   mark_total = 0;

   printf( "mulle_dtostr -> mulle_strtod bit exactness\n");
   printf( "==========================================\n\n");

   /* zero, the subnormals and the first normals */
   for( bits = 0; bits <= 70000; bits++)
   {
      check( bits);
      check( bits | 0x8000000000000000ull);
   }
   report( "zero and subnormals", &mark_fail, &mark_total);

   /* every binade boundary, where the exponent handling changes */
   for( i = 1; i < 2047; i++)
   {
      base = (uint64_t) i << 52;
      for( bits = base - 3; bits <= base + 3; bits++)
         check( bits);
   }
   report( "binade boundaries", &mark_fail, &mark_total);

   /* the powers of ten, the classic trouble spots */
   d = 1.0;
   for( i = 0; i < 309; i++)
   {
      check( bits_of( d));
      check( bits_of( -d));
      d *= 10.0;
   }
   d = 1.0;
   for( i = 0; i < 324; i++)
   {
      check( bits_of( d));
      check( bits_of( -d));
      d /= 10.0;
   }
   report( "powers of ten", &mark_fail, &mark_total);

   /* small integers and simple fractions */
   for( i = 0; i < 2000; i++)
   {
      check( bits_of( (double) i));
      check( bits_of( (double) i / 10.0));
      check( bits_of( (double) i / 3.0));
   }
   report( "integers and fractions", &mark_fail, &mark_total);

   /* a broad random sample over the whole range */
   for( i = 0; i < 200000; i++)
   {
      bits = next_rand() & 0x7FFFFFFFFFFFFFFFull;
      check( bits);
      check( bits | 0x8000000000000000ull);
   }
   report( "random bit patterns", &mark_fail, &mark_total);

   printf( "\n%-28s %7d tested %7d failed\n", "TOTAL", n_total, n_fail);

   if( n_fail)
   {
      printf( "\nROUNDTRIP IS NOT EXACT\n");
      return( 1);
   }

   printf( "\nevery printed double parsed back to the same bits\n");
   return( 0);
}
