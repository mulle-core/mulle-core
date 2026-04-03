# mulle-regex Library Documentation for AI
<!-- Keywords: regex, unicode -->

## 1. Introduction & Purpose

**mulle-regex** is a Unicode-enabled regular expression library based on Henry Spencer's well-known regexp(3) implementation. It provides:

- **egrep-style regular expressions**: Supports extended regular expressions (ERE) syntax
- **Unicode support**: Works with UTF-32 encoded strings (adapted from original ASCII version)
- **Pattern matching**: Execute regex matching on Unicode strings
- **Pattern substitution**: Replace matched patterns with replacement text
- **Capture groups**: Support for backreferences (\1-\9) in patterns and replacements
- **Range queries**: Extract matched ranges and capture group positions
- **Simple, proven algorithm**: Based on nondeterministic automata (simpler and faster at compilation, adequate execution speed)

This library is a foundational component of mulle-core and provides regex functionality for text processing, pattern matching, and string manipulation in applications and frameworks.

## 2. Key Concepts & Design Philosophy

- **NFA-based**: Uses nondeterministic automata rather than DFA; simpler, smaller, faster compilation
- **Unicode-centric**: Operates on UTF-32 encoded strings (mulle_utf32_t arrays)
- **Extended regex syntax**: Supports egrep pattern syntax (not POSIX.2 compliant, but practical)
- **Separate compile/execute**: Compile pattern once, execute multiple times
- **Manual memory management**: Caller allocates buffers and manages memory
- **Capture group support**: Patterns can include parentheses for grouping; replacements can reference \1-\9
- **Stateful matching**: Tracks matched ranges and capture group positions after execute

## 3. Core API & Data Structures

### 3.1. `mulle-utf32regex.h`

#### `struct mulle_utf32regex`
- **Purpose**: Compiled regular expression pattern (opaque structure)
- **Lifecycle**:
  - Created by `mulle_utf32regex_compile(pattern)`
  - Destroyed by `mulle_utf32regex_free(regex)` or `mulle_free(regex)`
  - Can be reused for multiple matches on different strings

#### Core Compilation Function

- `struct mulle_utf32regex *mulle_utf32regex_compile(mulle_utf32_t *pattern)`: Compile pattern to regex
  - **Parameters**: `pattern` - null-terminated UTF-32 string containing regex pattern
  - **Returns**: Pointer to compiled regex, or NULL if pattern is malformed
  - **Pattern syntax**: egrep style (., *, +, ?, |, [], ^, $, (), \1-\9, etc.)
  - **Compiling fails if**: Unmatched parentheses, invalid escape sequences, malformed character classes

#### Core Execution Function

- `int mulle_utf32regex_execute(struct mulle_utf32regex *regex, mulle_utf32_t *src)`: Execute regex match
  - **Parameters**:
    - `regex`: Compiled regex from `mulle_utf32regex_compile`
    - `src`: null-terminated UTF-32 string to match against
  - **Returns**:
    - 1 if match found (matched text and capture groups are recorded internally)
    - 0 if no match
    - < 0 on error
  - **Side effects**: Updates internal matched ranges accessible via `mulle_utf32regex_range_for_index`

#### Pattern Substitution Functions

- `int mulle_utf32regex_substitute(struct mulle_utf32regex *regex, mulle_utf32_t *replacement, mulle_utf32_t *dst, size_t dst_len, int zero)`: Substitute matched text
  - **Parameters**:
    - `regex`: Compiled regex (must have just executed)
    - `replacement`: Replacement text; can include \1-\9 for capture group references
    - `dst`: Buffer to write substitution result
    - `dst_len`: Size of buffer (in UTF-32 units, not bytes)
    - `zero`: If 1, null-terminates output; if 0, does not
  - **Returns**: 0 on success, < 0 on error
  - **Important**: Does NOT include the prefix and suffix of the original string; caller must handle those
  - **Buffer size**: Use `mulle_utf32regex_substitution_length` to calculate needed space

- `size_t mulle_utf32regex_substitution_length(struct mulle_utf32regex *regex, mulle_utf32_t *replacement)`: Calculate substitution length
  - **Parameters**: Same as substitute
  - **Returns**: Length in UTF-32 characters of the substituted text (not including null terminator)
  - **Returns**: (size_t)-1 on error

- `static inline size_t mulle_utf32regex_substitution_buffer_size(struct mulle_utf32regex *regex, mulle_utf32_t *replacement)`: Calculate buffer size in bytes
  - Convenience macro: `(substitution_length + 1) * sizeof(mulle_utf32_t)`
  - Includes space for null terminator

#### Convenience Functions (Single-step operations)

- `mulle_utf32_t *mulle_utf32_match(mulle_utf32_t *pattern, mulle_utf32_t *src)`: Match pattern against string
  - **Returns**: Malloced matched substring (null-terminated), or NULL if no match or error
  - **Combines**: Compile and execute in one call
  - **Caller must free** the returned string

- `mulle_utf32_t *mulle_utf32_substitute(mulle_utf32_t *pattern, mulle_utf32_t *replacement, mulle_utf32_t *src)`: Substitute pattern in string
  - **Returns**: Malloced result string (null-terminated), or NULL if error
  - **Combines**: Compile, execute, and substitute in one call
  - **Important**: Result includes ONLY the substituted match, not prefix/suffix
  - **Caller must free** the returned string

#### Capture Group Access

- `struct mulle_range mulle_utf32regex_range_for_index(struct mulle_utf32regex *regex, unsigned int i)`: Get matched range for capture group
  - **Parameters**:
    - `i = 0`: Range of entire match
    - `i = 1-9`: Range of \1 through \9 capture group
  - **Returns**: `struct mulle_range` with location and length in the source string
  - **Valid after**: `mulle_utf32regex_execute` returns 1 (match found)
  - **Use with**: `&src[range.location]` to access matched text

#### Memory Management

- `mulle_utf32regex_free(regex)`: Free compiled regex (inline wrapper around `mulle_free`)
- Patterns compiled with `mulle_utf32regex_compile` are heap-allocated and should be freed
- Convenience functions return heap-allocated strings; caller must free

### 3.2. Regex Pattern Syntax

**Basic Elements**:
- `.` - Any single character (except newline in strict mode)
- `*` - Zero or more of preceding element
- `+` - One or more of preceding element
- `?` - Zero or one of preceding element
- `^` - Start of string
- `$` - End of string
- `|` - Alternation (or)

**Character Classes**:
- `[abc]` - Character set (a, b, or c)
- `[a-z]` - Character range
- `[^abc]` - Negated character set (any except a, b, c)

**Grouping**:
- `(pattern)` - Group (capture group, can be referenced as \1-\9)
- `(?:pattern)` - Non-capturing group (may not be supported)

**Escaping**:
- `\` - Escape special character (e.g., `\.` matches literal dot)
- `\1-\9` - Backreference to capture group (in replacement text or pattern)

**Anchors**:
- `^` - Match start of string
- `$` - Match end of string
- `^pattern$` - Match entire string

**Examples**:
- `[0-9]+` - One or more digits
- `[a-zA-Z_][a-zA-Z0-9_]*` - Identifier pattern
- `(.*?)@(.*)` - Email-like pattern (capture user and domain)
- `(\w+)\s+(\d+)` - Word followed by digits

## 4. Performance Characteristics

- **Compilation**: O(m) for pattern length m; uses NFA so fast to compile
- **Execution**: O(n * m) worst case for n characters and m compiled pattern nodes; average case typically O(n)
- **NFA vs DFA trade-off**: Slower execution but simpler code and faster compilation than DFA
- **Memory**: Compiled regex typically several KB depending on pattern complexity
- **Substitution**: O(n + r) for n characters matched and r characters in replacement
- **Thread-safety**: Compiled regex can be safely shared; execute is thread-safe after compilation (no mutation)

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **Compile once, use many times**: For patterns used repeatedly, compile once and execute multiple times
2. **Buffer management**: Use `mulle_utf32regex_substitution_length` to calculate buffer sizes; add space for null terminator if needed
3. **Handle matches and non-matches**: Check return value of `mulle_utf32regex_execute` for 0 (no match), 1 (match), or < 0 (error)
4. **Extract capture groups**: Use `mulle_utf32regex_range_for_index` after successful match to get capture group ranges
5. **String prefix/suffix handling**: `mulle_utf32regex_substitute` doesn't include prefix/suffix; manually concatenate with `mulle_range_subtract` or similar
6. **UTF-32 encoding**: Ensure strings are properly UTF-32 encoded before passing to regex functions
7. **Error checking**: Always check return values; malformed patterns return NULL, failed execution returns < 0

### Common Pitfalls

1. **Unfreed compiled regex**: `mulle_utf32regex_compile` allocates memory; must call `mulle_utf32regex_free`
2. **Buffer overflow**: `mulle_utf32regex_substitute` overwrites buffer if too small; calculate size with `mulle_utf32regex_substitution_length`
3. **Capture group numbering**: \1-\9 only; can't reference more than 9 capture groups
4. **Prefix/suffix loss**: `mulle_utf32regex_substitute` returns ONLY the substituted match; manually handle prefix/suffix (see test example)
5. **State after execute**: Capture group ranges only valid after successful `mulle_utf32regex_execute` (return == 1)
6. **UTF-32 assumption**: Functions expect UTF-32 encoding; passing UTF-8 or UTF-16 will produce incorrect results
7. **Null termination**: String parameters must be null-terminated UTF-32 strings; some functions expect size in characters not bytes

## 6. Integration Examples

### Example 1: Simple pattern matching

```c
#include <mulle-regex/mulle-regex.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    mulle_utf32_t pattern[] = { 'a', '+', 0 };  // Pattern: a+
    mulle_utf32_t text[] = { 'b', 'a', 'a', 'a', 'c', 0 };  // Text: baaac
    
    struct mulle_utf32regex *regex = mulle_utf32regex_compile(pattern);
    if (!regex) {
        printf("Pattern compilation failed\n");
        return 1;
    }
    
    int result = mulle_utf32regex_execute(regex, text);
    if (result == 1) {
        printf("Match found!\n");
    } else if (result == 0) {
        printf("No match\n");
    } else {
        printf("Error: %d\n", result);
    }
    
    mulle_utf32regex_free(regex);
    return 0;
}
```

### Example 2: Extract matched range

```c
#include <mulle-regex/mulle-regex.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    mulle_utf32_t pattern[] = { '[', '0', '-', '9', ']', '+', 0 };  // \d+
    mulle_utf32_t text[] = { 'a', 'b', 'c', '1', '2', '3', 'd', 'e', 'f', 0 };  // abc123def
    
    struct mulle_utf32regex *regex = mulle_utf32regex_compile(pattern);
    
    if (mulle_utf32regex_execute(regex, text) == 1) {
        struct mulle_range range = mulle_utf32regex_range_for_index(regex, 0);
        printf("Matched at location %zu, length %zu\n", range.location, range.length);
        printf("Matched text: ");
        for (size_t i = 0; i < range.length; i++) {
            printf("%c", (char)text[range.location + i]);
        }
        printf("\n");
    }
    
    mulle_utf32regex_free(regex);
    return 0;
}
```

### Example 3: Capture groups and backreferences

```c
#include <mulle-regex/mulle-regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    // Pattern with two capture groups: (word) (number)
    mulle_utf32_t pattern[] = { '(', '[', 'a', '-', 'z', ']', '+', ')', ' ', '(', '[', '0', '-', '9', ']', '+', ')', 0 };
    mulle_utf32_t text[] = { 'h', 'e', 'l', 'l', 'o', ' ', '1', '2', '3', 0 };  // hello 123
    
    struct mulle_utf32regex *regex = mulle_utf32regex_compile(pattern);
    
    if (mulle_utf32regex_execute(regex, text) == 1) {
        struct mulle_range group0 = mulle_utf32regex_range_for_index(regex, 0);  // Full match
        struct mulle_range group1 = mulle_utf32regex_range_for_index(regex, 1);  // First group (word)
        struct mulle_range group2 = mulle_utf32regex_range_for_index(regex, 2);  // Second group (number)
        
        printf("Full match at %zu-%zu\n", group0.location, group0.location + group0.length - 1);
        printf("Group 1 at %zu-%zu\n", group1.location, group1.location + group1.length - 1);
        printf("Group 2 at %zu-%zu\n", group2.location, group2.location + group2.length - 1);
    }
    
    mulle_utf32regex_free(regex);
    return 0;
}
```

### Example 4: String substitution

```c
#include <mulle-regex/mulle-regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    mulle_utf32_t pattern[] = { 'b', '+', 0 };  // Pattern: b+
    mulle_utf32_t text[] = { 'a', 'b', 'b', 'b', 'c', 0 };  // Text: abbbc
    mulle_utf32_t replacement[] = { 'B', 0 };  // Replace with: B
    
    struct mulle_utf32regex *regex = mulle_utf32regex_compile(pattern);
    
    if (mulle_utf32regex_execute(regex, text) == 1) {
        size_t subst_len = mulle_utf32regex_substitution_length(regex, replacement);
        mulle_utf32_t *buffer = malloc((subst_len + 1) * sizeof(mulle_utf32_t));
        
        if (mulle_utf32regex_substitute(regex, replacement, buffer, subst_len + 1, 1) == 0) {
            printf("Substituted: B (length %zu)\n", subst_len);
            // Note: buffer contains only "B", not "aBc" - must add prefix/suffix manually
        }
        
        free(buffer);
    }
    
    mulle_utf32regex_free(regex);
    return 0;
}
```

### Example 5: Convenience single-step match

```c
#include <mulle-regex/mulle-regex.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    mulle_utf32_t pattern[] = { 'd', '+', 0 };  // Match digits
    mulle_utf32_t text[] = { 'x', 'y', 'z', '4', '5', '6', 'a', 'b', 'c', 0 };
    
    mulle_utf32_t *matched = mulle_utf32_match(pattern, text);
    
    if (matched) {
        printf("Matched: ");
        for (size_t i = 0; matched[i]; i++) {
            printf("%c", (char)matched[i]);
        }
        printf("\n");
        free(matched);
    } else {
        printf("No match\n");
    }
    
    return 0;
}
```

### Example 6: Substitution with capture groups

```c
#include <mulle-regex/mulle-regex.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
    // Swap two words: (word1) (word2) -> \2 \1
    mulle_utf32_t pattern[] = { '(', '[', 'a', '-', 'z', ']', '+', ')', ' ', '(', '[', 'a', '-', 'z', ']', '+', ')', 0 };
    mulle_utf32_t replacement[] = { '\\', '2', ' ', '\\', '1', 0 };
    mulle_utf32_t text[] = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', 0 };  // hello world
    
    mulle_utf32_t *result = mulle_utf32_substitute(pattern, replacement, text);
    
    if (result) {
        printf("Substituted: ");
        for (size_t i = 0; result[i]; i++) {
            printf("%c", (char)result[i]);
        }
        printf("\n");  // Outputs: world hello
        free(result);
    }
    
    return 0;
}
```

## 7. Dependencies

- `mulle-utf`: UTF-8/UTF-16/UTF-32 analysis and manipulation
- `mulle-c11`: Cross-platform C compiler glue

