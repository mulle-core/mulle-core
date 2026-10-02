#include <mulle-data/mulle-data.h>


int   main( int argc, char *argv[])
{
   return( mulle_data_get_version() == MULLE__DATA_VERSION ? 0 : -1);
}
