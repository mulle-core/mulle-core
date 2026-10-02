#include <stdio.h>


int  main( void)
{
#ifdef __APPLE__
   FILE  *fp;

   fp = fmemopen( NULL, 100, "w");
   if( ! fp)
   {
      perror( "fmemopen:");
      return( 1);
   }
   fclose( fp);
#endif
   return( 0);
}


