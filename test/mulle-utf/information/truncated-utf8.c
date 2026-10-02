#include <mulle-utf/mulle-utf.h>

#include <stdio.h>
#include <stdlib.h>

static int test_invalid(const char *description, char *text, size_t len)
{
   struct mulle_utf_information info;
   int result;

   result = mulle_utf8_information(text, len, &info);
   if (result == 0)
   {
      printf("FAIL: %s - expected error but got success\n", description);
      return 1;
   }
   printf("PASS: %s\n", description);
   return 0;
}

int main()
{
   int fails = 0;

   // 2-byte sequences - truncated (missing continuation bytes)
   fails += test_invalid("2-byte truncated (only start)", 
                         (char[]){0xC3}, 1);
   fails += test_invalid("2-byte truncated at end of buffer", 
                         (char[]){'A', 'B', 'C', 0xC3}, 4);
   fails += test_invalid("Another 2-byte truncated at end of buffer",
                         (char[]){'"', 'H', 0xc3 }, 3);

   // 3-byte sequences - truncated
   fails += test_invalid("3-byte truncated (only start)", 
                         (char[]){0xE2}, 1);
   fails += test_invalid("3-byte truncated (start + 1 continuation)", 
                         (char[]){0xE2, 0x80}, 2);
   fails += test_invalid("3-byte truncated at end (only start)", 
                         (char[]){'H', 'i', 0xE2}, 3);
   fails += test_invalid("3-byte truncated at end (start + 1)", 
                         (char[]){'H', 'e', 'l', 'l', 'o', 0xE2, 0x80}, 7);

   // 4-byte sequences - truncated
   fails += test_invalid("4-byte truncated (only start)", 
                         (char[]){0xF0}, 1);
   fails += test_invalid("4-byte truncated (start + 1 continuation)", 
                         (char[]){0xF0, 0x9F}, 2);
   fails += test_invalid("4-byte truncated (start + 2 continuations)", 
                         (char[]){0xF0, 0x9F, 0x98}, 3);
   fails += test_invalid("4-byte truncated at end (start + 2)", 
                         (char[]){'T', 'e', 's', 't', 0xF0, 0x9F, 0x98}, 7);

   // Invalid continuation bytes (wrong bit pattern - not 10xxxxxx)
   fails += test_invalid("2-byte with ASCII as continuation", 
                         (char[]){0xC3, 0x41}, 2);
   fails += test_invalid("2-byte with start byte as continuation", 
                         (char[]){0xC3, 0xC0}, 2);
   fails += test_invalid("2-byte with 0xFF as continuation", 
                         (char[]){0xC3, 0xFF}, 2);

   fails += test_invalid("3-byte with ASCII as 1st continuation", 
                         (char[]){0xE2, 0x41, 0x93}, 3);
   fails += test_invalid("3-byte with ASCII as 2nd continuation", 
                         (char[]){0xE2, 0x80, 0x41}, 3);
   fails += test_invalid("3-byte with 0x00 as 1st continuation", 
                         (char[]){0xE2, 0x00, 0x93}, 3);
   fails += test_invalid("3-byte with 0x7F as 2nd continuation", 
                         (char[]){0xE2, 0x80, 0x7F}, 3);
   fails += test_invalid("3-byte with start byte as 1st continuation", 
                         (char[]){0xE2, 0xC0, 0x93}, 3);
   fails += test_invalid("3-byte with both continuations invalid", 
                         (char[]){0xE2, 0x41, 0x42}, 3);

   fails += test_invalid("4-byte with ASCII as 1st continuation", 
                         (char[]){0xF0, 0x41, 0x98, 0x80}, 4);
   fails += test_invalid("4-byte with ASCII as 2nd continuation", 
                         (char[]){0xF0, 0x9F, 0x41, 0x80}, 4);
   fails += test_invalid("4-byte with ASCII as 3rd continuation", 
                         (char[]){0xF0, 0x9F, 0x98, 0x41}, 4);
   fails += test_invalid("4-byte with start bytes as continuations", 
                         (char[]){0xF0, 0xC0, 0xC1, 0x80}, 4);
   fails += test_invalid("4-byte with all continuations invalid", 
                         (char[]){0xF0, 0x41, 0x42, 0x43}, 4);
   fails += test_invalid("4-byte with start byte as 1st continuation", 
                         (char[]){0xF0, 0xE2, 0x98, 0x80}, 4);

   // Invalid continuation bytes in middle of string
   fails += test_invalid("2-byte invalid continuation in middle", 
                         (char[]){'A', 'B', 0xC3, 0x41, 'C'}, 5);
   fails += test_invalid("3-byte invalid 1st continuation in middle", 
                         (char[]){'X', 0xE2, 0x41, 0x93, 'Y'}, 5);
   fails += test_invalid("3-byte invalid 2nd continuation in middle", 
                         (char[]){'X', 0xE2, 0x80, 0x20, 'Y'}, 5);

   if (fails == 0)
      printf("\nAll tests passed!\n");
   else
      printf("\n%d test(s) failed!\n", fails);

   return fails;
}
