//
//  mulle_pointerarray.c
//  mulle-container
//
//  Created by Nat! on 03.11.16.
//  Copyright © 2016 Mulle kybernetiK. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#include "mulle--pointerarray-debug.h"

#include "include-private.h"


static void   default_item_printer( struct mulle_buffer *buffer,
                                    void *item,
                                    void *userinfo)
{
   MULLE_C_UNUSED( userinfo);

   mulle_buffer_sprintf( buffer, "%p", item);
}


// use this only for debugging
void   mulle__pointerarray_describe_buffer_callback( struct mulle__pointerarray *array,
                                                     struct mulle_buffer *buffer,
                                                     mulle_container_item_printer_t *callback,
                                                     void *userinfo)
{
   char     *sep;
   size_t   n;
   void     *item;

   if( ! array)
   {
      mulle_buffer_add_string( buffer, "NULL");
      return;
   }

   if( ! callback)
      callback = default_item_printer;

   n = mulle__pointerarray_get_count( array);
   switch( n)
   {
   case 0 : mulle_buffer_sprintf( buffer, "{}");
            return;
   case 1 : mulle_buffer_sprintf( buffer, "{ %p }", 
                                  mulle__pointerarray_get( array, 0));
            return;
   }

   sep = "   ";
   mulle_buffer_add_string( buffer, "{\n");
   mulle__pointerarray_for( array, item)
   {
      mulle_buffer_add_string( buffer, sep);
      (*callback)( buffer, item, userinfo);
      sep = ",\n   ";
   }
   mulle_buffer_add_string( buffer, "\n}");
}




void   mulle__pointerarray_describe_buffer( struct mulle__pointerarray *array,
                                            struct mulle_buffer *buffer)
{
   mulle__pointerarray_describe_buffer_callback( array, buffer, NULL, NULL);
}


// use this only for debugging
char   *mulle__pointerarray_describe( struct mulle__pointerarray *array)
{
   char   *s;

   mulle_buffer_do_string( buffer, NULL, s)
   {
      mulle__pointerarray_describe_buffer( array, buffer);
   }
   return( s);
}

