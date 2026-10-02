#include <mulle-slug/mulle-slug.h>

#include <stdio.h>

int   main( int argc, char *argv[])
{
   return( mulle_slug_get_version() == MULLE__SLUG_VERSION ? 0 : -1);
}
