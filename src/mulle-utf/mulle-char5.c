//
//  mulle_char5.c
//  mulle-utf
//
//  Created by Nat! on 02.05.16.
//  Copyright © 2016 Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
//  All rights reserved.
//
#define _XOPEN_SOURCE   700

#include "mulle-char5.h"


char   mulle_char5_lookup_table[ 128] =
{
   /* 00-1F */
    0, -1, -1, -1,  -1, -1, -1, -1,
   -1, -1, -1, -1,  -1, -1, -1, -1,
   -1, -1, -1, -1,  -1, -1, -1, -1,
   -1, -1, -1, -1,  -1, -1, -1, -1,

   /* 20-3F */
   -1, -1, -1, -1,  -1, -1, -1, -1,
   -1, -1, -1, -1,  -1, -1,  1, -1, /* . */
   -1, -1, -1, -1,  -1, -1, -1, -1,
   -1, -1, -1, -1,  -1, -1, -1, -1,

   /* 40-5F */
   -1,  2, -1,  3,   4,  5, -1, -1, /* A C D E */
   -1,  6, -1, -1,  -1, -1,  7,  8,
    9, -1, -1, 10,  11, -1, -1, -1,
   -1, -1, -1, -1,  -1, -1, -1, 12,

   /* 60-7F */
   -1, 13, 14, 15,  16, 17, 18, 19,
   20, 21, -1, -1,  22, 23, 24, 25,
   26, -1, 27, 28,  29, 30, -1, -1,
   -1, 31, -1, -1,  -1, -1, -1, -1,
};



int   mulle_char5_encode_character( int c)
{
   switch( c)
   {
   case  0  : return( 0);
   case '.' : return( 1);
   case 'A' : return( 2);
   case 'C' : return( 3);
   case 'D' : return( 4);
   case 'E' : return( 5);
   case 'I' : return( 6);
   case 'N' : return( 7);

   case 'O' : return( 8);
   case 'P' : return( 9);
   case 'S' : return( 10);
   case 'T' : return( 11);
   case '_' : return( 12);
   case 'a' : return( 13);
   case 'b' : return( 14);
   case 'c' : return( 15);

   case 'd' : return( 16);
   case 'e' : return( 17);
   case 'f' : return( 18);
   case 'g' : return( 19);
   case 'h' : return( 20);
   case 'i' : return( 21);
   case 'l' : return( 22);
   case 'm' : return( 23);

   case 'n' : return( 24);
   case 'o' : return( 25);
   case 'p' : return( 26);
   case 'r' : return( 27);
   case 's' : return( 28);
   case 't' : return( 29);
   case 'u' : return( 30);
   case 'y' : return( 31);
   }
   return( -1);
}


int   mulle_char5_is_char5string32( char *src, size_t len)
{
   char   *sentinel;

   if( len > mulle_char5_maxlength32)
      return( 0);

   sentinel = &src[ len];
   while( src < sentinel)
      switch( mulle_char5_lookup_character( *src++))
      {
      case 0  : return( 1);   // zero byte, ok fine!
      case -1 : return( 0);   // invalid char
      }

   return( 1);
}


int   mulle_char5_is_char5string64( char *src, size_t len)
{
   char   *sentinel;

   if( len > mulle_char5_maxlength64)
      return( 0);

   sentinel = &src[ len];
   while( src < sentinel)
      switch( mulle_char5_lookup_character( *src++))
      {
      case 0  : return( 1);
      case -1 : return( 0);
      }

   return( 1);
}


uint32_t   mulle_char5_encode32( char *src, size_t len)
{
   char       *s;
   char       *sentinel;
   char       c;
   int        char5;
   uint32_t   value;

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      c = *--s;
      if( ! c)
         continue;

      char5   = mulle_char5_lookup_character( c);
      assert( char5 > 0 && char5 < 0x20);
      assert( value << 5 >> 5 == value);  // hope the optimizer doesn't fck up
      value <<= 5;
      value  |= char5;
   }
   return( value);
}


uint64_t   mulle_char5_encode64( char *src, size_t len)
{
   char       *s;
   char       *sentinel;
   char       c;
   int        char5;
   uint64_t   value;

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      c = *--s;
      if( ! c)
         continue;

      char5 = mulle_char5_lookup_character( c);
      assert( char5 > 0 && char5 < 0x20);
      assert( value << 5 >> 5 == value);  // hope the optimizer doesn't fck up
      value <<= 5;
      value  |= char5;
   }
   return( value);
}


uint32_t   mulle_char5_encode32_utf16( mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *s;
   mulle_utf16_t   *sentinel;
   int             c;
   int             char5;
   uint32_t        value;

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      c = *--s;
      if( ! c)
         continue;

      char5   = mulle_char5_lookup_character( c);
      assert( char5 > 0 && char5 < 0x20);
      assert( value << 5 >> 5 == value);  // hope the optimizer doesn't fck up
      value <<= 5;
      value  |= char5;
   }
   return( value);
}


uint64_t   mulle_char5_encode64_utf16( mulle_utf16_t *src, size_t len)
{
   mulle_utf16_t   *s;
   mulle_utf16_t   *sentinel;
   int             c;
   int             char5;
   uint64_t        value;

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      c = *--s;
      if( ! c)
         continue;

      char5 = mulle_char5_lookup_character( c);
      assert( char5 > 0 && char5 < 0x20);
      assert( value << 5 >> 5 == value);  // hope the optimizer doesn't fck up
      value <<= 5;
      value  |= char5;
   }
   return( value);
}




uint32_t   mulle_char5_encode32_utf32( mulle_utf32_t *src, size_t len)
{
   mulle_utf32_t   *s;
   mulle_utf32_t   *sentinel;
   int             c;
   int             char5;
   uint32_t        value;

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      c = *--s;
      if( ! c)
         continue;

      char5   = mulle_char5_lookup_character( c);
      assert( char5 > 0 && char5 < 0x20);
      assert( value << 5 >> 5 == value);  // hope the optimizer doesn't fck up
      value <<= 5;
      value  |= char5;
   }
   return( value);
}


uint64_t   mulle_char5_encode64_utf32( mulle_utf32_t *src, size_t len)
{
   mulle_utf32_t   *s;
   mulle_utf32_t   *sentinel;
   int             c;
   int             char5;
   uint64_t        value;

   value    = 0;
   sentinel = src;
   s        = &src[ len];
   while( s > sentinel)
   {
      c = *--s;
      if( ! c)
         continue;

      char5 = mulle_char5_lookup_character( c);
      assert( char5 > 0 && char5 < 0x20);
      assert( value << 5 >> 5 == value);  // hope the optimizer doesn't fck up
      value <<= 5;
      value  |= char5;
   }
   return( value);
}


size_t  mulle_char5_decode32( uint32_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;
   int    char5;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      char5  = value & 0x1F;
      *s++ = (char) mulle_char5_decode_character( char5);

      value >>= 5;
   }
   return( s - dst);
}


size_t  mulle_char5_decode64( uint64_t value, char *dst, size_t len)
{
   char   *s;
   char   *sentinel;
   int    char5;

   s        = dst;
   sentinel = &s[ len];
   while( s < sentinel)
   {
      if( ! value)
         break;

      char5 = value & 0x1F;
      *s++  = (char) mulle_char5_decode_character( char5);

      value >>= 5;
   }
   return( s - dst);
}


int   mulle_char5_get64( uint64_t value, unsigned int index)
{
   int   char5;

   do
   {
      char5 = value & 0x1F;
      if( ! value)
         break;

      value >>= 5;
   }
   while( index--);

   return( mulle_char5_decode_character( char5));
}


int   mulle_char5_get32( uint32_t value, unsigned int index)
{
   int   char5;

   do
   {
      char5 = value & 0x1F;
      if( ! value)
         break;

      value >>= 5;
   }
   while( index--);

   return( mulle_char5_decode_character( char5));
}
