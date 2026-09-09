# mulle-data Library Documentation for AI
<!-- Keywords: blobs, ranges, hashing, xxhash, fnv1a -->

## 1. Introduction & Purpose

**mulle-data** is a foundational C library that provides essential data structures and hash
functions for building high-performance hashtables and data manipulation. It solves three core
problems:

1. **Unified data representation** - The `struct mulle_data` combines a pointer and length,
avoiding separate parameter passing and enabling safer memory operations.
2. **Efficient hashing** - Multiple hash algorithms optimized for different use cases: bit
avalanche (MurmurHash3 finalizer) for integers/pointers, FNV-1a for small strings (especially
Objective-C selectors), and xxHash for large data blocks. Width-independent `mulle_hash_32` /
`mulle_hash_64` variants allow hashes to be shared across platforms of different pointer widths.
3. **Range operations** - The `struct mulle_range` represents ranges with location and length,
supporting complex range arithmetic, intersections, unions, and searches.

This library is a foundational component of **mulle-core**, serving as a low-level dependency
for container libraries and hashtable implementations. It depends only on **mulle-c11** for
cross-platform C compiler compatibility.

## 2. Key Concepts & Design Philosophy

**mulle-data** follows these design principles:

- **Composability**: Small, focused data structures (`mulle_data`, `mulle_range`) that work
together seamlessly.
- **Platform independence**: Functions adapt to 32-bit or 64-bit platforms automatically using
`uintptr_t`.
- **Performance first**: Inline functions for hot paths, specialized hash functions for
different data types, and optimized range operations.
- **Explicit validation**: Ranges can be created in invalid states and filtered later with
`mulle_range_is_valid`, separating creation from validation logic.
- **Zero-cost abstractions**: Most operations are static inline functions with no runtime
overhead.
- **Consistent naming**: Uses mulle-sde conventions with prefixes (`mulle_`, `_mulle_`)
indicating public/private API scope.

**Core architectural patterns:**
- **Dual implementations**: Most hash functions provide both 32-bit and 64-bit versions,
selected via static inline dispatchers based on `sizeof(uintptr_t)`.
- **Chained hashing**: Hash functions support incremental hashing via state pointers, enabling
streaming hash computation.
- **Range algebra**: Complete set of operations (union, intersection, subtraction, insertion)
for complex range manipulations.
- **No allocator dependency**: All structures are stack-allocated or embedded; no heap
management within the library.

## 3. Core API & Data Structures

### 3.1. `mulle-data.h`

#### `struct mulle_data`
- **Purpose:** Combines a pointer to memory and its length in a single structure for convenient
passing.
- **Key Fields:**
  - `void *bytes` - Pointer to the data
  - `size_t length` - Length of the data in bytes

#### **Lifecycle Functions:**
- `mulle_data_make(void *bytes, size_t length)` - Creates a mulle_data from bytes and length
- `mulle_data_make_empty(void)` - Creates empty mulle_data (points to "", length 0)
- `mulle_data_make_invalid(void)` - Creates invalid mulle_data (NULL bytes, length 0)

#### **Validation Functions:**
- `mulle_data_is_empty(struct mulle_data data)` - Returns 1 if length is 0
- `mulle_data_is_invalid(struct mulle_data data)` - Returns 1 if bytes is NULL
- `mulle_data_assert(struct mulle_data data)` - Asserts that non-zero length implies non-NULL
bytes

#### **Operations:**
- `mulle_data_hash(struct mulle_data data)` - Computes hash using xxHash algorithm
- `mulle_data_hash_chained(struct mulle_data data, void **state_p)` - Computes chained hash for
streaming; finalise by passing `mulle_data_make_invalid()` (bytes == NULL) to retrieve the hash
- `mulle_data_subdata(struct mulle_data data, struct mulle_range range)` - Extracts subdata
using range
- `mulle_data_search_data(struct mulle_data haystack, struct mulle_data needle)` - Searches for
needle in haystack, returns pointer or NULL
- `mulle_data_equals(struct mulle_data a, struct mulle_data b)` - Returns 1 if both have same
length and identical bytes; two invalid data values are equal, invalid never equals valid
- `mulle_data_compare(struct mulle_data a, struct mulle_data b)` - Lexicographic comparison;
returns < 0, 0, or > 0. Invalid data compares less than any valid data

#### **Version Functions:**
- `mulle_data_get_version_major(void)` - Returns major version number
- `mulle_data_get_version_minor(void)` - Returns minor version number
- `mulle_data_get_version_patch(void)` - Returns patch version number
- `mulle_data_get_version(void)` - Returns full version as uint32_t

### 3.2. `mulle-range.h`

#### `struct mulle_range`
- **Purpose:** Represents a range with a starting location and length, used for indexing and
slicing operations.
- **Key Fields:**
  - `uintptr_t location` - Starting position (must be ≤ `mulle_range_location_max`)
  - `uintptr_t length` - Length of the range

#### **Constants:**
- `mulle_not_found_e` - Value indicating "not found" (`INTPTR_MAX`)
- `mulle_range_location_min` - Minimum valid location (0)
- `mulle_range_location_max` - Maximum valid location (`mulle_not_found_e - 1`)
- `mulle_range_zero` - Empty range `{0, 0}`
- **Note:** `mulle_not_found_e` is an *out-of-band* sentinel. Valid range locations are capped at
  `mulle_range_location_max == mulle_not_found_e - 1`, so a valid non-empty range can never
  produce it from `get_first_location`/`get_last_location`. Do not widen the range ceiling.

#### **Creation Functions:**
- `mulle_range_make(uintptr_t location, uintptr_t length)` - Creates range from location and
length
- `mulle_range_make_locations(uintptr_t location, uintptr_t location2)` - Creates range from two
 locations (inclusive)
- `mulle_range_make_all(void)` - Creates range covering all possible locations
- `mulle_range_make_invalid(void)` - Creates invalid range (location = `mulle_not_found_e`)

#### **Validation:**
- `mulle_range_is_valid(struct mulle_range range)` - Checks if range is valid (location ≤ max,
no overflow)
- `mulle_range_equals(struct mulle_range range, struct mulle_range other)` - Checks equality
- `mulle_range_validate_against_length(struct mulle_range range, uintptr_t length)` - Validates
range against container length

#### **Accessor Functions:**
- `mulle_range_get_min(struct mulle_range range)` - Returns starting location
- `mulle_range_get_max(struct mulle_range range)` - Returns end position (location + length)
- `mulle_range_get_first_location(struct mulle_range range)` - Returns first location or
`mulle_not_found_e`
- `mulle_range_get_last_location(struct mulle_range range)` - Returns last location or
`mulle_not_found_e`
- `_mulle_range_get_min()`, `_mulle_range_get_max()`, etc. - Unchecked versions without
validation

#### **Containment Tests:**
- `mulle_range_contains_location(struct mulle_range range, uintptr_t location)` - Tests if
location is within range
- `mulle_range_contains(struct mulle_range big, struct mulle_range small)` - Tests if one range
contains another
- `mulle_range_intersects(struct mulle_range range, struct mulle_range other)` - Tests if ranges
 intersect

#### **Comparison Functions:**
- `mulle_range_less_than_location(struct mulle_range range, uintptr_t location)` - Tests if
range is before location
- `mulle_range_less_than_or_equal_to_location(...)` - Tests if range is before or at location
- `mulle_range_greater_than_location(...)` - Tests if range is after location
- `mulle_range_greater_than_or_equal_to_location(...)` - Tests if range is after or at location
- `mulle_range_equal_to_location(...)` - Tests if range starts at location
- `mulle_range_distance_to_location(...)` - Returns absolute distance to location

#### **Set Operations:**
- `mulle_range_intersection(struct mulle_range range, struct mulle_range other)` - Returns
intersection; empty (`length == 0`) but still valid range if inputs do not overlap
- `mulle_range_union(struct mulle_range range, struct mulle_range other)` - Returns union of
ranges
- `mulle_range_subtract(struct mulle_range a, struct mulle_range b, struct mulle_range
result[2])` - Subtracts b from a, returns up to 2 result ranges
- `mulle_range_subtract_location(struct mulle_range a, uintptr_t location, struct mulle_range
result[2])` - Removes single location from range
- `mulle_range_insert(struct mulle_range a, struct mulle_range b, struct mulle_range result[2])`
  - b must be adjacent to or intersect a (else returns 0). Inserts range b into a, returns 2 result
  ranges: `result[0]` is the prefix (before the insertion) and `result[1]` is the suffix (shifted
  by `b.length`)

#### **Binary Search:**
- `_mulle_range_hole_bsearch(struct mulle_range *buf, uintptr_t n, uintptr_t search_location)` -
Finds insertion point (use after verifying not contained)
- `mulle_range_contains_bsearch(struct mulle_range *buf, uintptr_t n, struct mulle_range
search)` - Searches for containing range
- `mulle_range_intersects_bsearch(struct mulle_range *buf, uintptr_t n, struct mulle_range
search)` - Searches for intersecting range

#### **Iteration Macro:**
- `mulle_range_for(range, name)` - For-loop macro to iterate over all locations in range.
  Snapshotting the range argument into a temporary variable first, so evaluating the macro
  argument only happens once

### 3.3. `mulle-hash.h`

#### **Avalanche Functions** (Bit mixing based on MurmurHash3 finalizer)
- `mulle_hash_avalanche32(uint32_t h)` - Avalanches 32-bit hash (MurmurHash3 finalizer)
- `mulle_hash_avalanche64(uint64_t h)` - Avalanches 64-bit hash (MurmurHash3 finalizer)
- `mulle_hash_avalanche(uintptr_t h)` - Platform-adaptive, selects between 32/64-bit based on
pointer width

#### **Integer/Pointer Hashing:**
- `mulle_integer_hash(uintptr_t p)` - Hashes an integer value via avalanche
- `mulle_pointer_hash(const void *p)` - Hashes a pointer value via avalanche (input
const-qualified)
- `mulle_float_hash(float f)` - Hashes a float; canonicalises -0.0 to +0.0 so equal values hash
identically, then type-puns to uint32_t and avalanches
- `mulle_double_hash(double f)` - Hashes the full 64-bit representation (no precision loss);
canonicalises -0.0; on 32-bit platforms folds the two 32-bit halves via XOR before avalanching
- `mulle_long_double_hash(long double ld)` - Converts to double first (saturates to infinity
outside double range), then uses `mulle_double_hash`
- `mulle_long_long_hash(long long value)` - Hashes 64-bit integer; on 64-bit uses
`mulle_hash_avalanche64`, on 32-bit folds halves so it agrees in width with `mulle_integer_hash`

#### **Byte-array Hashing (xxHash-based):**
- `_mulle_hash_32(const void *bytes, size_t length)` - 32-bit xxHash (XXH32) of byte array
- `_mulle_hash_64(const void *bytes, size_t length)` - 64-bit xxHash (XXH64) of byte array
- `mulle_hash_32(const void *bytes, size_t length)` - Width-independent 32-bit hash; safe for
persistence / wire protocols / cross-pointer-width sharing. NULL bytes treated as "" (empty)
- `mulle_hash_64(const void *bytes, size_t length)` - Width-independent 64-bit hash; same
guarantees as `mulle_hash_32`
- `_mulle_hash(const void *bytes, size_t length)` - Platform-adaptive hash (unchecked)
- `mulle_hash(const void *bytes, size_t length)` - Platform-adaptive hash (NULL-safe).
  **Pointer-width dependent**: different values on 32-bit vs 64-bit platforms; do NOT persist or
  share across processes of different pointer widths. `mulle_hash(NULL, 0)` equals
  `mulle_hash("", 0)` (no in-band sentinel)

#### **Chained Hashing (Streaming, xxHash-based):**
- `mulle_hash_chained_32(const void *bytes, size_t length, void **state_p)` - 32-bit chained
- `mulle_hash_chained_64(const void *bytes, size_t length, void **state_p)` - 64-bit chained
- `mulle_hash_chained(const void *bytes, size_t length, void **state_p)` - Platform-adaptive
chained hash

**Chained hash usage pattern (legacy):**
```c
void     *state = NULL;  // Must initialize to NULL
uintptr_t hash;

(void) mulle_hash_chained( chunk1, len1, &state);
(void) mulle_hash_chained( chunk2, len2, &state);
hash = mulle_hash_chained( NULL, 0, &state);  // Get final hash
// Reset state = NULL before next chain
```

#### **Type-safe Chained Hash Wrappers** (recommended over the in-band NULL sentinel):
- `mulle_hash_chained_32_add(const void *bytes, size_t length, void **state_p)` - Feed bytes
  (returns void; nothing to misinterpret)
- `mulle_hash_chained_32_final(void **state_p)` - Retrieve final 32-bit hash; resets state to NULL
- `mulle_hash_chained_64_add(const void *bytes, size_t length, void **state_p)` - Feed bytes
- `mulle_hash_chained_64_final(void **state_p)` - Retrieve final 64-bit hash; resets state to NULL
- `mulle_hash_chained_add(const void *bytes, size_t length, void **state_p)` - Feed bytes
  (pointer-width)
- `mulle_hash_chained_final(void **state_p)` - Retrieve final pointer-width hash; resets state to
  NULL
- `mulle_hash_chained_32_done(void **state_p)` / `mulle_hash_chained_64_done(void **state_p)` /
  `mulle_hash_chained_done(void **state_p)` - Abandon an in-progress chain, discarding the
  result and resetting state to NULL (safe with NULL state_p / already-NULL state)

### 3.4. `mulle-fnv1a.h`

#### **FNV-1a Constants:**
- `MULLE_FNV1A_32_INIT` - 32-bit FNV-1a initial value (0x811c9dc5)
- `MULLE_FNV1A_64_INIT` - 64-bit FNV-1a initial value
- `MULLE_FNV1A_32_PRIME` - 32-bit FNV prime (0x01000193)
- `MULLE_FNV1A_64_PRIME` - 64-bit FNV prime

#### **Initialization:**
- `_mulle_fnv1a_init_32(void)` - Returns 32-bit FNV-1a initial value
- `_mulle_fnv1a_init_64(void)` - Returns 64-bit FNV-1a initial value
- `_mulle_fnv1a_init(void)` - Platform-adaptive initialization

#### **Single-Step Hashing:**
- `_mulle_fnv1a_step_32(uint32_t hash, unsigned char value)` - Updates 32-bit hash with one byte
- `_mulle_fnv1a_step_64(uint64_t hash, unsigned char value)` - Updates 64-bit hash with one byte
- `_mulle_fnv1a_step(uintptr_t hash, unsigned char value)` - Platform-adaptive step

#### **Buffer Hashing:**
- `_mulle_fnv1a_32(const void *buf, size_t len)` - 32-bit FNV-1a hash of buffer
- `_mulle_fnv1a_64(const void *buf, size_t len)` - 64-bit FNV-1a hash of buffer
- `_mulle_fnv1a(const void *buf, size_t len)` - Platform-adaptive FNV-1a hash

#### **Chained Hashing:**
- `_mulle_fnv1a_chained_32(const void *buf, size_t len, uint32_t hash)` - 32-bit chained FNV-1a
- `_mulle_fnv1a_chained_64(const void *buf, size_t len, uint64_t hash)` - 64-bit chained FNV-1a
- `_mulle_fnv1a_chained(const void *buf, size_t len, uintptr_t hash)` - Platform-adaptive chained
  FNV-1a (takes initial hash, returns updated hash)

#### **Inline Versions:**
- `_mulle_fnv1a_32_inline(const void *buf, size_t len)` - Inline 32-bit FNV-1a (for hot paths)
- `_mulle_fnv1a_64_inline(const void *buf, size_t len)` - Inline 64-bit FNV-1a
- `_mulle_fnv1a_inline(const void *buf, size_t len)` - Platform-adaptive inline
- `_mulle_fnv1a_chained_32_inline(const void *buf, size_t len, uint32_t hash)` /
  `_mulle_fnv1a_chained_64_inline(const void *buf, size_t len, uint64_t hash)` /
  `_mulle_fnv1a_chained_inline(const void *buf, size_t len, uintptr_t hash)` - Inline chained
  versions

#### **String Hashing:**
- `_mulle_string_hash_32(const char *s)` - 32-bit FNV-1a of null-terminated string
- `_mulle_string_hash_64(const char *s)` - 64-bit FNV-1a of null-terminated string
- `_mulle_string_hash(const char *s)` - Platform-adaptive string hash
- `_mulle_string_hash_chained_32(const char *s, uint32_t hash)` - Chained string hash (32-bit)
- `_mulle_string_hash_chained_64(const char *s, uint64_t hash)` - Chained string hash (64-bit)
- `_mulle_string_hash_chained(const char *s, uintptr_t hash)` - Platform-adaptive chained string
  hash

### 3.5. `mulle-prime.h`

#### **Prime Number Functions:**
- `mulle_prime_for_depth(int depth)` - Returns prime number for given depth (-32 to 32)
- `mulle_prime_hash_for_depth(uintptr_t value, int depth)` - Computes hash using modulo with
depth-specific prime

**Purpose:** Provides pre-computed prime numbers for hashtable sizing and fast modulo-based
hashing. The depth parameter allows scaling hashtable size up or down by powers of ~2.

### 3.6. `mulle-qsort.h`

#### **Type Definitions:**
- `mulle_qsort_r_cmp_t` - Comparison function with thunk: `int cmp(void *a, void *b, void
*thunk)`
- `mulle_qsort_cmp_t` - Simple comparison function: `int cmp(void *a, void *b)`

#### **Sort Functions:**
- `mulle_qsort_r(void *a, size_t n, size_t es, mulle_qsort_r_cmp_t *cmp, void *thunk)` -
Quicksort with user data
- `mulle_qsort(void *a, size_t n, size_t es, mulle_qsort_cmp_t *cmp)` - Standard quicksort
- `_mulle_qsort_r_inline(...)` - Inline version of mulle_qsort_r

**Purpose:** Cross-platform quicksort implementation based on BSD's algorithm, ensuring
consistent behavior across platforms. Provides both simple and reentrant (with thunk) versions.
Note the comparators intentionally take `void *` (not `const void *`); changing would break
existing downstream comparator function pointers. The internal swap is a no-op for element size 0.

## 4. Performance Characteristics

### Hash Functions:
- **FNV-1a**: O(n) where n is data length. Fast for small strings (<100 bytes), especially
suited for Objective-C selectors and short identifiers. Produces good distribution with minimal
collisions for strings.
- **xxHash** (via mulle_hash / `_mulle_hash_32/64`): O(n) where n is data length. Optimized for
larger data blocks (>100 bytes). Uses platform-specific optimizations and produces high-quality
32/64-bit hashes.
- **Avalanche functions**: O(1). Ultra-fast bit mixing for integers and pointers. Provides good
distribution for hashtable indexing.
- **Chained hashing**: O(total bytes) across all chunks. Minimal overhead for streaming hash
computation.

### Range Operations:
- **Creation, validation, containment tests**: O(1)
- **Intersection, union**: O(1)
- **Subtraction, insertion**: O(1) - produces up to 2 result ranges
- **Binary search functions**: O(log n) where n is number of ranges in array

### mulle_data:
- **Creation, validation**: O(1)
- **Hashing**: Depends on hash function (see above)
- **Search**: `O(n*m)` worst case where n is haystack length, m is needle length. Uses naive
search algorithm.

### Sorting:
- **mulle_qsort**: O(n log n) average case, O(n²) worst case (standard quicksort
characteristics). Switches to insertion sort for small partitions (<7 elements) for better
performance.

### Thread Safety:
- **All functions are thread-safe** if called on different data.
- **Not thread-safe** for concurrent access to the same data structure.
- **No internal state** except for chained hash state pointers managed by caller.
- **No locks or atomics** - zero synchronization overhead.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Always validate ranges before use in production code**: Use `mulle_range_is_valid()` to
check ranges, but creating invalid ranges temporarily is acceptable for filtering.

2. **Use the correct hash function for your data**:
   - Small strings/selectors → FNV-1a (`_mulle_fnv1a` or `_mulle_string_hash`)
   - Large data blocks → xxHash (`mulle_hash`)
   - Integers/pointers → Avalanche (`mulle_integer_hash`, `mulle_pointer_hash`)
   - Persistent / cross-platform (wire, disk, IPC) → width-independent `mulle_hash_32` /
     `mulle_hash_64` (NOT `mulle_hash`)

3. **Prefer static inline functions**: Most API functions are static inline, resulting in zero
function call overhead.

4. **Initialize chained hash state to NULL**: Always set `void *state = NULL;` before first
chained hash call, and reset to NULL before starting a new chain. Prefer the type-safe
`mulle_hash_chained_add` / `mulle_hash_chained_final` wrappers over the NULL-sentinel style.

5. **Use mulle_data for passing byte arrays**: Avoids separate pointer and length parameters,
reducing error potential.

6. **Range subtraction produces two ranges**: Always allocate `result[2]` for subtraction
operations; both ranges may have zero length.

### Common Pitfalls:

1. **Do not access internal `_` prefixed functions unless documented**: Functions starting with
`_` (e.g., `_mulle_hash`, `_mulle_range_get_min`) are implementation details but some are
documented for specific use cases.

2. **mulle_data with NULL bytes and non-zero length is invalid**: Always validate with
`mulle_data_assert()` in debug builds.

3. **Range overflow is not checked in mulle_range_make**: The function allows creating invalid
ranges. Use `mulle_range_validate_against_length()` when working with container bounds.

4. **FNV-1a 32-bit and 64-bit hashes are incompatible**: Do not mix them; choose one based on
platform or use the adaptive `_mulle_fnv1a()`.

5. **Chained hash state is opaque**: Do not inspect or modify the state pointer; treat it as a
black box.

6. **mulle_data_search_data returns pointer into haystack**: The returned pointer is not
allocated; do not free it.

### Idiomatic Usage:

- **Declare variables C89 style**: All variables at function start, no inline initialization
unless from arguments or aggregates.
- **Use 3-space indentation**: Follow project conventions (Allman braces, columnar alignment).
- **Prefer checked API over unchecked**: Use `mulle_range_get_min()` over
`_mulle_range_get_min()` unless performance-critical and validity is guaranteed.
- **Use mulle_range_for macro for iteration**: Clean, readable range iteration without manual
sentinel calculation.

## 6. Integration Examples

### Example 1: Creating and Hashing Data

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   struct mulle_data   data;
   uintptr_t           hash;
   char                *text = "VfL Bochum 1848";

   data = mulle_data_make( text, 15);
   mulle_data_assert( data);

   hash = mulle_data_hash( data);
   printf( "Hash: %#tx\n", hash);

   return( 0);
}
```

### Example 2: Chained Hashing for Streaming Data

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   void        *state;
   uintptr_t   hash;

   state = NULL;

   mulle_hash_chained( "VfL ", 4, &state);
   mulle_hash_chained( "Bochum ", 7, &state);
   mulle_hash_chained( "1848", 4, &state);
   hash = mulle_hash_chained( NULL, 0, &state);

   printf( "Chained hash: %#tx\n", hash);

   return( 0);
}
```

### Example 3: Range Operations - Intersection and Union

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   struct mulle_range   a;
   struct mulle_range   b;
   struct mulle_range   intersection;
   struct mulle_range   uni;

   a            = mulle_range_make( 10, 20);
   b            = mulle_range_make( 15, 25);
   intersection = mulle_range_intersection( a, b);
   uni          = mulle_range_union( a, b);

   printf( "Range A: [%tu, %tu)\n", a.location, mulle_range_get_max( a));
   printf( "Range B: [%tu, %tu)\n", b.location, mulle_range_get_max( b));
   printf( "Intersection: [%tu, %tu) length=%tu\n",
           intersection.location,
           mulle_range_get_max( intersection),
           intersection.length);
   printf( "Union: [%tu, %tu) length=%tu\n",
           uni.location,
           mulle_range_get_max( uni),
           uni.length);

   return( 0);
}
```

### Example 4: Range Subtraction (Punching Holes)

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   struct mulle_range   a;
   struct mulle_range   b;
   struct mulle_range   result[ 2];

   a = mulle_range_make( 0, 100);
   b = mulle_range_make( 30, 20);

   mulle_range_subtract( a, b, result);

   printf( "Original: [%tu-%tu]\n", a.location, mulle_range_get_last_location( a));
   printf( "Subtract: [%tu-%tu]\n", b.location, mulle_range_get_last_location( b));
   printf( "Result[0]: [%tu-%tu] length=%tu\n",
           result[ 0].location,
           result[ 0].length ? mulle_range_get_last_location( result[ 0]) : 0,
           result[ 0].length);
   printf( "Result[1]: [%tu-%tu] length=%tu\n",
           result[ 1].location,
           result[ 1].length ? mulle_range_get_last_location( result[ 1]) : 0,
           result[ 1].length);

   return( 0);
}
```

### Example 5: Using FNV-1a for String Hashing

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   uintptr_t   hash1;
   uintptr_t   hash2;

   hash1 = _mulle_string_hash( "selector:");
   hash2 = _mulle_fnv1a( "data", 4);

   printf( "String hash: %#tx\n", hash1);
   printf( "Buffer hash: %#tx\n", hash2);

   return( 0);
}
```

### Example 6: Searching Within mulle_data

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>
#include <string.h>

int   main( void)
{
   struct mulle_data   haystack;
   struct mulle_data   needle;
   char                *text;
   char                *found;

   text     = "The quick brown fox";
   haystack = mulle_data_make( text, strlen( text));
   needle   = mulle_data_make( "brown", 5);

   found = mulle_data_search_data( haystack, needle);

   if( found)
      printf( "Found at offset: %td\n", found - text);
   else
      printf( "Not found\n");

   return( 0);
}
```

### Example 7: Iterating Over a Range

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   struct mulle_range   range;

   range = mulle_range_make( 10, 5);

   mulle_range_for( range, i)
   {
      printf( "Location: %tu\n", i);
   }

   return( 0);
}
```

### Example 8: Type-Safe Chained Hashing (add/final)

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   void        *state;
   uint64_t    hash;

   // initialize state to NULL before starting a new chain
   state = NULL;

   // _add feeds data, _final retrieves the hash and resets state
   mulle_hash_chained_64_add( "VfL ", 4, &state);
   mulle_hash_chained_64_add( "Bochum ", 7, &state);
   hash = mulle_hash_chained_64_final( &state);

   printf( "Chained hash (add/final): %#llx\n",
           (unsigned long long) hash);

   return( 0);
}
```

### Example 9: Width-Independent Hashing

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   uint32_t   hash32;
   uint64_t   hash64;

   // stable on 32-bit and 64-bit platforms: safe for persistence / wire
   hash32 = mulle_hash_32( "VfL Bochum 1848", 15);
   hash64 = mulle_hash_64( "VfL Bochum 1848", 15);

   printf( "hash32: 0x%08x\n", (unsigned int) hash32);
   printf( "hash64: 0x%016llx\n", (unsigned long long) hash64);

   return( 0);
}
```

### Example 10: Comparing and Testing Equality of mulle_data

```c
#include <mulle-data/mulle-data.h>
#include <stdio.h>

int   main( void)
{
   struct mulle_data   a;
   struct mulle_data   b;
   struct mulle_data   c;

   a = mulle_data_make( "abc", 3);
   b = mulle_data_make( "abc", 3);
   c = mulle_data_make( "abd", 3);

   printf( "a == b: %d\n", mulle_data_equals( a, b));
   printf( "a == c: %d\n", mulle_data_equals( a, c));
   printf( "compare( a, c) < 0: %d\n", mulle_data_compare( a, c) < 0);

   return( 0);
}
```

## 7. Dependencies

This library has minimal dependencies:

- **mulle-c11** - Cross-platform C compiler glue and preprocessor conveniences (no-link,
header-only dependency)
- **xxhash.h** - Bundled xxHash implementation (included in `src/xxhash/`), the actual hashing
engine behind `mulle_hash` / `mulle_hash_32` / `mulle_hash_64` and the chained hash APIs

mulle-data is designed to be a foundational library with no external runtime dependencies beyond
 the C standard library.
