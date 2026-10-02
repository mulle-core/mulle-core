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


static int  callback( struct mulle_rbnode *node, void *userinfo)
{
   char   *payload;

   payload = _mulle_rbnode_get_payload( node);
   printf( "%s\n", payload);
   mulle_free( payload);

   return( 1);
}


//
// https://stackoverflow.com/questions/12252103/read-line-from-stdin-blocking
//
int main( int argc, const char * argv[])
{
   char                  *line = NULL;
   size_t                size = 0;
   size_t                len;
   struct mulle__rbtree  tree;
   struct mulle_rbnode   *node;

   _mulle__rbtree_init( &tree, NULL);

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

      node = _mulle__rbtree_new_node( &tree, mulle_strdup( line));
      if( _mulle__rbtree_insert_node( &tree, node, (void *) strcmp))
         abort();
      fprintf( stderr , "inserted: %s\n", line);
   }

   // clean up payloads (and print)
   _mulle__rbtree_walk( &tree, callback, NULL);

   _mulle__rbtree_done( &tree);

   return( 0);
}
