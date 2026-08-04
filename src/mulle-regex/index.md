# mulle-regex Library Documentation for AI
<!-- Keywords: regex, unicode, utf32, pattern, substitute, compile -->
## 1. Introduction & Purpose

- A compact Unicode-aware regular-expression library (Henry Spencer's regexp reimplementation adapted for Unicode).
- Solves: pattern matching and substitution on UTF-32 strings (mulle_utf32_t) for C programs.
- Key features: egrep-style patterns, compile/execute API, substitution helpers, convenience one-shot functions.
- Relationship: component of mulle-core; depends on mulle-utf for Unicode types and helpers.

## 2. Key Concepts & Design Philosophy

- Uses nondeterministic finite automata (NFA) for regex implementation: fast to compile, potentially slower to execute than DFA-based engines.
- API separates compile/execution and substitution phases; substitution is not automatic — caller composes prefix/match/suffix.
- Minimal, C-idiomatic, and portable: opaque regex struct and explicit allocation/free semantics.

## 3. Core API & Data Structures

### 3.1. src/mulle-utf32regex.h

#### struct mulle_utf32regex
- Purpose: Opaque compiled regular-expression object for matching UTF-32 input.
- Lifecycle Functions:
  - mulle_utf32regex_compile(mulle_utf32_t *pattern): Compile a pattern; returns malloced struct or NULL on error.
  - mulle_utf32regex_free(struct mulle_utf32regex *regex): Free a compiled regex (inline wrapper around mulle_free).

- Core Operations:
  - mulle_utf32regex_execute(struct mulle_utf32regex *regex, mulle_utf32_t *src):
     - Returns <0 on error, 1 on match, 0 on no match.
  - mulle_utf32regex_substitute(struct mulle_utf32regex *regex,
                                mulle_utf32_t *replacement,
                                mulle_utf32_t *dst,
                                size_t dst_len,
                                int zero):
     - Performs substitution into caller-provided dst buffer; returns <0 on failure, 0 on success.
     - Requires dst buffer length; does not append trailing zero unless `zero` and dst_len include space for it.

- Convenience One-shot Functions:
  - mulle_utf32_match(mulle_utf32_t *pattern, mulle_utf32_t *src):
     - Compiles, executes, and returns a malloced match buffer or NULL.
  - mulle_utf32_substitute(mulle_utf32_t *pattern, mulle_utf32_t *replacement, mulle_utf32_t *src):
     - Compiles, substitutes and returns a malloced result buffer or NULL.

- Substitution sizing helpers:
  - mulle_utf32regex_substitution_length(struct mulle_utf32regex *regex, mulle_utf32_t *replacement):
     - Returns length (number of codepoints) of the substituted part (does not include trailing zero). Returns (size_t)-1 on error.
  - mulle_utf32regex_substitution_buffer_size(...):
     - Inline helper returning number of bytes to malloc for substitution (adds space for trailing zero).

- Match inspection:
  - mulle_utf32regex_range_for_index(struct mulle_utf32regex *regex, unsigned int i):
     - Returns struct mulle_range for the whole match (index 0) or capture groups (1-9 for \1..\9).
     - struct mulle_range exposes .location and .length (from mulle-utf).

### 3.2. src/mulle-regex.h and src/generic/include.h
- src/mulle-regex.h acts as umbrella include and re-exports mulle-utf32regex API.
- include.h defines MULLE__REGEX_GLOBAL macros for proper linkage and includes generated reflect headers.

## 4. Performance Characteristics

- Compilation: generally O(p) in pattern length; NFA compilation is typically fast and lightweight.
- Execution: NFA-based matching can be slower than DFA; worst-case behavior can be superlinear for pathological patterns (backtracking). Expect acceptable speed for typical use but avoid catastrophic patterns for very large inputs.
- Substitution: costs include match execution plus copying; caller-provided buffers avoid extra allocations.
- Thread-safety: library is not documented as thread-safe. Treat compiled regex and operations as not internally synchronized; manage concurrency externally.

## 5. AI Usage Recommendations & Patterns

- Best Practices:
  - Always compile with mulle_utf32regex_compile and free with mulle_utf32regex_free.
  - Use mulle_utf32regex_substitution_length to compute needed buffer size, then allocate (use buffer_size helper for bytes).
  - For quick tasks use convenience functions (mulle_utf32_match / mulle_utf32_substitute) but remember they return malloced buffers the caller must free.
  - Use mulle_utf32regex_range_for_index( regex, 0 ) to get the whole match; 1..9 for capture groups.

- Common Pitfalls:
  - mulle_utf32regex_substitute requires correct dst_len; if too small it returns an error — always size buffers properly.
  - Substitution helpers return lengths that exclude the trailing zero; add 1 when allocating space for the NUL.
  - Substitution does not include prefix/suffix of original string — caller must stitch prefix + replacement + suffix.
  - Do not access internal/underscore headers or private structs — stick to public headers.

- Idiomatic Usage:
  - Prefer explicit compile/execute when performing multiple matches with the same pattern (better performance than one-shot helpers).
  - Use mulle-utf helpers (mulle_range_* and mulle_utf32_strlen) when manipulating ranges and slices.

## 6. Integration Examples

### Example 1: Compile, match, and inspect ranges

```c
#include <mulle-regex/mulle-regex.h>
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

int
main()
{
   struct mulle_utf32regex  *r;
   mulle_utf32_t           text[]    = { 'a', 'b', 'c', 0 };
   mulle_utf32_t           pattern[] = { 'b', 0 };
   struct mulle_range      range;
   int                     rc;

   r = mulle_utf32regex_compile( pattern);
   if( ! r)
   {
      return( 1);
   }

   rc = mulle_utf32regex_execute( r, text);
   if( rc == 1)
   {
      range = mulle_utf32regex_range_for_index( r, 0);
      printf( "match at %zu length %zu\n", (size_t) range.location, (size_t) range.length);
   }

   mulle_utf32regex_free( r);
   return( 0);
}
```

### Example 2: Substitution with explicit buffer

```c
#include <mulle-regex/mulle-regex.h>
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

int
main()
{
   struct mulle_utf32regex  *r;
   mulle_utf32_t           src[] = { 'a', 'b', 'c', 0 };
   mulle_utf32_t           pat[] = { 'b', 0 };
   mulle_utf32_t           repl[] = { 'X', 0 };
   size_t                  dst_len;
   mulle_utf32_t           *dst;
   int                     rc;

   r = mulle_utf32regex_compile( pat);
   if( ! r)
      return( 1);

   dst_len = mulle_utf32regex_substitution_length( r, repl);
   if( dst_len == (size_t) -1)
   {
      mulle_utf32regex_free( r);
      return( 1);
   }

   dst = mulle_malloc( mulle_utf32regex_substitution_buffer_size( r, repl));
   rc  = mulle_utf32regex_substitute( r, repl, dst, dst_len + 1, 1);
   if( rc < 0)
   {
      mulle_free( dst);
      mulle_utf32regex_free( r);
      return( 1);
   }

   /* caller must stitch prefix + dst + suffix using ranges */

   mulle_free( dst);
   mulle_utf32regex_free( r);
   return( 0);
}
```

### Example 3: One-shot convenience substitute

```c
#include <mulle-regex/mulle-regex.h>
#include <mulle-utf/mulle-utf.h>

/* returns malloced result or NULL */
mulle_utf32_t *
example_one_shot()
{
   mulle_utf32_t   pattern[] = { 'b', 0 };
   mulle_utf32_t   replacement[] = { 'Z', 0 };
   mulle_utf32_t   src[] = { 'a', 'b', 'c', 0 };

   return( mulle_utf32_substitute( pattern, replacement, src));
}
```

## 7. Dependencies

- mulle-utf         (provides mulle_utf32_t, struct mulle_range, and string/range helpers)
- mulle-core        (this project is a component of mulle-core; typically used via mulle-core)

## 8. Shortcut

- Existing TOC last commit: 2026-04-04 (check repo history for diffs).  If updating, compare since that commit.

---
Notes for AI assistants:
- Use public headers (src/mulle-utf32regex.h) and test/ (test/unicode/simple.c) as authoritative examples.
- Prefer using mulle_utf32regex_substitution_length and buffer_size helpers to allocate substitution buffers correctly.
