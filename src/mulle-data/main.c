//
//  main.c
//  mulle-fnva1
//
//  Created by Nat! on 26.09.20
//  Copyright (c) 2020 Nat! - Mulle kybernetiK.
//  Copyright (c) 2020 Codeon GmbH.
//  All rights reserved.
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
#define MULLE_DATA_BUILD
 
#include "mulle-fnv1a.h"
#include <ctype.h>
#ifdef _WIN32
# include <malloc.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#pragma clang diagnostic ignored "-Wparentheses"

static void   print_hash( char *s, size_t len,
                          unsigned long long value,
                          char *prefix, char *suffix)
{
#ifdef _WIN32
   char   *buf = alloca( sizeof( char) * (len + 1));
#else
   char   buf[ len + 1];
#endif
   char   *s1, *s2;
   char   c;

   s1 = s;
   s2 = buf;
   while( c = *s1++)
      *s2++ = (char) toupper( c);
   *s2 = c;

   printf( "#define %s%s%s   0x%08llx  // \"%s\"\n",
            prefix,
            buf,
            suffix,
            value,
            s);
}

//
// grep through all words and get a CSV list of the hashes:
//
// grep -h -o -R --include "*.m" --include "*.h" -E '\w+' src \
//    | sort \
//    | sort -u \
//    | CSV="" xargs dependency/bin/mulle-objc-uniqueid
//
// Not that useful, because grep doesn't grep selectors well
//
int   main( int argc, char *argv[])
{
   unsigned long long   value;
   char                 **sentinel;
   char                 *csv;
   char                 *prefix;
   char                 *suffix;
   char                 *s;
   size_t               len;
   int                  mode;
   int                  i;

   if( argc < 2)
      goto usage;

   mode = 0;
   i = 1;
   if( argv[ i][ 0] == '-')
   {
      switch( argv[ i][ 1])
      {
      case '3' : mode = 32; break;
      case '6' : mode = 64; break;
      default  : goto usage;
      }
      ++i;
   }

   csv    = getenv( "CSV");
   prefix = getenv( "PREFIX");
   suffix = getenv( "SUFFIX");
   if( ! suffix)
      suffix = "_METHODID";

   sentinel = &argv[ argc];
   argv     = &argv[ i];

   if( argv == sentinel)
      goto usage;

   while( argv < sentinel)
   {
      s     = *argv++;
      len   = strlen( s);

      switch( mode)
      {
      default :
         value = (unsigned long long) _mulle_fnv1a( s, len);
         break;

      case 32 :
         value = (unsigned long long) _mulle_fnv1a_32( s, len);
         break;

      case 64 :
         value = (unsigned long long) _mulle_fnv1a_64( s, len);
         break;
      }

      if( csv)
      {
         printf( "%08llx;%s\n", value, s);
         continue;
      }

      if( ! prefix)
      {
         printf( "%08llx\n", value);
         continue;
      }

      print_hash( s, len, value, prefix, suffix);
   }

   return( 0);

usage:
   fprintf( stderr, "Usage:\n   mulle-fnva1 [-32|-64] <string>*\n"
                    "    Generates fnv1a hash values from strings.\n"
                    "\n"
                    "Options:\n"
                    "   -32    : 32 bit hash\n"
                    "   -64    : 64 bit hash\n"
                    "\n"
                    "Environment:\n"
                    "   CSV    : output hash;name\n"
                    "   PREFIX : output a #define with prefix\n"
                    "   SUFFIX : suffix for #define (only if prefix is defined)\n"
                    "\n");
   return( -1);
}
