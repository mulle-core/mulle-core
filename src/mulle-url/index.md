# mulle-url Library Documentation for AI
<!-- Keywords: url, validation -->

## 1. Introduction & Purpose

mulle-url is a character classification library for URL parsing and validation. It determines which Unicode characters are valid for different parts of a URL (scheme, host, user, password, path, query, fragment). This library solves the problem of RFC-compliant URL component validation without requiring full HTTP parsing. It provides table-driven character validation functions enabling efficient, character-by-character verification of URL components. This is a foundational utility in the mulle-c ecosystem for web frameworks and URL processing.

## 2. Key Concepts & Design Philosophy

**Design Principles:**

- **Component-Based Validation:** Each URL part (scheme, host, user, etc.) has specific valid character sets according to RFC specifications.

- **Table-Driven Lookup:** Uses efficient lookup tables for O(1) character validation.

- **Dual-Width Support:** Provides UTF-16 (uint16_t) and UTF-32 (int32_t) validation functions.

- **Plane Queries:** Includes plane-level queries for optimization.

- **Non-Destructive:** Only validates; does not modify or encode characters.

- **Compact:** Minimal binary overhead (~50KB).

## 3. Core API & Data Structures

### 3.1 URL Component Validation Functions

#### Scheme Validation

**`int mulle_unicode_is_validurlscheme(int32_t c)`**

- **Purpose:** Check if character is valid in URL scheme (e.g., "https", "ftp").
- **Valid Characters:** Letters, digits, +, -, . (scheme must start with letter).
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurlscheme(uint16_t c)`**

- **Purpose:** UTF-16 variant.

**`int mulle_unicode_is_validurlschemeplane(unsigned int plane)`**

- **Purpose:** Query if plane contains valid scheme characters.

#### Host Validation

**`int mulle_unicode_is_validurlhost(int32_t c)`**

- **Purpose:** Check if character is valid in URL host/domain.
- **Valid Characters:** Letters, digits, hyphen, period (domain components).
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurlhost(uint16_t c)`**

- **Purpose:** UTF-16 variant.

#### User Validation

**`int mulle_unicode_is_validurluser(int32_t c)`**

- **Purpose:** Check if character is valid in URL user component.
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurluser(uint16_t c)`**

- **Purpose:** UTF-16 variant.

#### Password Validation

**`int mulle_unicode_is_validurlpassword(int32_t c)`**

- **Purpose:** Check if character is valid in URL password component.
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurlpassword(uint16_t c)`**

- **Purpose:** UTF-16 variant.

#### Path Validation

**`int mulle_unicode_is_validurlpath(int32_t c)`**

- **Purpose:** Check if character is valid in URL path.
- **Valid Characters:** Alphanumerics, hyphen, period, underscore, tilde, and percent-encoding.
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurlpath(uint16_t c)`**

- **Purpose:** UTF-16 variant.

#### Query Validation

**`int mulle_unicode_is_validurlquery(int32_t c)`**

- **Purpose:** Check if character is valid in URL query string.
- **Valid Characters:** Path characters plus & and =.
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurlquery(uint16_t c)`**

- **Purpose:** UTF-16 variant.

#### Fragment Validation

**`int mulle_unicode_is_validurlfragment(int32_t c)`**

- **Purpose:** Check if character is valid in URL fragment.
- **Returns:** Non-zero if valid, 0 otherwise.

**`int mulle_unicode16_is_validurlfragment(uint16_t c)`**

- **Purpose:** UTF-16 variant.

#### Percent-Escape Validation

**`int mulle_unicode_is_nonpercentescape(int32_t c)`**

- **Purpose:** Check if character should NOT be percent-encoded (safe characters).
- **Returns:** Non-zero if character is safe (no encoding needed), 0 if encoding recommended.

**`int mulle_unicode16_is_nonpercentescape(uint16_t c)`**

- **Purpose:** UTF-16 variant.

## 4. Performance Characteristics

- **Validation Time:** O(1) table lookup per character.
- **Memory:** ~50KB binary size for all validation tables.
- **Cache Efficiency:** Small lookup tables fit in L1 cache.
- **Thread Safety:** No global state; suitable for concurrent use.

## 5. AI Usage Recommendations & Patterns

### Best Practices:

1. **Validate Component-By-Component:** Use specific validators for each URL part.

2. **UTF-32 for Full Support:** Use int32_t functions for complete Unicode support.

3. **Normalize Before Validation:** Consider normalizing or decoding percent-escapes before validation.

4. **Use with URL Parsing:** Combine with parsing logic to validate each component.

### Common Pitfalls:

1. **Assuming ASCII Equivalence:** URL validation has specific rules beyond basic alphanumeric.

2. **UTF-16 Limitations:** UTF-16 functions only handle BMP; use UTF-32 for full Unicode URLs.

3. **Missing Percent-Encoding:** Reserved characters in wrong components require percent-encoding (e.g., space → %20).

## 6. Integration Examples

### Example 1: Validating URL Components

```c
#include <mulle-url/mulle-url.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int is_valid_component(const int32_t *chars, size_t len,
                       int (*validator)(int32_t)) {
    for (size_t i = 0; i < len; i++)
        if (!validator(chars[i]))
            return 0;
    return 1;
}

int main() {
    int32_t scheme[] = { 'h', 't', 't', 'p', 's' };
    int32_t host[] = { 'e', 'x', 'a', 'm', 'p', 'l', 'e', '.', 'c', 'o', 'm' };
    int32_t path[] = { '/', 'p', 'a', 't', 'h' };
    
    printf("Scheme valid: %s\n",
           is_valid_component(scheme, 5, mulle_unicode_is_validurlscheme) ? "YES" : "NO");
    printf("Host valid: %s\n",
           is_valid_component(host, 11, mulle_unicode_is_validurlhost) ? "YES" : "NO");
    printf("Path valid: %s\n",
           is_valid_component(path, 5, mulle_unicode_is_validurlpath) ? "YES" : "NO");
    
    return 0;
}
```

### Example 2: Character-by-Character URL Parsing

```c
#include <mulle-url/mulle-url.h>
#include <stdio.h>
#include <stdint.h>

int main() {
    const char *url = "https://user:pass@example.com:8080/path?query=value#fragment";
    
    // Validate scheme
    printf("h: %s\n", mulle_unicode_is_validurlscheme('h') ? "valid" : "invalid");
    printf("t: %s\n", mulle_unicode_is_validurlscheme('t') ? "valid" : "invalid");
    
    // Validate host characters
    printf("example.com:\n");
    for (char c = 'a'; c <= 'z'; c++)
        printf("  %c: %s\n", c, mulle_unicode_is_validurlhost(c) ? "valid" : "invalid");
    
    printf("  /: %s\n", mulle_unicode_is_validurlhost('/') ? "valid" : "invalid");
    printf("  .: %s\n", mulle_unicode_is_validurlhost('.') ? "valid" : "invalid");
    
    return 0;
}
```

### Example 3: Path Component Validation

```c
#include <mulle-url/mulle-url.h>
#include <stdio.h>
#include <stdint.h>

int validate_path(const int32_t *path, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (!mulle_unicode_is_validurlpath(path[i])) {
            printf("Invalid character at position %zu: U+%04X\n", i, path[i]);
            return 0;
        }
    }
    return 1;
}

int main() {
    int32_t path1[] = { '/', 'p', 'a', 't', 'h', '/', 't', 'o', '/', 'r', 'e', 's', 'o', 'u', 'r', 'c', 'e' };
    int32_t path2[] = { '/', 'p', 'a', 't', 'h', ' ', 'w', 'i', 't', 'h', ' ', 's', 'p', 'a', 'c', 'e' };  // INVALID
    
    printf("Path 1: %s\n", validate_path(path1, 17) ? "VALID" : "INVALID");
    printf("Path 2: %s\n", validate_path(path2, 17) ? "VALID" : "INVALID");
    
    return 0;
}
```

## 7. Dependencies

Direct mulle-sde dependencies:
- `mulle-c11`: C99/C11 compatibility macros and utilities
