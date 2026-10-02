#include <mulle-dlfcn/mulle-dlfcn.h>


int   main( void)
{
   // already happy if it links on the platform
   if( dlsym( MULLE_RTLD_DEFAULT, "main"))
      return( 0);
   return( 1);
}

