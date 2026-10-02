#include <mulle-rbtree/mulle-rbtree.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>


#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>

ssize_t getline(char **lineptr, size_t *n, FILE *stream) {
    if (lineptr == NULL || n == NULL || stream == NULL)
        return -1;

    if (*lineptr == NULL || *n == 0) {
        *n = 128; // Initial buffer size
        *lineptr = malloc(*n);
        if (*lineptr == NULL)
            return -1;
    }

    size_t pos = 0;
    int c;

    while ((c = fgetc(stream)) != EOF) {
        // Resize buffer if full
        if (pos + 1 >= *n) {
            size_t new_size = *n * 2;
            char *temp = realloc(*lineptr, new_size);
            if (!temp)
                return -1;
            *lineptr = temp;
            *n = new_size;
        }

        (*lineptr)[pos++] = (char)c;
        if (c == '\n')
            break;
    }

    if (pos == 0 && c == EOF)
        return -1;

    (*lineptr)[pos] = '\0';
    return (ssize_t)pos;
}
#endif



static int  callback( void *payload, void *userinfo)
{
   printf( "%s\n", (char *) payload);
   return( 1);
}


//
// https://stackoverflow.com/questions/12252103/read-line-from-stdin-blocking
//
int main( int argc, const char * argv[])
{
   char                   *line = NULL;
   size_t                 size = 0;
   size_t                 len;
   struct mulle_rbtree    tree;

   mulle_rbtree_init( &tree,
                      (int (*)( void *, void *)) strcmp,
                      &mulle_container_valuecallback_copied_cstring,
                      NULL);

   for(;;)
   {
      if( getline( &line, &size, stdin) == -1)
      {
         free( line);
         break;
      }

      len = strlen( line);
      assert( len);

      if( line[ len - 1] == '\n')
         line[ len - 1] = 0;

      mulle_rbtree_add( &tree, line);
      fprintf( stderr , "inserted: %s\n", line);
   }

   mulle_rbtree_walk( &tree, callback, NULL);
   mulle_rbtree_done( &tree);

   return( 0);
}
