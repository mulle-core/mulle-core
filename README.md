# mulle-core

#### 🌋 Amalgamated library of mulle-core + mulle-concurrent + mulle-c

This is an amalgamation of the [mulle-core](//github.com/mulle-core),
[mulle-concurrent](//github.com/mulle-concurrent), and
[mulle-c](//github.com/mulle-c) projects that need not be force-linked. See
the [constituent projects](#Constituents) for documentation, bug reports,
and pull requests.

## Project model

`mulle-core` is a release-time amalgamation, not a second source repository
for all of its constituents. The constituent projects are the canonical source
for the implementations, APIs, behavioral tests, portability tests, and CI.
This repository gathers versioned constituent snapshots into one library and
one envelope header for convenient consumption.

The amalgamated sources are copied from the selected constituent revisions;
they are not independently maintained here. Implementation fixes should be
made in the owning constituent and then brought here by the next
amalgamation. Maintainers may use `mulle-sde` to select dependencies and
regenerate a snapshot, but consumers of a released snapshot do not need
`mulle-sde`.

The amalgamation's own build and release checks focus on the distribution:
compilation, linking, generated manifests, envelope headers, and the supported
Git submodule plus CMake `add_subdirectory` integration. It intentionally does
not duplicate every constituent test suite.

See [the design and project model](dox/DESIGN.md) for the maintainer and
consumer workflow.

mulle-core is tested to build on the following platforms and compiler
matrix:

| OS       | Compiler   | Flags  | Note
|----------|------------|--------|--------------------
| Ubuntu   | GCC        | &nbsp; | 
| Ubuntu   | GCC        | &nbsp; | Out of source build
| Ubuntu   | GCC        | -O3    | 
| Ubuntu   | Clang      | &nbsp; | 
| Ubuntu   | Clang      | Debug  | 
| Windows  | MSVC Win32 | &nbsp; |       
| Windows  | MSVC Win64 | &nbsp; |       
| Windows  | GCC        | &nbsp; | 
| macOS    | Clang      | &nbsp; | 
| macOS    | GCC        | &nbsp; | 

The advantages of using **mulle-core** over the individual projects are:

* compiles faster than two dozen individual projects
* you only need to link against one library file
* `#include` statements may remain unchanged or simplify to `#include <mulle-core/mulle-core.h>`






## Documentation & Guides

* [API Summary](asset/dox/api/toc)
* [Coder Guide](asset/howto/coder/mulle-core)
* [Debugger Guide](asset/howto/debugger/mulle-core)
* [Verifier Guide](asset/howto/verifier/mulle-core)






## Constituents


| Constituent                                  | Description
|----------------------------------------------|-----------------------
| [mulle-core/mulle-allocator](https://github.com/mulle-c/mulle-allocator) | 🔄 Flexible C memory allocation scheme
| [mulle-core/mulle-buffer](https://github.com/mulle-c/mulle-buffer) | ↗️  A growable C char array and also a stream - on stack and heap
| [mulle-core/mulle-c11](https://github.com/mulle-c/mulle-c11) | 🔀 Cross-platform C compiler glue (and some cpp conveniences)
| [mulle-core/mulle-container-debug](https://github.com/mulle-c/mulle-container-debug) | 🛄 Debugging support for mulle-container
| [mulle-core/mulle-container](https://github.com/mulle-c/mulle-container) | 🛄 Arrays, hashtables and a queue
| [mulle-core/mulle-data](https://github.com/mulle-c/mulle-data) | #️⃣  A collection of hash functions
| [mulle-core/mulle-http](https://github.com/mulle-c/mulle-http) | 🈚 http URL parser
| [mulle-core/mulle-rbtree](https://github.com/mulle-c/mulle-rbtree) | 🍫 mulle-rbtree organizes data in a red/black tree
| [mulle-core/mulle-regex](https://github.com/mulle-c/mulle-regex) | 📣 Unicode regex library
| [mulle-core/mulle-slug](https://github.com/mulle-c/mulle-slug) | 🐌 Creates URL slugs
| [mulle-core/mulle-storage](https://github.com/mulle-c/mulle-storage) | 🛅 Memory management for tree nodes
| [mulle-core/mulle-unicode](https://github.com/mulle-c/mulle-unicode) | 🈚 Unicode ctype like library
| [mulle-core/mulle-url](https://github.com/mulle-c/mulle-url) | 🈷️ Support for URL parsing
| [mulle-core/mulle-utf](https://github.com/mulle-c/mulle-utf) | 🔤 UTF8-16-32 analysis and manipulation library
| [mulle-core/mulle-vararg](https://github.com/mulle-c/mulle-vararg) | ⏪ Access variable arguments in struct layout fashion in C
| [mintomic](https://github.com/mulle-concurrent/mintomic) | For more information, see [the documentation](http://mintomic.github.io/) or the accompanying blog post, [Introducing Mintomic](http://preshing.com/20130505/introducing-mintomic-a-small-portable-lock-free-api).
| [mulle-core/mulle-aba](https://github.com/mulle-concurrent/mulle-aba) | 🚮 A lock-free, cross-platform solution to the ABA problem
| [mulle-core/mulle-concurrent](https://github.com/mulle-concurrent/mulle-concurrent) | 📶 A lock- and wait-free hashtable (and an array too), written in C
| [mulle-core/mulle-fifo](https://github.com/mulle-concurrent/mulle-fifo) | 🐍 mulle-fifo fixed sized producer/consumer FIFOs holding `void *`
| [mulle-core/mulle-linkedlist](https://github.com/mulle-concurrent/mulle-linkedlist) | 🔂 mulle-linkedlist a wait and lock-free linked list
| [mulle-core/mulle-multififo](https://github.com/mulle-concurrent/mulle-multififo) | 🐛 mulle-multififo multi-producer/multi-consumer FIFO holding `void *`
| [mulle-core/mulle-thread](https://github.com/mulle-concurrent/mulle-thread) | 🔠 Cross-platform thread/mutex/tss/atomic operations in C
| [mulle-core/mulle-dtostr](https://github.com/mulle-core/mulle-dtostr) | 🧶 Double to string conversion
| [mulle-core/mulle-fprintf](https://github.com/mulle-core/mulle-fprintf) | 🔢 mulle-fprintf marries mulle-sprintf to stdio.h
| [mulle-core/mulle-mmap](https://github.com/mulle-core/mulle-mmap) | 🇧🇿 Memory mapped file access
| [mulle-core/mulle-rbtree-debug](https://github.com/mulle-core/mulle-rbtree-debug) | 🍫 mulle-rbtree-debug organizes data in a red/black tree
| [mulle-core/mulle-sprintf](https://github.com/mulle-core/mulle-sprintf) | 🔢 An extensible sprintf function supporting stdarg and mulle-vararg
| [mulle-core/mulle-time](https://github.com/mulle-core/mulle-time) | 🕕 Simple time types with arithmetic on timespec and timeval

> #### Add another constituent to the amalgamation
>
> ``` bash
> mulle-sde dependency add --amalgamated \
>                          --fetchoptions "clibmode=copy" \
>                          --address src/mulle-container-debug \
>                          clib:mulle-c/mulle-container-debug
> ```
>
> Then edit `mulle-core.h` and add the envelope header to the others.
>


## Add

There are various methods how to get mulle-core into your project.
One common denominator is that you will
`#include <mulle-core/mulle-core.h>` in your sources and link
with `-lmulle-core`.


### Add as a dependency with mulle-sde

Use [mulle-sde](//github.com/mulle-sde) to add mulle-core to your project:

``` sh
mulle-sde add github:mulle-core/mulle-core
```

This library does not include [mulle-atinit](//github.com/mulle-core/mulle-atinit)
and [mulle-atexit](//github.com/mulle-core/mulle-atexit) and
[mulle-testallocator](//github.com/mulle-core/mulle-testallocator). If you
add these libraries, it is important that mulle-core is added before them.




### Add as subproject with cmake and git

``` bash
git submodule add https://github.com/mulle-core/mulle-core.git stash/mulle-core
git submodule update --init --recursive
```

Add this to your `CMakeLists.txt`:

``` cmake
add_subdirectory( stash/mulle-core)
target_link_libraries( ${PROJECT_NAME} PRIVATE mulle-core)
```


## Install

Use [mulle-sde](//github.com/mulle-sde) to build and install mulle-core and all dependencies:

``` sh
mulle-sde install --prefix /usr/local \
   https://github.com/mulle-core/mulle-core/archive/latest.tar.gz
```

### Legacy Installation

Download the latest [tar](https://github.com/mulle-core/mulle-core/archive/refs/tags/latest.tar.gz) or [zip](https://github.com/mulle-core/mulle-core/archive/refs/tags/latest.zip) archive and unpack it.

Install **mulle-core** into `/usr/local` with [cmake](https://cmake.org):

``` sh
PREFIX_DIR="/usr/local"
cmake -B build                               \
      -DMULLE_SDK_PATH="${PREFIX_DIR}"       \
      -DCMAKE_INSTALL_PREFIX="${PREFIX_DIR}" \
      -DCMAKE_PREFIX_PATH="${PREFIX_DIR}"    \
      -DCMAKE_BUILD_TYPE=Release &&
cmake --build build --config Release &&
cmake --install build --config Release
```

## Author

[Nat!](https://mulle-kybernetik.com/weblog) for Mulle kybernetiK  



