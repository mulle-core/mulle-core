#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

// NAN's sign bit is implementation-defined (gcc positive, tcc negative) and
// this test prints the sign. Pin the bit pattern instead of inheriting it.
static double  nan_with_sign( int negative)
{
   uint64_t   bits = 0x7ff8000000000000ull;
   double     value;

   if( negative)
      bits |= 0x8000000000000000ull;
   memcpy( &value, &bits, sizeof( value));
   return( value);
}

void test_decompose(double value, const char* label) {
  struct mulle_dtostr_decimal dec = mulle_dtostr_decompose(value);
  
  printf("%-20s sign=%d special=%d sig=%llu exp=%d\n", 
         label, dec.sign, dec.special, 
         (unsigned long long)dec.significand, dec.exponent);
}

int main(void) {
  printf("Testing mulle_dtostr_decompose\n");
  printf("============================\n\n");
  
  test_decompose(0.0, "0.0");
  test_decompose(-0.0, "-0.0");
  test_decompose(1.0, "1.0");
  test_decompose(-1.0, "-1.0");
  test_decompose(INFINITY, "+inf");
  test_decompose(-INFINITY, "-inf");
  test_decompose(nan_with_sign( 0), "nan");
  test_decompose(3.14159265358979323846, "pi");
  test_decompose(6.62607015e-34, "Planck");
  test_decompose(1.23456789e20, "1.23e20");
  
  printf("\nStruct size: %zu bytes (should be 16)\n", sizeof(struct mulle_dtostr_decimal));
  
  return 0;
}
