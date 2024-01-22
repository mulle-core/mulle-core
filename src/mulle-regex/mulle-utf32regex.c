/*
 * mulle_unicode_regex_compile and mulle_unicode_regex_execute --
 * mulle_unicode_regex_substitute is elsewhere
 */
#include "mulle-utf32regex.h"
#include "mulle-utf32regex-private.h"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* (nat) to unconfuse/confuse the code, the "program space" is pointed
 *       by a unknown pointer called node. This should make arithmetic harder,
 *       which is good, because it catches more errors. Also inside the code,
 *       it is clearer what is a real "char", "mulle_utf32_t" or just
 *       amorphous buffer space
 */
typedef struct _anonymous_node   node;

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; '\0' if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that mulle_unicode_regex_compile() supplies a regmust only if the r.e. contains
 * something  * potentially expensive (at present, the only such thing detected
 * is * or + at the start of the r.e., which can involve a lot of backup).
 * Regmlen is  * supplied because the test in mulle_unicode_regex_execute() needs
 * it and mulle_unicode_regex_compile() is computing it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition	number	opnd?	meaning */
#define  END      0  /* no	End of program. */
#define  BOL      1  /* no	Match beginning of line. */
#define  EOL      2  /* no	Match end of line. */
#define  ANY      3  /* no	Match any character. */
#define  ANYOF    4  /* str	Match any of these. */
#define  ANYBUT   5  /* str	Match any but one of these. */
#define  BRANCH   6  /* node	Match this, or the next..\&. */
#define  BACK     7  /* no	"next" ptr points backward. */
#define  EXACTLY  8  /* str	Match this string. */
#define  NOTHING  9  /* no	Match empty string. */
#define  STAR    10  /* node	Match this 0 or more times. */
#define  PLUS    11    /* node	Match this 1 or more times. */
#define  OPEN    (12 - 1) /* no	Sub-RE starts here. */
/*	OPEN+1 is number 1, etc. */
#define  CLOSE   256 /* no	Analogous to OPEN, but goes down so CLOSE - 1. */
                     /* matches OPEN + 1*/

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
static inline int  OPCODE( node *p)
{
   return( *(unsigned char *) p);
}


static inline unsigned int   NEXT_NODE_OFFSET( node *p)
{
   unsigned char   *q;
   unsigned int    offset;

   q      = (unsigned char *) p;
   offset = ((unsigned int) q[ 3] << 16) + ((unsigned int) q[ 2] << 8) + q[ 1];
   assert( (int) offset >= 0);
   return( offset);
}


static inline node   *_NEXT(node *p)
{
   return( (node *) ((char *) p + 4));
}


static inline mulle_utf32_t   *OPERAND(node *p)
{
   return( (mulle_utf32_t *) ((char *) p + 4));
}


/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions. What this code is mostly missing is the {2} specifier
 * how many repetitions are sought. That's probably no super convenient
 * to "explode" in preprocessing code.
 */
#define  ISREPN(c)   ((c) == '*' || (c) == '+' || (c) == '?')
#define  META     "^$.[()|?+*\\"

/*
 * Flags to be passed up and down.
 */
#define  HASWIDTH 01   /* Known never to match null string. */
#define  SIMPLE   02   /* Simple enough to be STAR/PLUS operand. */
#define  SPSTART  04   /* Starts with * or +. */
#define  WORST    0    /* Worst case. */

/*
 * Work-variable struct for mulle_unicode_regex_compile().
 */
struct comp
{
   mulle_utf32_t   *regparse;     /* Input-scan pointer. */
   int             regnpar;       /* () count. */
   node            *regcode;      /* Code-emit pointer; &regdummy = don't. */
   long            regsize;       /* Code size. */
   char            regdummy[4];   /* NOTHING, 0 next ptr */
   jmp_buf         bail;
};


#define FAIL_COMPILE( co, m, s)     \
do                                  \
{                                   \
   longjmp( (co)->bail, m);         \
}                                   \
while( 0)

#define  EMITTING(co)   ((co)->regcode != (void *) (co)->regdummy)

/*
 * Forward declarations for mulle_unicode_regex_compile()'s friends.
 */
static node   *regex_generic( struct comp *co, int paren, int *flagp);
static node   *regex_branch( struct comp *co, int *flagp);
static node   *regex_piece( struct comp *co, int *flagp);
static node   *regex_atom( struct comp *co, int *flagp);
static node   *regex_node( struct comp *co, int op);
static node   *regex_next_node(node *node);

static void   regex_c( struct comp *co, int c);
static void   regex_insert_ahead( struct comp *co, int op, node *opnd);
static void   regex_set_tail( struct comp *co, node *p, node *val);
static void   regex_set_tail_if_needed( struct comp *co, node *p, node *val);


/* convenience, convenience, convenience */
mulle_utf32_t   *mulle_utf32_match( mulle_utf32_t *pattern, mulle_utf32_t *s)
{
   regexp    *p;
   mulle_utf32_t   *found;

   p = mulle_utf32regex_compile( pattern);
   if( ! p)
      return( NULL);

   found = NULL;
   if( mulle_utf32regex_execute( p, s) > 0)
      found = p->startp[ 0];
   mulle_utf32regex_free( p);

   return( found);
}


mulle_utf32_t   *mulle_utf32_substitute( mulle_utf32_t *pattern,
                                         mulle_utf32_t *replacement,
                                         mulle_utf32_t *s)
{
   regexp          *p;
   mulle_utf32_t   *buf;
   size_t          len;
   int             rval;

   p = mulle_utf32regex_compile( pattern);
   if( ! p)
      return( NULL);

   if( mulle_utf32regex_execute( p, s) <= 0)
   {
      mulle_utf32regex_free( p);
      return( NULL);
   }

   /* figure out, how much space is needed */

   buf  = NULL;
   rval = 0;
   // len in bytes
   len  = mulle_utf32regex_substitution_buffer_size( p, replacement);
   if( len)
   {
      buf = mulle_malloc( len);
      if( buf)
         rval = mulle_utf32regex_substitute( p, replacement, buf, len, 1);
   }
   mulle_utf32regex_free( p);

   if( rval)
   {
      mulle_free( buf);
      buf = NULL;
   }

   return( buf);
}


struct mulle_range   mulle_utf32regex_range_for_index( struct mulle_utf32regex *rp, unsigned int i)
{
   struct mulle_range   range;
   regexp               *p;

   p = rp;
   if( i >= MULLE_UTF32REGEX_NSUBEXP || ! p->startp[ i])
   {
      range.location = LONG_MAX;
      range.length   = 0;
      return( range);
   }

   range.length   = p->endp[ i] - p->startp[ i];
   range.location = p->startp[ i] - p->start;
   return( range);
}


/*
 *  - mulle_unicode_regex_compile - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
struct mulle_utf32regex   *mulle_utf32regex_compile( mulle_utf32_t *exp)
{
   regexp        *r;
   node          *scan;
   int           flags;
   struct comp   co;

   assert( exp);

   /* First pass: determine size, legality. */
   co.regparse    = (mulle_utf32_t *) exp;
   co.regnpar     = 1;
   co.regsize     = 0L;
   co.regdummy[0] = NOTHING;
   co.regdummy[1] = co.regdummy[2] = co.regdummy[3] = 0;
   co.regcode     = (node *) co.regdummy;

   if( (errno = setjmp( co.bail)))
      return( NULL);

   regex_c( &co, MULLE_UTF32REGEX_MAGIC);
   regex_generic( &co, 0, &flags);

   /* Small enough for pointer-storage convention? */
   if( co.regsize >= 0x7fffL) /* Probably could be 0xffffL. */
      FAIL_COMPILE( &co, EFBIG, "too complex");

   /* Allocate space. */
   r = (regexp *) mulle_malloc( sizeof( regexp) + (size_t) co.regsize);
   if( ! r)
      FAIL_COMPILE( &co, ENOMEM, "out of memory on your nintendo ds");
#ifndef NDEBUG
   memset( r, 0xFA, sizeof( regexp) + (size_t) co.regsize);
#endif
   /* Second pass: emit code. */
   co.regparse = (mulle_utf32_t *) exp;
   co.regnpar  = 1;
   co.regcode  = (node *) r->program;

   regex_c( &co, MULLE_UTF32REGEX_MAGIC);
   regex_generic( &co, 0, &flags);

   /* Dig out information for optimizations. */
   r->regstart = '\0';     /* Worst-case defaults. */
   r->reganch  = 0;
   r->regmust  = NULL;
   r->regmlen  = 0;
   /* skip MULLE_UTF32REGEX_MAGIC */
   scan = (node *) (r->program + sizeof( mulle_utf32_t));     /* First BRANCH. */

   if( OPCODE(regex_next_node(scan)) == END)    /* Only one top-level choice. */
   {
      scan = _NEXT(scan);

      /* Starting-point info. */
      if( OPCODE(scan) == EXACTLY)
         r->regstart = *OPERAND(scan);
      else if( OPCODE(scan) == BOL)
         r->reganch = 1;

      /*
       * If there's something expensive in the r.e., find the
       * longest literal string that must appear and make it the
       * regmust.  Resolve ties in favor of later strings, since
       * the regstart check works with the beginning of the r.e.
       * and avoiding duplication strengthens checking.  Not a
       * strong reason, but sufficient in the absence of others.
       */
      if( flags & SPSTART)
      {
         mulle_utf32_t   *longest = NULL;
         size_t          len      = 0;
         size_t          test_len = 0;

         for(; scan; scan = regex_next_node(scan))
            if( OPCODE(scan) == EXACTLY)
            {
               test_len = mulle_utf32_strlen(OPERAND(scan));

               if( test_len >= len)
               {
                  longest = OPERAND(scan);
                  len     = test_len;
               }
            }

         r->regmust = longest;
         r->regmlen = (int) len;
      }
   }

   return( (struct mulle_utf32regex  *) r);
}


/*
 *  - regex_generic - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static node   *regex_generic( struct comp *co, int paren, int *flagp)     /* Parenthesized? */
{
   node   *ret;
   node   *br;
   node   *ender;
   int     parno;
   int     flags;

   *flagp = HASWIDTH;   /* Tentatively. */

   if( paren)
   {
      /* Make an OPEN node. */
      if( co->regnpar >= MULLE_UTF32REGEX_NSUBEXP)
         FAIL_COMPILE( co, EMLINK, "too many ()");

      parno = co->regnpar;
      assert( parno >= 1 && parno < MULLE_UTF32REGEX_NSUBEXP);
      co->regnpar++;
      ret = regex_node(co, OPEN + parno);
   }

   /* Pick up the branches, linking them together. */
   br = regex_branch(co, &flags);

   if( paren)
      regex_set_tail(co, ret, br);   /* OPEN -> first. */
   else
      ret = br;

   *flagp &= ~(~flags & HASWIDTH); /* Clear bit if bit 0. */
   *flagp |= flags & SPSTART;

   while( *co->regparse == '|')
   {
      co->regparse++;
      br = regex_branch(co, &flags);

      regex_set_tail(co, ret, br);   /* BRANCH -> BRANCH. */
      *flagp &= ~(~flags & HASWIDTH);
      *flagp |= flags & SPSTART;
   }

   /* Make a closing node, and hook it on the end. */
   ender = regex_node(co, (paren) ? CLOSE - parno : END);
   regex_set_tail(co, ret, ender);

   /* Hook the tails of the branches to the closing node. */
   for( br = ret; br; br = regex_next_node(br))
      regex_set_tail_if_needed(co, br, ender);

   /* Check for proper termination. */
   if( paren && *co->regparse++ != ')')
   {
      FAIL_COMPILE( co, EINVAL, "unterminated ()");
   }
   else
      if (!paren && *co->regparse != '\0')
      {
         if (*co->regparse == ')')
            FAIL_COMPILE( co, EINVAL, "unmatched ()");
         else
            FAIL_COMPILE( co, EINVAL, "internal error: junk on end");
         /* NOTREACHED */
      }

   return( ret);
}


/*
 *  - regex_branch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static node   *regex_branch( struct comp *co, int *flagp)
{
   node            *ret;
   node            *chain;
   node            *latest;
   int             flags;
   mulle_utf32_t   c;

   *flagp = WORST;            /* Tentatively. */

   ret   = regex_node(co, BRANCH);
   chain = NULL;

   while((c = *co->regparse) != '\0' && c != '|' && c != ')')
   {
      latest  = regex_piece(co, &flags);
      *flagp |= flags & HASWIDTH;

      if( ! chain)      /* First piece. */
         *flagp |= flags & SPSTART;
      else
         regex_set_tail(co, chain, latest);

      chain = latest;
   }

   if( ! chain)         /* Loop ran zero times. */
      (void) regex_node(co, NOTHING);

   return( ret);
}


/*
 *  - regex_piece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static node   *regex_piece( struct comp *co, int *flagp)
{
   node   *ret;
   mulle_utf32_t     op;
   node   *next;
   int         flags;

   ret = regex_atom(co, &flags);
   op  = *co->regparse;

   if( ! ISREPN( op))
   {
      *flagp = flags;
      return( ret);
   }

   if( ! (flags & HASWIDTH) && (op != '?'))
      FAIL_COMPILE( co, EINVAL, "*+ operand could be empty");

   switch( op)
   {
   case '*':
      *flagp = WORST | SPSTART;
      break;

   case '+':
      *flagp = WORST | SPSTART | HASWIDTH;
      break;

   case '?':
      *flagp = WORST;
      break;
   }

   if((op == '*') && (flags & SIMPLE))
      regex_insert_ahead(co, STAR, ret);
   else if( op == '*')
   {
      /* Emit x* as (x&|), where & means "self". */
      regex_insert_ahead(co, BRANCH, ret);      /* Either x */
      regex_set_tail_if_needed(co, ret, regex_node(co, BACK)); /* and loop */
      regex_set_tail_if_needed(co, ret, ret);      /* back */
      regex_set_tail(co, ret, regex_node(co, BRANCH)); /* or */
      regex_set_tail(co, ret, regex_node(co, NOTHING));   /* null. */
   }
   else if((op == '+') && (flags & SIMPLE))
      regex_insert_ahead(co, PLUS, ret);
   else if( op == '+')
   {
      /* Emit x+ as x(&|), where & means "self". */
      next = regex_node(co, BRANCH);      /* Either */
      regex_set_tail(co, ret,               next);
      regex_set_tail(co, regex_node(co, BACK), ret);   /* loop back */
      regex_set_tail(co, next,              regex_node(co, BRANCH)); /* or */
      regex_set_tail(co, ret,               regex_node(co, NOTHING)); /* null. */
   }
   else if( op == '?')
   {
      /* Emit x? as (x|) */
      regex_insert_ahead(co, BRANCH, ret);      /* Either x */
      regex_set_tail(co, ret, regex_node(co, BRANCH)); /* or */
      next = regex_node(co, NOTHING);     /* null. */
      regex_set_tail(co, ret, next);
      regex_set_tail_if_needed(co, ret, next);
   }

   co->regparse++;

   if( ISREPN( *co->regparse))
      FAIL_COMPILE( co, EINVAL, "nested *?+");

   return( ret);
}


static size_t   mulle_utf32_metacspn( mulle_utf32_t *s1)
{
   mulle_utf32_t  c;
   size_t   len;

   len = 0;
   while( (c = *s1++))
   {
      switch( c)  /* #define  META     "^$.[()|?+*\\" */
      {
      case '^'  :
      case '$'  :
      case '.'  :
      case '['  :
      case '('  :
      case ')'  :
      case '|'  :
      case '?'  :
      case '+'  :
      case '*'  :
      case '\\' : return( len);
      }
      ++len;
   }
   return( len);
}


/*
 *  - regex_atom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static node   *regex_atom( struct comp *co, int *flagp)
{
   node   *ret;
   int         flags;

   *flagp = WORST;      /* Tentatively. */

   switch( *co->regparse++)
   {
      case '^':
         ret = regex_node(co, BOL);
         break;

      case '$':
         ret = regex_node(co, EOL);
         break;

      case '.':
         ret     = regex_node(co, ANY);
         *flagp |= HASWIDTH | SIMPLE;
         break;

      case '[':
      {
         int   range;
         int   rangeend;
         int   c;

         if( *co->regparse == '^')     /* Complement of range. */
         {
            ret = regex_node(co, ANYBUT);
            co->regparse++;
         }
         else
            ret = regex_node(co, ANYOF);

         if( ((c = *co->regparse) == ']') || (c == '-'))
         {
            regex_c(co, c);
            co->regparse++;
         }

         while( (c = *co->regparse++) && c != ']')
         {
            if( c != '-')
               regex_c(co, c);
            else
               if( ((c = *co->regparse) == ']') || (c == '\0'))
                  regex_c(co, '-');
               else
               {
                  range    = (unsigned char) *(co->regparse - 2);
                  rangeend = (unsigned char) c;

                  if( range > rangeend)
                     FAIL_COMPILE( co, EINVAL, "invalid [] range");

                  for( range++; range <= rangeend; range++)
                     regex_c(co, range);

                  co->regparse++;
               }
         }

         regex_c( co, '\0');

         if( c != ']')
            FAIL_COMPILE( co, EINVAL, "unmatched []");

         *flagp |= HASWIDTH | SIMPLE;
         break;
      }

      case '(':
         ret = regex_generic( co, 1, &flags);
         *flagp |= flags & (HASWIDTH | SPSTART);
         break;

      case '\0':
      case '|':
      case ')':
         /* supposed to be caught earlier */
         FAIL_COMPILE( co, EDEADLK, "internal error: \\0|) unexpected");
         break;

      case '?':
      case '+':
      case '*':
         FAIL_COMPILE( co, EINVAL, "?+* follows nothing");
         break;

      case '\\':

         if( *co->regparse == '\0')
            FAIL_COMPILE( co, EINVAL, "trailing \\");

         ret = regex_node(co, EXACTLY);
         regex_c(co, *co->regparse++);
         regex_c(co, '\0');
         *flagp |= HASWIDTH | SIMPLE;
         break;

      default:
      {
         size_t    len;
         mulle_utf32_t   ender;

         co->regparse--;
         len = mulle_utf32_metacspn(co->regparse);

         if( len == 0)
            FAIL_COMPILE( co, EINVAL, "internal error: strcspn 0");

         ender = *(co->regparse + len);

         if((len > 1) && ISREPN(ender))
            len--;      /* Back off clear of ?+* operand. */

         *flagp |= HASWIDTH;

         if( len == 1)
            *flagp |= SIMPLE;

         ret = regex_node(co, EXACTLY);

         for(; len > 0; len--)
            regex_c(co, *co->regparse++);

         regex_c( co, '\0');
         break;
      }
   }

   return( ret);
}


/*
 *  - regex_node - emit a node
 */ /* Location. */
static node   *regex_node( struct comp *co, int op)
{
   node   *ret = co->regcode;
   char   *ptr;

   if( ! EMITTING(co))
   {
      co->regsize += 4;
      return( ret);
   }


   ptr         = (char *) ret;

   assert( ptr[ -1] != (char) 0xFA);
   assert( ptr[ 0] == (char) 0xFA);

   *ptr++      = (char) op;
   *ptr++      = '\0'; /* Null next pointer. */
   *ptr++      = '\0';
   *ptr++      = '\0';
   co->regcode = (node *) ptr;

   return( ret);
}


/*
 *  - regex_c - emit (if appropriate) a byte of code
 */
static void   regex_c( struct comp *co, int b)
{
   if( EMITTING(co))
   {
      // used to be just *((mulle_utf32_t *) co->regcode)++  ... <nerv>
      *(mulle_utf32_t *) co->regcode = b;
      co->regcode = (node *) ((mulle_utf32_t *) co->regcode + 1);
   }
   else
      co->regsize += sizeof( mulle_utf32_t);
}


/*
 *  - regex_insert_ahead - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void   regex_insert_ahead( struct comp *co, int op, node *opnd)
{
   char   *place;

   if( ! EMITTING(co))
   {
      co->regsize += 4;
      return;
   }

   (void) memmove((char *) opnd + 4, opnd, (size_t) ((char *) co->regcode - (char *) opnd));
   co->regcode = (node *) ((char *) co->regcode + 4);

   place    = (char *) opnd;     /* Op node, where operand used to be. */
   *place++ = (char) op;
   *place++ = '\0';
   *place++ = '\0';
   *place++ = '\0';
}


/*
 *  - regex_set_tail - set the next-pointer at the end of a node chain
 */
static void   regex_set_tail( struct comp *co, node *p, node *val)
{
   node        *scan;
   node        *temp;
   char        *s;
   ptrdiff_t   offset;

   if( ! EMITTING( co))
      return;

   /* Find last node. */
   for( scan = p; (temp = regex_next_node(scan)); scan = temp)
      continue;

   if( OPCODE( scan) == BACK)
   {
      offset = (char *) scan - (char *) val;
      assert( scan >= val);
   }
   else
   {
      offset = (char *) val - (char *) scan;
      assert( val >= scan);
   }

   s        = (char *) scan;
   *(s + 1) = (unsigned int) offset & 0xFF;
   *(s + 2) = ((unsigned int) offset >> 8) & 0xFF;
   *(s + 3) = ((unsigned int) offset >> 16) & 0xFF;

   assert( regex_next_node( scan) == val);
}


/*
 *  - regex_set_tail_if_needed - regex_set_tail on operand of first argument; nop if operandless
 */
static void   regex_set_tail_if_needed( struct comp *co, node *p, node *val)
{
   /* "Operandless" and "op != BRANCH" are synonymous in practice. */
   if( ! EMITTING(co) || (OPCODE(p) != BRANCH))
      return;

   regex_set_tail( co, _NEXT( p), val);
}


/*
 * mulle_unicode_regex_execute and friends
 */

/*
 * Work-variable struct for mulle_unicode_regex_execute().
 */
struct exec
{
   mulle_utf32_t   *reginput;      /* String-input pointer. */
   mulle_utf32_t   *regbol;        /* Beginning of input, for ^ check. */
   mulle_utf32_t   **regstartp;    /* Pointer to startp array. */
   mulle_utf32_t   **regendp;      /* Ditto for endp. */
};

/*
 * Forwards.
 */
static int      regex_try_match( struct exec *ep, regexp *rp, mulle_utf32_t *string);
static int      regex_match( struct exec *ep, node *prog);
static size_t   regex_repeat( struct exec *ep, node *node);

#ifdef DEBUG
int           regnarrate = 0;
void          regex_dump();
static char * regex_string_from_opcode();

#endif

/*
 *  - mulle_unicode_regex_execute - match a regexp against a string
 */
int    mulle_utf32regex_execute(  struct mulle_utf32regex *p, mulle_utf32_t *string)
{
   regexp         *prog = (regexp *) p;
   mulle_utf32_t  *s;
   struct exec    ex;

   /* Be paranoid. */
   assert( prog);
   assert( string);

   /* Check validity of program. */
   if( *(mulle_utf32_t *) prog->program != MULLE_UTF32REGEX_MAGIC)
   {
      errno = EINVAL;
      return( -EINVAL);
   }

   /* If there is a "must appear" string, look for it. */
   if( prog->regmust && ! mulle_utf32_strstr( string, prog->regmust))
      return( 0);

   /* Mark beginning of line for ^ . */
   ex.regbol    = string;
   ex.regstartp = prog->startp;
   ex.regendp   = prog->endp;

   /* memorize for later computation of ranges */
   prog->start = string;

   /* Simplest case:  anchored match need be tried only once. */
   if( prog->reganch)
      return( regex_try_match( &ex, prog, string));

   /* Messy cases:  unanchored match. */
   if( prog->regstart)
   {
      /* We know what char it must start with. */
      for( s = string; s; s = *s ? mulle_utf32_strchr( s + 1, prog->regstart) : NULL)
         if( regex_try_match (&ex, prog, s))
            return( 1);

      return( 0);
   }

   /* We don't -- general case. */
   for( s = string; ! regex_try_match( &ex, prog, s); s++)
      if( ! *s)
         return( 0);
   return( 1);
}


/*
 *  - regex_try_match - try match at specific point
 */
/* 0 failure, 1 success */
static int   regex_try_match( struct exec *ep, regexp *prog, mulle_utf32_t *s)
{
   /* clean */
   memset( prog->startp, 0, (char *) &prog->endp[ MULLE_UTF32REGEX_NSUBEXP] - (char *) prog->startp);

   ep->reginput = s;

   if( regex_match( ep, (node *) (prog->program + sizeof( mulle_utf32_t))))
   {
      prog->startp[ 0] = s;
      prog->endp[ 0]   = ep->reginput;
      return( 1);
   }
   return( 0);
}


/*
 *  - regex_match - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
/* 0 failure, 1 success */
static int  regex_match( struct exec *ep, node *prog)
{
   node   *scan;   /* Current node. */
   node   *next;   /* Next node. */
   int    c;

#ifdef DEBUG
   auto char    buf[ 64];

   if( prog && regnarrate)
      fprintf(stderr, "%s(\n", regex_string_from_opcode(prog, buf));
#endif

   for( scan = prog; scan; scan = next)
   {
#ifdef DEBUG
      if( regnarrate)
         fprintf( stderr, "%s...\n", regex_string_from_opcode( scan, buf));
#endif
      next = regex_next_node(scan);

      switch( OPCODE(scan))
      {
      case BOL:
         if( ep->reginput != ep->regbol)
            return(0);
         break;

      case EOL:
         if( *ep->reginput != '\0')
            return(0);
         break;

      case ANY:
         c = *ep->reginput;
         if( ! c)
            return( 0);

         ep->reginput++;
         break;

      case EXACTLY:
      {
         size_t          len;
         mulle_utf32_t   *opnd = OPERAND(scan);

         /* Inline the first character, for speed. */
         if( *opnd != *ep->reginput)
            return( 0);

         len = mulle_utf32_strlen( opnd);

         if((len > 1) && (mulle_utf32_strncmp( opnd, ep->reginput, len) != 0))
            return( 0);

         ep->reginput += len;
         break;
      }

      case ANYOF:
         c = *ep->reginput;
         if( ! c)
            return( 0);
         if( ! mulle_utf32_strchr( OPERAND(scan), c))
            return( 0);
         ep->reginput++;
         break;

      case ANYBUT:
         c = *ep->reginput;
         if( mulle_utf32_strchr( OPERAND(scan), c))
            return( 0);
         ep->reginput++;
         break;

      case NOTHING:
         break;

      case BACK:
         break;

      case OPEN + 1:
      case OPEN + 2:
      case OPEN + 3:
      case OPEN + 4:
      case OPEN + 5:
      case OPEN + 6:
      case OPEN + 7:
      case OPEN + 8:
      case OPEN + 9:
      additional_open:
      {
         int       no     = OPCODE(scan) - OPEN;
         mulle_utf32_t   *input = ep->reginput;

         assert( no >= 1 && no < MULLE_UTF32REGEX_NSUBEXP);
         if( regex_match(ep, next))
         {
            /*
             * Don't set startp if some later
             * invocation of the same parentheses
             * already has.
             */
            if( ! ep->regstartp[no])
               ep->regstartp[no] = input;

            return(1);
         }
         return(0);
      }

      case CLOSE - 1:
      case CLOSE - 2:
      case CLOSE - 3:
      case CLOSE - 4:
      case CLOSE - 5:
      case CLOSE - 6:
      case CLOSE - 7:
      case CLOSE - 8:
      case CLOSE - 9:
      additional_close :
      {
         int       no     = CLOSE - OPCODE(scan);
         mulle_utf32_t   *input = ep->reginput;

         if( regex_match(ep, next))
         {
            /*
             * Don't set endp if some later
             * invocation of the same parentheses
             * already has.
             */
            if( ep->regendp[no] == NULL)
               ep->regendp[no] = input;

            return(1);
         }
         return(0);
      }

      case BRANCH:
      {
         mulle_utf32_t   *save = ep->reginput;

         if( OPCODE(next) != BRANCH)    /* No choice. */
            next = _NEXT(scan);  /* Avoid recursion. */
         else
         {
            while( OPCODE(scan) == BRANCH)
            {
               if( regex_match(ep, _NEXT(scan)))
                  return(1);

               ep->reginput = save;
               scan         = regex_next_node(scan);
            }

            return( 0);
         }

         break;
      }

      case STAR:
      case PLUS:
      {
         mulle_utf32_t   nextch = (OPCODE(next) == EXACTLY)
                                  ? *OPERAND(next)
                                  : '\0';
         size_t          no;
         mulle_utf32_t   *save = ep->reginput;
         size_t          min   = (OPCODE(scan) == STAR) ? 0 : 1;

         for( no = regex_repeat(ep, _NEXT(scan)) + 1; no > min; no--)
         {
            ep->reginput = save + no - 1;

            /* If it could work, try it. */
            if((nextch == '\0') || (*ep->reginput == nextch))
               if( regex_match(ep, next))
                  return(1);

         }

         return(0);
      }

      case END:
         return(1);  /* Success! */

      default:
         if( OPCODE(scan) > OPEN + 9 && OPCODE( scan) < OPEN + MULLE_UTF32REGEX_NSUBEXP)
            goto additional_open;
         if( OPCODE( scan) < CLOSE - 9 && OPCODE( scan) >= CLOSE - MULLE_UTF32REGEX_NSUBEXP)
            goto additional_close;

         errno = EFAULT;
         return( -1);
      }
   }

   /*
    * We get here only if there's trouble -- normally "case END" is
    * the terminating point.
    */
   errno = EFAULT;
   return( -1);
}


/*
 *  - regex_repeat - report how many times something simple would match
 */
static size_t   regex_repeat( struct exec *ep, node *p)
{
   size_t    count;
   mulle_utf32_t   *scan;
   mulle_utf32_t   ch;

   switch( OPCODE( p))
   {
   case ANY:
      return( mulle_utf32_strlen( ep->reginput));

      break;

   case EXACTLY:
      ch    = *OPERAND(p);
      count = 0;

      for( scan = ep->reginput; *scan == ch; scan++)
         count++;

      return(count);

      break;

   case ANYOF:
      return( mulle_utf32_strspn( ep->reginput, OPERAND(p)));

      break;

   case ANYBUT:
      return( mulle_utf32_strcspn( ep->reginput, OPERAND(p)));

      break;

   default:    /* Oh dear.  Called inappropriately. */
#if DEBUG
      abort();  /* terrible internal corruption */
#endif
      fprintf( stderr, "internal corruption detected");
      return( 0);  /* Best compromise. */

      break;
   }

   /* NOTREACHED */
}


/*
 *  - regex_next_node - dig the "next" pointer out of a node
 */
static node   *regex_next_node( node *p)
{
   unsigned int   offset;

   offset = NEXT_NODE_OFFSET( p);
   if( offset == 0)
      return( NULL);

   return( (node *) ((OPCODE(p) == BACK) ? (char *) p - offset : (char *) p + offset));
}


//#ifdef DEBUG

static char   *regex_string_from_opcode( node *s, char buf[ 64]);

/*
 *  - regex_dump - dump a regexp onto stdout in vaguely comprehensible form
 */
void   mulle_utf32regex_dump( regexp *r)
{
   node              *p;
   node              *next;
   mulle_utf32_t     *s;
   int               op = EXACTLY; /* Arbitrary non-END op. */
   auto char         buf[ 64];

   if( ! r)
   {
      fprintf( stderr, "NULL\n");
      return;
   }

   p = (node *) ((char *) r->program + sizeof( mulle_utf32_t));

   while( op != END)    /* While that wasn't END last time... */
   {
      op = OPCODE( p);
         fprintf( stderr, "%2td%s", (char *) p - r->program, regex_string_from_opcode( p, buf)); /* Where, what. */
      next = regex_next_node( p);

      if( ! next)    /* Next ptr. */
         fprintf( stderr, "(0)");
      else
         fprintf( stderr, "(%td)", ((char *) p - r->program) + ((char *) next - (char *) p));

      p = _NEXT( p);

      if( op == ANYOF || op == ANYBUT || op == EXACTLY)
      {
         s = (mulle_utf32_t *) p;

         /* Literal string, where present. */
         while( *s != '\0')
         {
            fputc( isprint( *s) ? *s : '.', stderr);
            s++;
         }

         s++;
         p = (void *) s;
      }

      fputc('\n', stderr);
   }

   /* Header fields of interest. */
   if( r->regstart != '\0')
      fprintf( stderr, "start `%c' ", isprint( r->regstart) ? r->regstart : '.');

   if( r->reganch)
      fprintf( stderr, "anchored ");

   if( r->regmust != NULL)
   {
      mulle_utf32_t *s1;

      fprintf( stderr, "must have \"");
      for( s1 = r->regmust; *s1; s1++)
         putc( isprint( *s1) ? *s1 : '.', stderr);
      fprintf( stderr, "\"");
   }

   fprintf( stderr, "\n");
}


/*
 *  - regex_string_from_opcode - printable representation of opcode
 */
static char    *regex_string_from_opcode( node *op, char buf[64])
{
   char   *s;
   int    opcode;

   strcpy( buf, ":");

   opcode = OPCODE(op);
   switch( opcode)
   {
   case BOL     : s = "BOL"; break;
   case EOL     : s = "EOL"; break;
   case ANY     : s = "ANY"; break;
   case ANYOF   : s = "ANYOF"; break;
   case ANYBUT  : s = "ANYBUT"; break;
   case BRANCH  : s = "BRANCH"; break;
   case EXACTLY : s = "EXACTLY"; break;
   case NOTHING : s = "NOTHING"; break;
   case BACK    : s = "BACK"; break;
   case END     : s = "END"; break;
   case STAR    : s = "STAR"; break;
   case PLUS    : s = "PLUS"; break;

   default:
      if( opcode > OPEN && opcode <= OPEN + MULLE_UTF32REGEX_NSUBEXP)
      {
         sprintf(buf + strlen(buf), "OPEN%d", OPCODE(op) - OPEN);
         return( buf);
      }

      if( opcode < CLOSE && opcode >= CLOSE - MULLE_UTF32REGEX_NSUBEXP)
      {
         sprintf( buf + strlen(buf), "CLOSE%d", CLOSE - OPCODE(op));
         return( buf);
      }

      s = "***corrupted opcode***";
#ifdef TEST
      abort();
#endif
   }

   strcat( buf, s);
   return( buf);
}


//#endif /* ifdef DEBUG */

