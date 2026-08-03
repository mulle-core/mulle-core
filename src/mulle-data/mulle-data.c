//
//  mulle-data.c
//  mulle-data
//
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  All rights reserved.
//
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
#include "mulle-data.h"


int   __MULLE_DATA_ranlib__;


uint32_t   mulle_data_get_version( void)
{
   return( MULLE__DATA_VERSION);
}


MULLE__DATA_GLOBAL
void   *mulle_data_search_data( struct mulle_data haystack,
                                struct mulle_data needle)
{
   char      *haystack_bytes;
   char      *needle_bytes;
   size_t    i;
   size_t    j;

   mulle_data_assert( haystack);
   mulle_data_assert( needle);

   if( ! haystack.bytes || ! needle.bytes || haystack.length < needle.length)
      return( NULL);


   haystack_bytes = (char *) haystack.bytes;
   needle_bytes   = (char *) needle.bytes;

   for( i = 0; i <= haystack.length - needle.length; i++)
   {
      j = 0;

      while( j < needle.length && haystack_bytes[ i + j] == needle_bytes[ j])
        j++;

      if( j == needle.length)
         return( &haystack_bytes[ i]);
   }

   return( NULL);
}
