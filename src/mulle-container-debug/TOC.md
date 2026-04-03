# mulle-container-debug Library Documentation for AI
<!-- Keywords: debugging, inspection -->

## 1. Introduction & Purpose

mulle-container-debug provides debugging support for mulle-container data structures, primarily through `describe` functions that generate human-readable representations of container contents. Currently implements detailed inspection for mulle-pointerarray with plans to extend to other container types. Used during development and debugging to visualize container state.

## 2. Key Concepts & Design Philosophy

- **Describe Functions**: Generate detailed, formatted string descriptions of container contents
- **Inspection Only**: Non-invasive; read-only access to container state for debugging
- **Pretty Printing**: Human-friendly formatting with proper indentation and structure
- **Extensibility**: Framework for adding describe functions to other mulle-container types
- **Development Focus**: Primarily for use during development and debugging workflows

## 3. Core API & Data Structures

### Debug Functions

#### Pointerarray Describe

- `mulle_pointerarray_describe(array)` → `char *`: Generates description string for pointerarray
- `mulle_pointerarray_describe_with_callback(array, callback)` → `char *`: Custom callback for element description

#### Generic Describe (Framework)

- Description format typically includes:
  - Container type and statistics
  - Element count and capacity
  - Memory address ranges
  - Per-element details (where applicable)

## 4. Performance Characteristics

- **Describe Operation**: O(n) where n is number of elements (must traverse entire container)
- **Memory**: Allocates new string for result; user responsible for freeing
- **I/O**: Suitable for logging and debugging; not for production hot paths

## 5. AI Usage Recommendations & Patterns

### Best Practices

- **Development Only**: Use in debug builds or testing, not production
- **Logging**: Pipe describe output to logging system for issue diagnosis
- **GDB Integration**: Can call describe functions directly in debugger
- **Conditional Compilation**: Wrap describe calls in #ifdef DEBUG

### Common Pitfalls

- **Memory Management**: Allocated descriptions must be freed by caller
- **Large Containers**: Describing enormous containers generates very large strings
- **Performance**: Don't call in tight loops; use sparingly during debugging

## 6. Integration Examples

### Example 1: Basic Array Describe

```c
#include <mulle-container-debug/mulle-container-debug.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    struct mulle_pointerarray array;
    
    mulle_pointerarray_init_default(&array);
    
    mulle_pointerarray_add(&array, "item1");
    mulle_pointerarray_add(&array, "item2");
    
    char *desc = mulle_pointerarray_describe(&array);
    printf("%s\n", desc);
    free(desc);
    
    mulle_pointerarray_done(&array);
    return 0;
}
```

### Example 2: Describe in Logging

```c
#include <mulle-container-debug/mulle-container-debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void log_array_state(struct mulle_pointerarray *array, const char *label) {
    char *desc = mulle_pointerarray_describe(array);
    fprintf(stderr, "[DEBUG] %s:\n%s\n", label, desc);
    free(desc);
}

int main() {
    struct mulle_pointerarray array;
    
    mulle_pointerarray_init_default(&array);
    
    for (int i = 0; i < 5; i++) {
        mulle_pointerarray_add(&array, (void *)(intptr_t)i);
        log_array_state(&array, "After adding item");
    }
    
    mulle_pointerarray_done(&array);
    return 0;
}
```

### Example 3: Debug Breakpoint Helper

```c
#include <mulle-container-debug/mulle-container-debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void inspect_array(struct mulle_pointerarray *array) {
    char *desc = mulle_pointerarray_describe(array);
    printf("\n=== Array State ===\n%s\n===================\n", desc);
    free(desc);
}

int main() {
    struct mulle_pointerarray array;
    
    mulle_pointerarray_init_default(&array);
    
    for (int i = 0; i < 10; i++) {
        mulle_pointerarray_add(&array, (void *)(intptr_t)(i * 100));
    }
    
    // Can call this from debugger breakpoint
    inspect_array(&array);
    
    mulle_pointerarray_done(&array);
    return 0;
}
```

## 7. Dependencies

- mulle-c11
- mulle-container (for container structures)
