#include <mulle-dtostr/mulle-dtostr.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

void test_roundtrip(double value, const char* name) {
    char buffer[MULLE__DTOSTR_BUFFER_SIZE];
    mulle_dtostr(value, buffer);
    
    double parsed = strtod(buffer, NULL);
    
    /* Get bit representations */
    uint64_t value_bits, parsed_bits;
    memcpy(&value_bits, &value, sizeof(value));
    memcpy(&parsed_bits, &parsed, sizeof(parsed));
    
    printf("%-20s: %s", name, buffer);
    
    /* Handle special cases */
    if (isnan(value)) {
        assert(isnan(parsed));
        printf(" ✓ (both NaN)\n");
    } else if (isinf(value)) {
        assert(isinf(parsed) && signbit(value) == signbit(parsed));
        printf(" ✓ (both inf, same sign)\n");
    } else if (value == 0.0) {
        assert(parsed == 0.0 && signbit(value) == signbit(parsed));
        printf(" ✓ (both zero, same sign)\n");
    } else {
        assert(value == parsed);
        if (value_bits == parsed_bits) {
            printf(" ✓ (exact bit match)\n");
        } else {
            printf(" ✓ (value match but different bits: 0x%016llx vs 0x%016llx)\n", 
                   (unsigned long long)value_bits, (unsigned long long)parsed_bits);
        }
    }
}

int main() {
    printf("Round-trip test: mulle_dtostr -> strtod\n");
    printf("=====================================\n\n");
    
    /* Test various values */
    test_roundtrip(0.0, "0.0");
    test_roundtrip(-0.0, "-0.0");
    test_roundtrip(1.0, "1.0");
    test_roundtrip(-1.0, "-1.0");
    test_roundtrip(2.0, "2.0");
    test_roundtrip(10.0, "10.0");
    test_roundtrip(12.2, "12.2");
    test_roundtrip(0.1, "0.1");
    test_roundtrip(0.01, "0.01");
    test_roundtrip(8.199999999999998, "8.199999999999998");
    test_roundtrip(8.199999999999999, "8.199999999999999");
//    test_roundtrip(8.1999999999999998, "8.1999999999999998");
//    test_roundtrip(8.1999999999999999, "8.1999999999999999");
    test_roundtrip(8.2, "8.2");
//    test_roundtrip(8.2000000000000001, "8.2000000000000001");
    test_roundtrip(8.200000000000001, "8.200000000000001");
    test_roundtrip(3.141592653589793, "pi");
    test_roundtrip(2.718281828459045, "e");
    test_roundtrip(6.62607015e-34, "Planck constant");
    test_roundtrip(1.23456789e20, "1.23456789e20");
    test_roundtrip(9.87654321e-15, "9.87654321e-15");
    test_roundtrip(1.7976931348623157e+308, "DBL_MAX");
    test_roundtrip(2.2250738585072014e-308, "DBL_MIN");
    test_roundtrip(-1.7976931348623157e+308, "-DBL_MAX");
    test_roundtrip(1.0/0.0, "+INFINITY");
    test_roundtrip(-1.0/0.0, "-INFINITY");
    test_roundtrip(NAN, "NAN");
    test_roundtrip(-NAN, "-NAN");
    
    /* Additional specific test values */
    test_roundtrip(8.589973428413488e+09, "8.589973428413488e+09");
    test_roundtrip(1.2999999999999998, "1.2999999999999998");
    test_roundtrip(9007199254740994.0, "9007199254740994.0");
    
    printf("\nAll round-trip tests passed! ✓\n");
    return 0;
}
