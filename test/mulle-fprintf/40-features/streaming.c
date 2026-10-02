#include <mulle-fprintf/mulle-fprintf.h>

#include <stdio.h>
#include <string.h>

// Exercises the flushablebuffer streaming path with output larger than the
// 1024-byte default capacity (forces multiple batched fwrite flushes).
int   main( void)
{
   char    big[ 1101];
   char    check[ 1101];
   FILE    *fp;
   long    len;
   size_t  n;

   memset( big, 'a', 1099);
   big[ 1099] = '\n';
   big[ 1100] = '\0';

   fp = tmpfile();
   if( ! fp)
      return( 1);

   mulle_fprintf( fp, "%s", big);   // 1100 bytes > 1024 flushable capacity

   fflush( fp);
   len = ftell( fp);
   printf( "multi_chunk: %s\n", len > 1024 ? "yes" : "no");

   rewind( fp);
   n       = fread( check, 1, sizeof( check) - 1, fp);
   check[ n] = '\0';
   printf( "read_back: %zu\n", n);
   printf( "content_matches: %s\n", strcmp( big, check) == 0 ? "yes" : "no");

   fclose( fp);
   return( 0);
}
