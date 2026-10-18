#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

// The sign bit of NAN is implementation-defined: gcc's NAN is positive
// (0x7ff8...), tcc's is negative (0xfff8...). This test prints the sign, so
// build the NaN from an explicit bit pattern instead of inheriting one from
// <math.h> -- otherwise the expected output depends on the compiler.
static double  nan_with_sign( int negative)
{
   uint64_t   bits = 0x7ff8000000000000ull;
   double     value;

   if( negative)
      bits |= 0x8000000000000000ull;
   memcpy( &value, &bits, sizeof( value));
   return( value);
}

void test_value(double value, const char* label) {
  char buf[MULLE__DTOSTR_BUFFER_SIZE];
  char sys_buf[100];
  
  mulle_dtostr(value, buf);
  snprintf(sys_buf, sizeof(sys_buf), "%.17g", value);
  
  printf("%-20s mulle: %-20s system: %s\n", label, buf, sys_buf);
}

int main(void) {
  printf("Comparison: mulle_dtostr vs system printf\n");
  printf("===========================================\n\n");
  
  test_value(INFINITY, "+INFINITY");
  test_value(-INFINITY, "-INFINITY");
  test_value(nan_with_sign( 0), "NAN");
  test_value(nan_with_sign( 1), "-NAN");
  test_value(0.0, "+0.0");
  test_value(-0.0, "-0.0");
  test_value(1.0, "1.0");
  test_value(-1.0, "-1.0");
  test_value(2.0, "2.0");
  test_value(10.0, "10.0");
  test_value(0.1, "0.1");
  test_value(0.5, "0.5");
  test_value(3.14159265358979323846, "pi");
  test_value(2.71828182845904523536, "e");
  test_value(6.62607015e-34, "Planck constant");
  test_value(1.23456789e20, "1.23456789e20");
  test_value(9.87654321e-15, "9.87654321e-15");
  test_value(1e308, "1e308 (near max)");
  test_value(1e-308, "1e-308 (near min)");
  test_value(0.9999999999999999, "0.9999999999999999");
  test_value(1.0000000000000002, "1.0000000000000002");
  
  return 0;
}
