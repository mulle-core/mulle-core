# mulle-utf Library Documentation for AI
<!-- Keywords: utf, encoding -->

## 1. Introduction & Purpose

mulle-utf is a comprehensive UTF-8/UTF-16/UTF-32 character encoding, decoding, and string manipulation library for C99. It provides low-level character operations (character type validation, encoding length calculation, character reading/writing) and high-level string functions (validation, conversion, analysis). The library handles the complexities of variable-width encodings transparently. This is a foundational utility in the mulle-c ecosystem, forming the backbone of NSString equivalent functionality and text processing.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Multi-Encoding Support:** Native handling of UTF-8 (most common), UTF-16 (for platform APIs), and UTF-32 (convenient for algorithms).

- **Validation-Based:** Distinguishes between valid and invalid UTF sequences according to RFC 3629 (UTF-8) and Unicode standards.

- **Character-Centric:** Provides functions for reading/writing individual characters (code points) in various encodings.

- **Zero-Copy Operations:** Many functions operate in-place on buffers without allocation.

- **Rover Pattern:** Iterator-like structures for safe character-by-character traversal without revalidation.

- **Compact Encodings:** Includes `mulle_char7` and `mulle_char5` for compressing small ASCII strings into integers.

- **No Allocations:** All core operations use stack and buffer parameters; no hidden allocations.

## 3. Core API & Data Structures

### 3.1 `mulle-utf8.h` - UTF-8 Character Operations

#### Character Classification

**`int mulle_utf8_is_asciicharacter(char c)`**

- **Purpose:** Check if byte is ASCII (0x00-0x7F).
- **Returns:** Non-zero if ASCII.

**`int mulle_utf8_is_invalidstartcharacter(char c)`**

- **Purpose:** Check if byte cannot start a UTF-8 sequence.
- **Invalid:** 0x80-0xBF (continuation only), 0xC0-0xC1, 0xF5-0xFF.
- **Returns:** Non-zero if invalid start.

**`int mulle_utf8_is_validcontinuationcharacter(char c)`**

- **Purpose:** Check if byte is valid continuation (0x80-0xBF).
- **Returns:** Non-zero if valid continuation.

#### Character Type Detection

**`int mulle_utf8_get_startcharactertype(char c)`**

- **Purpose:** Classify a byte as start character type.
- **Returns:** 
  - `mulle_utf8_ascii_start_character` (0x00-0x7F)
  - `mulle_utf8_multiple_start_character` (multi-byte start)
  - `mulle_utf8_invalid_start_character` (invalid)

#### Length Calculation

**`size_t mulle_utf8_get_extracharacterslength(char c)`**

- **Purpose:** Get number of continuation bytes for a multi-byte sequence.
- **Parameters:** `c` - valid multi-byte start character.
- **Returns:** 1, 2, or 3 (extra bytes).

**`size_t mulle_utf8_utf16length(char *src, size_t len)`**

- **Purpose:** Calculate UTF-16 length needed for UTF-8 string.
- **Returns:** Number of UTF-16 code units.

**`size_t mulle_utf8_utf32length(char *src, size_t len)`**

- **Purpose:** Calculate number of code points in UTF-8 string.
- **Returns:** Number of UTF-32 characters (code points).

#### Validation and Detection

**`int mulle_utf8_has_leading_bomcharacter(char *src, size_t len)`**

- **Purpose:** Check if UTF-8 string starts with BOM (U+FEFF).
- **Returns:** Non-zero if BOM present.

**`int mulle_utf8_are_valid_extracharacters(char *s, size_t len, mulle_utf32_t *p_c)`**

- **Purpose:** Validate continuation bytes and decode to code point.
- **Parameters:**
  - `s`: Pointer to first continuation byte.
  - `len`: Number of bytes available (must be `get_extracharacterslength() + 1`).
  - `p_c`: Output code point.
- **Returns:** Non-zero if valid.

**`int mulle_utf8_information(char *s, size_t len, struct mulle_utf_information *info)`**

- **Purpose:** Analyze UTF-8 string properties.
- **Returns:** 0 if valid; -1 if invalid.

**`int mulle_utf8_is_ascii(char *s, size_t len)`**

- **Purpose:** Check if entire UTF-8 string is ASCII-only.
- **Returns:** Non-zero if all characters < 0x80.

**`char *mulle_utf8_validate(char *src, size_t len)`**

- **Purpose:** Validate UTF-8 string and return error position.
- **Returns:** Pointer to first invalid byte (NULL if valid).

#### Character Reading/Writing

**`mulle_utf32_t _mulle_utf8_next_utf32character(char **s_p)`**

- **Purpose:** Read and advance past next UTF-8 character (destructive on pointer).
- **Parameters:** `s_p` - pointer-to-pointer; updated to position after character.
- **Returns:** UTF-32 code point.

**`mulle_utf32_t _mulle_utf8_previous_utf32character(char **s_p)`**

- **Purpose:** Read and retreat before previous UTF-8 character.

**`mulle_utf32_t mulle_utf8_next_utf32character(char **s_p)`** (inline wrapper)

- **Purpose:** Safe version checking ASCII before decoding.

### 3.2 `mulle-utf16.h` - UTF-16 Character Operations

**`int mulle_utf16_is_highsurrogate(uint16_t c)`**

- **Purpose:** Check if character is high surrogate (0xD800-0xDBFF).
- **Returns:** Non-zero if high surrogate.

**`int mulle_utf16_is_lowsurrogate(uint16_t c)`**

- **Purpose:** Check if character is low surrogate (0xDC00-0xDFFF).
- **Returns:** Non-zero if low surrogate.

**`size_t mulle_utf16_utf8length(uint16_t *src, size_t len)`**

- **Purpose:** Calculate UTF-8 byte length for UTF-16 string.
- **Returns:** Number of UTF-8 bytes.

**`size_t mulle_utf16_utf32length(uint16_t *src, size_t len)`**

- **Purpose:** Calculate number of code points (accounting for surrogates).
- **Returns:** Number of code points.

**`uint16_t *mulle_utf16_validate(uint16_t *src, size_t len)`**

- **Purpose:** Validate UTF-16 string.
- **Returns:** Pointer to first invalid character (NULL if valid).

**`mulle_utf32_t _mulle_utf16_next_utf32character(uint16_t **s_p)`**

- **Purpose:** Read next UTF-32 code point from UTF-16 string.
- **Returns:** Code point; handles surrogates.

### 3.3 `mulle-utf32.h` - UTF-32 Character Operations

**`size_t mulle_utf32_utf8length(mulle_utf32_t *src, size_t len)`**

- **Purpose:** Calculate UTF-8 byte length for UTF-32 string.
- **Returns:** Number of UTF-8 bytes.

**`size_t mulle_utf32_utf16length(mulle_utf32_t *src, size_t len)`**

- **Purpose:** Calculate UTF-16 code unit length (accounting for surrogates).
- **Returns:** Number of UTF-16 units.

**`mulle_utf32_t *mulle_utf32_validate(mulle_utf32_t *src, size_t len)`**

- **Purpose:** Validate UTF-32 string.
- **Returns:** Pointer to first invalid character (NULL if valid).

### 3.4 String Functions

#### `mulle-utf8-string.h`

**`size_t mulle_utf8_strlen(char *s)`**

- **Purpose:** Get UTF-8 string length (number of code points).
- **Parameters:** Null-terminated UTF-8 string.
- **Returns:** Character count.

**`int mulle_utf8_strcmp(char *a, char *b)`**

- **Purpose:** Compare UTF-8 strings.
- **Returns:** <0, 0, or >0 (like strcmp).

#### `mulle-utf16-string.h`

**`size_t mulle_utf16_strlen(uint16_t *s)`**

- **Purpose:** Get UTF-16 string length (code points, accounting for surrogates).

**`int mulle_utf16_strcmp(uint16_t *a, uint16_t *b)`**

- **Purpose:** Compare UTF-16 strings.

#### `mulle-utf32-string.h`

**`size_t mulle_utf32_strlen(mulle_utf32_t *s)`**

- **Purpose:** Get UTF-32 string length (simple character count).

**`int mulle_utf32_strcmp(mulle_utf32_t *a, mulle_utf32_t *b)`**

- **Purpose:** Compare UTF-32 strings.

### 3.5 Special Formats

#### `mulle-char7.h` - Compact ASCII Storage

**`uint64_t mulle_char7_encode(const char *src, size_t len)`**

- **Purpose:** Encode up to 9 ASCII characters into a 64-bit integer (7 bits/char).
- **Returns:** Encoded value.

**`char *mulle_char7_decode(uint64_t value, size_t len, char *dst)`**

- **Purpose:** Decode from 64-bit integer back to ASCII string.
- **Returns:** Pointer to decoded string.

#### `mulle-char5.h` - More Compact Storage

**`uint64_t mulle_char5_encode(const char *src, size_t len)`**

- **Purpose:** Encode characters using 5-bit encoding (alphanumerics + symbols).
- **Returns:** Encoded value.

#### `mulle-ascii.h` - ASCII Analysis

**`int mulle_ascii_is_alnum(int c)`**

- **Purpose:** ASCII alphanumeric check.

**`int mulle_ascii_is_space(int c)`**

- **Purpose:** ASCII whitespace check.

### 3.6 Utility Functions

#### `mulle-utf-type.h`

Defines core types:
- `mulle_utf32_t`: Unsigned 32-bit integer for code points.
- `struct mulle_utf_information`: Analysis results.

#### `mulle-utf-rover.h` - Safe Iteration

**`struct mulle_utf8_rover`**

- **Purpose:** Safe iterator over UTF-8 strings.
- **Operations:**
  - `mulle_utf8_rover_init()`: Create rover for string.
  - `mulle_utf8_rover_next()`: Get next code point and advance.
  - `mulle_utf8_rover_done()`: Cleanup.

### 3.7 Non-Character and Private Character Detection

**`int mulle_utf_is_noncharacter(mulle_utf32_t c)`**

- **Purpose:** Check if code point is designated non-character.

**`int mulle_utf_is_privatecharacter(mulle_utf32_t c)`**

- **Purpose:** Check if code point is private-use character.

## 4. Performance Characteristics

- **UTF-8 Reading:** O(1) per character (up to 4 bytes).
- **Validation:** O(n) single-pass for string length n.
- **Length Calculation:** O(n) to count code points.
- **Char7/Char5 Encoding:** O(1) for fixed-size input.
- **Memory:** Minimal overhead; table-based where needed.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Validate Once, Use Many:** Validate string once; cache result in flag.

2. **Use Rovers for Safe Iteration:** Iterator pattern prevents off-by-one errors.

3. **Check Encoding First:** Use `is_ascii()` to fast-path ASCII-only strings.

4. **Prefer UTF-32 for Algorithms:** Simpler fixed-width representation for text processing.

5. **Handle Surrogates in UTF-16:** Remember UTF-16 uses surrogates for characters > U+FFFF.

### Common Pitfalls:

1. **Assuming UTF-8 Byte-Count Equals Character Count:** UTF-8 is variable-width.

2. **Not Validating Input:** Invalid sequences cause undefined behavior.

3. **Mixing Encodings:** Don't mix UTF-8 and UTF-16 pointers.

4. **Surrogates in UTF-32:** UTF-32 should never contain surrogates; these are invalid.

5. **Off-By-One on Lengths:** Distinguish between byte length and character length.

## 6. Integration Examples

### Example 1: UTF-8 Validation and Length

```c
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

int main() {
    const char *text = "Hëllö, Wörld!";
    size_t byte_len = strlen(text);
    
    if (mulle_utf8_validate((char *)text, byte_len)) {
        printf("Invalid UTF-8\n");
        return 1;
    }
    
    size_t char_count = mulle_utf8_utf32length((char *)text, byte_len);
    size_t utf16_len = mulle_utf8_utf16length((char *)text, byte_len);
    
    printf("Bytes: %zu, Characters: %zu, UTF-16 units: %zu\n",
           byte_len, char_count, utf16_len);
    
    return 0;
}
```

### Example 2: Character-by-Character Iteration

```c
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

int main() {
    char *text = "Hëllo";
    size_t len = strlen(text);
    
    char *s = text;
    mulle_utf32_t c;
    int i = 0;
    
    while (s < text + len) {
        c = mulle_utf8_next_utf32character(&s);
        printf("[%d] U+%04X\n", i++, c);
    }
    
    return 0;
}
```

### Example 3: Using Rovers for Safe Iteration

```c
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

int main() {
    char *text = "Hello World";
    struct mulle_utf8_rover rover;
    
    rover = mulle_utf8_rover_init(text, strlen(text));
    
    mulle_utf32_t c;
    while (mulle_utf8_rover_next(&rover, &c)) {
        printf("%c ", (char)c);
    }
    printf("\n");
    
    mulle_utf8_rover_done(&rover);
    
    return 0;
}
```

### Example 4: UTF-8 to UTF-16 Conversion

```c
#include <mulle-utf/mulle-utf.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    const char *utf8 = "Grüße";
    size_t utf8_len = strlen(utf8);
    
    size_t utf16_len = mulle_utf8_utf16length((char *)utf8, utf8_len);
    uint16_t *utf16 = malloc(sizeof(uint16_t) * (utf16_len + 1));
    
    // Note: actual conversion requires character-by-character reading
    // and writing in UTF-16 format
    
    printf("UTF-8 length: %zu bytes\n", utf8_len);
    printf("UTF-16 length: %zu units\n", utf16_len);
    
    free(utf16);
    
    return 0;
}
```

### Example 5: ASCII Detection and Optimization

```c
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

void process_string(const char *s, size_t len) {
    if (mulle_utf8_is_ascii((char *)s, len)) {
        printf("Pure ASCII - fast path\n");
        // Use simple byte operations
        for (size_t i = 0; i < len; i++)
            process_ascii_byte(s[i]);
    } else {
        printf("Contains UTF-8 - slow path\n");
        // Use UTF-8 aware decoding
        char *p = (char *)s;
        mulle_utf32_t c;
        while (p < s + len) {
            c = mulle_utf8_next_utf32character(&p);
            process_codepoint(c);
        }
    }
}

int main() {
    const char *text1 = "Hello";
    const char *text2 = "Hëllo";
    
    process_string(text1, strlen(text1));
    process_string(text2, strlen(text2));
    
    return 0;
}
```

### Example 6: Char7 Compact Encoding

```c
#include <mulle-utf/mulle-utf.h>
#include <stdio.h>

int main() {
    const char *short_text = "Hello";
    
    // Encode short ASCII strings
    uint64_t encoded = mulle_char7_encode(short_text, strlen(short_text));
    printf("Encoded: 0x%llX\n", (unsigned long long)encoded);
    
    // Decode
    char decoded[10];
    mulle_char7_decode(encoded, strlen(short_text), decoded);
    printf("Decoded: %s\n", decoded);
    
    return 0;
}
```

## 7. Dependencies

Direct mulle-sde dependencies:
- `mulle-c11`: C11 compatibility macros and standard utilities
