/*
 *  (c) 2018 nat 
 *
 *  version:  major, minor, patch
 */
#define MULLE__REGEX_VERSION  ((0UL << 20) | (0 << 8) | 5)


static inline unsigned int   mulle_regex_get_version_major( void)
{
   return( MULLE__REGEX_VERSION >> 20);
}


static inline unsigned int   mulle_regex_get_version_minor( void)
{
   return( (MULLE__REGEX_VERSION >> 8) & 0xFFF);
}


static inline unsigned int   mulle_regex_get_version_patch( void)
{
   return( MULLE__REGEX_VERSION & 0xFF);
}
