#include <stdio.h>

#include <mulle-buffer/mulle-buffer.h>
#include <mulle-utf/mulle-utf.h>
#include <mulle-slug/mulle-slug.h>


void test_mulle_buffer_slugify_utf8data(const char *input) {
    struct mulle_buffer buffer;
    mulle_buffer_init_default(&buffer);
    
    struct mulle_utf8data data;
    data.characters = (char *)input;
    data.length = strlen(input);
    
    mulle_buffer_slugify_utf8data(&buffer, data);
    
    printf("Input: %s\n", input);
    printf("Output: %s\n\n", (char *) mulle_buffer_get_bytes( &buffer));
    
    mulle_buffer_done(&buffer);
}

int main() {
    test_mulle_buffer_slugify_utf8data("Hello, World!");
    test_mulle_buffer_slugify_utf8data(""); // Empty string
    test_mulle_buffer_slugify_utf8data("🙂 Emoji test 👍"); // Emoji content
    test_mulle_buffer_slugify_utf8data("Test!#$%&*()_+-={}[]|;:'\"<>,.?/"); // Special characters
    test_mulle_buffer_slugify_utf8data("Привет мир!"); // Non-English characters (Cyrillic)

    return 0;
}
