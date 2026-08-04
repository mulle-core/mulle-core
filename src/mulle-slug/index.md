# mulle-slug Library Documentation for AI
<!-- Keywords: url-slugs, text-normalization -->

## 1. Introduction & Purpose

mulle-slug creates URL-safe slugs from arbitrary text, converting titles and identifiers into clean, human-readable URL components. It solves the problem of transforming unstructured text (with special characters, accents, spaces, and emojis) into standardized URL segments. The library provides intelligent Unicode handling including diacritic removal, character normalization, and multi-language support. This is a foundational utility in the mulle-c ecosystem for web applications requiring SEO-friendly URL generation.

## 2. Key Concepts & Design Philosophy

**Slug Concept:**
A slug is a simplified, URL-safe representation of text typically used in web URLs. For example, "A Beginner's Guide to Web Development" becomes "a-beginners-guide-to-web-development". Slugs are human-readable, search-engine friendly, and safe for URL transmission.

**Design Principles:**

- **Unicode-Aware:** Fully handles UTF-8/UTF-16/UTF-32 encoded text. Characters with diacritical marks (accents, umlauts) are intelligently normalized to their ASCII equivalents (e.g., "Café" → "Cafe").

- **Multi-Language Support:** Includes comprehensive character mapping tables for major languages including Latin, Greek, Cyrillic, and extended scripts. Diacritics are stripped preserving the base character.

- **Customizable Delimiters:** By default, spaces and separator characters are replaced with hyphens, but this is configurable.

- **Non-Destructive Approach:** Characters that don't normalize cleanly are typically removed (not mangled), preserving semantic meaning.

- **Performance:** Uses lookup tables for character mapping rather than regex or complex rules, enabling fast single-pass conversion.

- **Buffer-Friendly API:** Provides functions that write to buffers, enabling zero-copy operations or pre-allocated stack buffers, reducing allocation overhead.

## 3. Core API & Data Structures

### 3.1 `mulle-slug.h` - Public API

#### UTF-8 String Functions

**`mulle_utf8_slugify(char *s)`**

- **Purpose:** Slugify a UTF-8 C string, returning a newly allocated string.
- **Parameters:**
  - `s`: Input UTF-8 encoded C string (null-terminated).
- **Returns:** Pointer to a newly allocated slug string (must be freed with `mulle_free()`).
- **Behavior:**
  - Converts the input string to a URL-safe slug.
  - Diacritics are removed/normalized.
  - Spaces become hyphens by default.
  - Special characters are removed or normalized.
  - Preserves case from the original text.

#### UTF8Data Functions

**`mulle_utf8data_slugify(struct mulle_utf8data data, struct mulle_allocator *allocator)`**

- **Purpose:** Slugify UTF-8 data with explicit length and custom allocator.
- **Parameters:**
  - `data`: `mulle_utf8data` structure containing character pointer and length.
  - `allocator`: Custom memory allocator (pass `NULL` for default).
- **Returns:** A new `mulle_utf8data` structure with the slugified result.
- **Behavior:**
  - Works with non-null-terminated UTF-8 data.
  - Allows specifying length explicitly.
  - Respects custom allocator for memory operations.
  - Returns structure must have its characters freed with the same allocator.

#### Buffer Integration Functions

**`mulle_buffer_add_slugified_utf8data(struct mulle_buffer *buffer, struct mulle_utf8data data)`**

- **Purpose:** Append a slugified version of UTF-8 data to an existing buffer.
- **Parameters:**
  - `buffer`: Target buffer (must be initialized).
  - `data`: UTF-8 data to slugify and add.
- **Behavior:**
  - Applies slugification rules.
  - Appends the result to the buffer (automatic growth if needed).
  - Useful for building HTML or URL fragments incrementally.

**`mulle_buffer_slugify_utf8data(struct mulle_buffer *buffer, struct mulle_utf8data data)`**

- **Purpose:** Replace buffer contents with slugified UTF-8 data.
- **Parameters:**
  - `buffer`: Target buffer (must be initialized).
  - `data`: UTF-8 data to slugify.
- **Behavior:**
  - Replaces any existing buffer content.
  - Writes slugified output.
  - Safe for stack-allocated buffers (can use `alloca` for temporary storage).

#### Version Information

**`mulle_slug_get_version()`**

- **Purpose:** Retrieve the library version at runtime.
- **Returns:** Combined version number (major, minor, patch encoded).

**`mulle_slug_get_version_major()`, `mulle_slug_get_version_minor()`, `mulle_slug_get_version_patch()`**

- **Purpose:** Extract individual version components.

### 3.2 Supported Character Classes

The library normalizes:

- **Latin Characters:** À-ÿ, including diacritics and variants
- **Greek Characters:** α-ώ, both lowercase and uppercase, with accents
- **Cyrillic Characters:** Basic Cyrillic alphabet normalization
- **Special Ligatures:** æ → "ae", ß → "ss", etc.
- **Symbols:** Greek letters like π → "p", mathematical symbols
- **Spaces & Separators:** Converted to delimiter (default hyphen)
- **Punctuation:** Removed or normalized (e.g., apostrophes removed)

### 3.3 Normalization Rules

- **Diacritics:** Removed and base character preserved (ë → e)
- **Accents:** Normalized to unaccented equivalents (é → e)
- **Ligatures:** Expanded to component letters (æ → ae)
- **Case:** Preserved from original input (mulle-slug does NOT lowercase)
- **Spaces:** Replaced with hyphen (configurable delimiter)
- **Unknown Characters:** Removed or kept depending on UTF-32 classification

## 4. Performance Characteristics

- **Time Complexity:** O(n) where n is the length of the input string in bytes (single pass through UTF-8 stream).
- **Space Complexity:** O(m) where m is the length of the output slug; typical output is 50-80% of input length.
- **Character Mapping:** Uses static lookup tables; no allocations during mapping.
- **UTF-8 Processing:** Direct UTF-8 stream processing; no conversion to UTF-32 for basic operations.
- **Memory Allocation:** Returns newly allocated strings; use buffer functions to avoid allocations.

**Trade-offs:**

- **Speed vs. Flexibility:** Table-based approach is fast but less flexible than regex; custom delimiters require buffer API.
- **Table Size:** Comprehensive Unicode mapping increases binary size (~50-100KB) but provides excellent coverage without dynamic lookups.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Use Buffer API When Building Strings:** If constructing multiple slugs or building URLs, use `mulle_buffer_*` functions to avoid repeated allocations.

2. **Pre-allocate Buffers:** For known maximum lengths, allocate buffers on the stack with `alloca()` or fixed-size arrays to avoid heap allocations entirely.

3. **Character Preservation:** Remember that mulle-slug preserves case; if you need lowercase slugs, apply `mulle_utf_to_lowercase()` before or after slugification depending on your needs.

4. **Allocator Consistency:** When using custom allocators, ensure the same allocator is used for both allocation and deallocation of slug strings.

5. **Idempotency:** Slugifying an already-slugified string typically produces the same result; safe for defensive programming.

### Common Pitfalls:

1. **Forgetting to Free Allocated Strings:** `mulle_utf8_slugify()` returns heap-allocated memory; use `mulle_free()` to avoid leaks.

2. **Assuming Lowercase Output:** Unlike some slug libraries, mulle-slug preserves case; "Hello World" → "Hello-World", not "hello-world".

3. **Mismatch Between Input Encoding:** Ensure input is valid UTF-8; invalid sequences may be dropped or cause undefined behavior.

4. **Buffer Overflow in Fixed Buffers:** If using fixed-size output buffers, ensure adequate size; UTF-8 expansion can occur during normalization (e.g., ß → "ss").

5. **Not Handling Empty Results:** Slugifying text with only special characters may produce empty or very short strings; validate output.

### Idiomatic Usage:

```c
// Quick string slug
char *slug = mulle_utf8_slugify("My Page Title");
// Use slug...
mulle_free(slug);

// Build URL with buffer
struct mulle_buffer buffer;
mulle_buffer_init_default(&buffer);
mulle_buffer_add_string(&buffer, "/articles/");
struct mulle_utf8data title = { "Breaking News!", 15 };
mulle_buffer_add_slugified_utf8data(&buffer, title);
mulle_buffer_add_string(&buffer, ".html");
// buffer now contains: "/articles/Breaking-News.html"
mulle_buffer_done(&buffer);

// Stack allocation for small strings
struct mulle_buffer stack_buffer;
char storage[256];
mulle_buffer_init_fixed(&stack_buffer, storage, sizeof(storage));
struct mulle_utf8data data = { "My Title", 8 };
mulle_buffer_slugify_utf8data(&stack_buffer, data);
// No malloc occurred
mulle_buffer_done(&stack_buffer);
```

## 6. Integration Examples

### Example 1: Basic String Slugification

```c
#include <mulle-slug/mulle-slug.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char *input = "## Take me home!";
    char *slug = mulle_utf8_slugify(input);
    
    printf("Input: %s\n", input);
    printf("Slug: %s\n", slug);
    
    // Must free the allocated slug
    mulle_free(slug);
    
    return 0;
}
// Output: Input: ## Take me home!
//         Slug: Take-me-home
```

### Example 2: Handling Accented Characters

```c
#include <mulle-slug/mulle-slug.h>
#include <stdio.h>

int main() {
    char *inputs[] = {
        "Café au lait",
        "Naïve approach",
        "Москва",           // Moscow in Cyrillic
        "Ελληνικά",         // Greek
        NULL
    };
    
    for (int i = 0; inputs[i]; i++) {
        char *slug = mulle_utf8_slugify(inputs[i]);
        printf("'%s' → '%s'\n", inputs[i], slug);
        mulle_free(slug);
    }
    
    return 0;
}
// Output: 'Café au lait' → 'Cafe-au-lait'
//         'Naïve approach' → 'Naive-approach'
//         'Москва' → (Cyrillic normalized form)
//         'Ελληνικά' → (Greek normalized form)
```

### Example 3: Building URLs with Buffer

```c
#include <mulle-slug/mulle-slug.h>
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

int main() {
    struct mulle_buffer buffer;
    mulle_buffer_init_default(&buffer);
    
    // Build a URL from components
    mulle_buffer_add_string(&buffer, "https://example.com/blog/");
    
    struct mulle_utf8data title = { "Learning C Slugs", 16 };
    mulle_buffer_add_slugified_utf8data(&buffer, title);
    
    mulle_buffer_add_string(&buffer, ".html");
    
    printf("URL: %s\n", buffer._storage);
    // Output: URL: https://example.com/blog/Learning-C-Slugs.html
    
    mulle_buffer_done(&buffer);
    
    return 0;
}
```

### Example 4: Fixed-Size Stack Buffer

```c
#include <mulle-slug/mulle-slug.h>
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>
#include <alloca.h>

int main() {
    const char *article_title = "Performance Tips & Tricks";
    size_t max_slug_len = strlen(article_title) * 2;  // Conservative upper bound
    
    struct mulle_buffer buffer;
    char *storage = alloca(max_slug_len);
    mulle_buffer_init_fixed(&buffer, storage, max_slug_len);
    
    struct mulle_utf8data data = { article_title, strlen(article_title) };
    mulle_buffer_slugify_utf8data(&buffer, data);
    
    printf("Slug (no malloc): %s\n", buffer._storage);
    // No heap allocation occurred
    
    mulle_buffer_done(&buffer);
    
    return 0;
}
```

### Example 5: Processing Multiple Titles

```c
#include <mulle-slug/mulle-slug.h>
#include <mulle-buffer/mulle-buffer.h>
#include <stdio.h>

typedef struct {
    const char *title;
    const char *author;
} Article;

int main() {
    Article articles[] = {
        { "The Art of C Programming", "Kernighan" },
        { "Understanding Pointers & Memory", "Stallman" },
        { "Unicode in Modern Languages", "Pike" },
        { NULL, NULL }
    };
    
    struct mulle_buffer buffer;
    mulle_buffer_init_default(&buffer);
    
    for (int i = 0; articles[i].title; i++) {
        mulle_buffer_reset(&buffer);
        
        struct mulle_utf8data title_data = {
            (char *)articles[i].title,
            strlen(articles[i].title)
        };
        
        mulle_buffer_slugify_utf8data(&buffer, title_data);
        
        printf("%s -> /articles/%s\n", 
               articles[i].title,
               buffer._storage);
    }
    
    mulle_buffer_done(&buffer);
    
    return 0;
}
// Output:
// The Art of C Programming -> /articles/The-Art-of-C-Programming
// Understanding Pointers & Memory -> /articles/Understanding-Pointers-Memory
// Unicode in Modern Languages -> /articles/Unicode-in-Modern-Languages
```

### Example 6: UTF8Data with Explicit Length

```c
#include <mulle-slug/mulle-slug.h>
#include <stdio.h>

int main() {
    // Process embedded data (not null-terminated)
    const char *buffer = "Hello\0World\0Test";
    
    // Process just the first part
    struct mulle_utf8data data = { (char *)buffer, 5 };
    struct mulle_utf8data slug = mulle_utf8data_slugify(data, NULL);
    
    printf("From buffer: %.*s\n", (int)slug.length, slug.characters);
    
    // Clean up returned data
    mulle_free(slug.characters);
    
    return 0;
}
```

## 7. Dependencies

Direct mulle-sde library dependencies:
- `mulle-utf`: UTF-8/UTF-16/UTF-32 encoding, decoding, and character classification
- `mulle-buffer`: Growable buffer structure for slug accumulation and stream operations
