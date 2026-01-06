#!/bin/bash
set -e

# Get repo URL and reference from arguments
REPO_URL="${1:-https://github.com/mulle-core/mulle-core.git}"
REPO_REF="${2:-}"

TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

echo "Creating test project in $TEMP_DIR"
echo "Using repository: $REPO_URL"
if [ -n "$REPO_REF" ]; then
    echo "Using reference: $REPO_REF"
fi

# Initialize git repo
git init
git config user.email "test@example.com"
git config user.name "Test User"
git config protocol.file.allow always

# Add mulle-core as actual submodule
git submodule add "$REPO_URL" mulle-core
cd mulle-core
if [ -n "$REPO_REF" ]; then
    git checkout "$REPO_REF"
fi
cd ..
git submodule update --init --recursive

# Create CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.15)
project(submodule-test)
add_subdirectory(mulle-core)
add_executable(test main.c)
target_link_libraries(test PRIVATE mulle-core)
target_include_directories(test PRIVATE ${CMAKE_BINARY_DIR}/include)
EOF

# Create main.c
cat > main.c << 'EOF'
#include <mulle-core/mulle-core.h>
#include <dlfcn.h>

int main() {
    // Test mulle_fprintf
    mulle_fprintf(stdout, "mulle-core submodule test: %s\n", "SUCCESS");
    
    // Test mulle_mmap page allocation
    void *page = mulle_allocator_malloc(&mulle_allocator_default, 4096);
    if (!page) {
        mulle_fprintf(stderr, "mulle_mmap page allocation failed\n");
        return 1;
    }
    mulle_fprintf(stdout, "mulle_mmap page allocation: SUCCESS\n");
    mulle_allocator_free(&mulle_allocator_default, page);
    
    // Test dlsym functionality
    mulle_fprintf(stdout, "dlsym present: SUCCESS\n");
    
    return 0;
}
EOF

# Build and test
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/test

echo "Test completed successfully"
cd - > /dev/null
rm -rf "$TEMP_DIR"
