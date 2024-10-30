#include "mulle-utf-type.h"
#include "mulle-utf-scan.h"
#include "mulle-utf-rover.h"

#include <limits.h>


enum scan_state
{
   init_scan,
   has_sign,
   has_value,
   is_unsigned,
   has_overflown
};



//
// returns 0  : is long long
// returns 1  : is unsigned long long
// returns -1 : is not valid
// returns -2 : has overflown
// returns -3 : contains trailing garbage
//

// JUST THE SAME WITH DIFFERENT TYPES :(

int   _mulle_utf_rover_longlong_decimal( struct mulle_utf_rover *rover,
                                         long long *p_value)
{
   enum scan_state      state;
   int                  sign;
   unsigned long long   value;
   unsigned long long   old;
   mulle_utf32_t        c;
   int                  rval;

   state    = init_scan;
   sign     = 1;
   value    = 0;

   while( _mulle_utf_rover_has_character( rover))
   {
      c = _mulle_utf_rover_next_character( rover);

      if( c >= '0' && c <= '9')  // use other unicode chars too ??
      {                          // but that is super slow and for what gain ?
         switch( state)
         {
         case init_scan :
         case has_sign  :
            state = has_value; // fall thru

         case has_value :
            old   = value;
            value = value * 10 + (c - '0');
            if( value >= old)
               continue;
            state = has_overflown;

         default:
            break;
         }
      }

      if( state == init_scan)
      {
         if( c == '-')
         {
            state = has_sign;
            sign  = -1;
            continue;
         }

         if( c == '+')
         {
            state = has_sign;
            continue;
         }
      }

      // put back garbage char
      _mulle_utf_rover_dial_back( rover);
      break;
   }

   switch( state)
   {
   // fail
   case init_scan     :
      return( mulle_utf_is_invalid); // empty string

   case has_sign      :
      _mulle_utf_rover_dial_back( rover);
      return( mulle_utf_is_invalid);

   case has_overflown :
      _mulle_utf_rover_dial_back( rover);
      return( mulle_utf_has_overflown);

   default :
      ;
   }

   rval = mulle_utf_is_valid;
   if( sign == 1)
   {
      *p_value = value;
      if( value > LLONG_MAX)
         rval = mulle_utf_is_too_large_for_signed;
   }
   else
   {
      if( value > (unsigned long long) LLONG_MIN)
      {
         _mulle_utf_rover_dial_back( rover);
         return( mulle_utf_has_overflown);
      }
      *p_value = -(long long) value;
   }

   if( _mulle_utf_rover_has_character( rover))
      rval |= mulle_utf_has_trailing_garbage;
   return( rval);
}


int   _mulle_utf8_scan_longlong_decimal( char **p_s,
                                         size_t len,
                                         long long *p_value)
{
   struct mulle_utf_rover   rover;
   int                      rval;

   _mulle_utf8_rover_init( &rover, *p_s, len);

   rval = _mulle_utf_rover_longlong_decimal( &rover, p_value);
   *p_s = _mulle_utf_rover_get_current( &rover);
   return( rval);
}


int   _mulle_utf16_scan_longlong_decimal( mulle_utf16_t **p_s,
                                          size_t len,
                                          long long *p_value)
{
   struct mulle_utf_rover   rover;
   int                      rval;

   _mulle_utf16_rover_init( &rover, *p_s, len);

   rval = _mulle_utf_rover_longlong_decimal( &rover, p_value);
   *p_s = _mulle_utf_rover_get_current( &rover);
   return( rval);
}


int   _mulle_utf32_scan_longlong_decimal( mulle_utf32_t **p_s,
                                          size_t len,
                                          long long *p_value)
{
   struct mulle_utf_rover   rover;
   int                      rval;

   _mulle_utf32_rover_init( &rover, *p_s, len);

   rval = _mulle_utf_rover_longlong_decimal( &rover, p_value);
   *p_s = _mulle_utf_rover_get_current( &rover);
   return( rval);
}



// CODE FRIEDHOF. THIS IS QUITE A BIT FASTER I ASSUME

#if 0

//
// returns 0  : is long long
// returns 1  : is unsigned long long
// returns -1 : is not valid
// returns -2 : has overflown
// returns -3 : contains trailing garbage
//
int   _mulle_utf32_scan_longlong_decimal( mulle_utf32_t **p_s,
                                          size_t len,
                                          long long *p_value)
{
   enum scan_state      state;
   int                  sign;
   unsigned long long   value;
   unsigned long long   old;
   mulle_utf32_t        *s;
   mulle_utf32_t        *sentinel;
   mulle_utf32_t        c;
   int                  rval;

   state    = init_scan;
   s        = *p_s;
   sentinel = &s[ len];
   sign     = 1;
   value    = 0;

   for( ;s < sentinel; s++)
   {
      c = *s;
      if( c >= '0' && c <= '9')  // use other unicode chars too ??
      {                          // but that is super slow and for what gain ?
         switch( state)
         {
         case init_scan :
         case has_sign  :
            state = has_value; // fall thru

         case has_value :
            old   = value;
            value = value * 10 + (c - '0');
            if( value < old)
            {
               state = has_overflown;
               break;
            }
            continue;
         }
      }

      if( state == init_scan)
      {
         if( c == '-')
         {
            state = has_sign;
            sign  = -1;
            continue;
         }

         if( c == '+')
         {
            state = has_sign;
            continue;
         }
      }

      break;
   }

   switch( state)
   {
   // fail
   case init_scan     :
      return( mulle_utf_is_invalid); // empty string

   case has_sign      :
      *p_s = s - 1 ;
      return( mulle_utf_is_invalid);

   case has_overflown :
      *p_s = s;
      return( mulle_utf_has_overflown);
   }

   *p_s = s;
   rval = mulle_utf_is_valid;
   if( sign == 1)
   {
      *p_value = value;
      if( value > LLONG_MAX)
         rval = mulle_utf_is_too_large_for_signed;
   }
   else
   {
      if( value > (unsigned long) LLONG_MIN)
      {
         *p_s = s - 1;
         return( mulle_utf_has_overflown);
      }
      *p_value = -(long long) value;
   }

   if( s != sentinel)
      rval |= mulle_utf_has_trailing_garbage;
   return( rval);
}

#endif
