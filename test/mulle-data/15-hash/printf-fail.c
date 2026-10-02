#include <stdio.h>


int  main( void)
{
//  printf( "%#016llx\n", 0x00000000ced51853LL);
//  printf( "%#016llx\n", 0x10000000ced51853LL);
//  printf( "%#016llx\n", 0x20000000ced51853LL);

   printf( "%#08x\n", 0x00000000);
   printf( "%#08x\n", 0x00000001);
   printf( "%#08x\n", 0x10000002);
   printf( "%#08x\n", 0x20000003);

   printf( "\n");

   printf( "%#x\n", 0x00000000);
   printf( "%#x\n", 0x00000001);
   printf( "%#x\n", 0x10000002);
   printf( "%#x\n", 0x20000003);

   printf( "\n");

   printf( "%x\n", 0x00000000);
   printf( "%x\n", 0x00000001);
   printf( "%x\n", 0x10000002);
   printf( "%x\n", 0x20000003);
   return( 0);
}

