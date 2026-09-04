# mulle-core design and project model

## What this repository is

`mulle-core` is an amalgamated distribution of selected C libraries from the
[mulle-core](https://github.com/mulle-core),
[mulle-concurrent](https://github.com/mulle-concurrent), and
[mulle-c](https://github.com/mulle-c) organizations.

The constituent repositories are the canonical source projects. This
repository gathers versioned snapshots of their sources into one library and
one public envelope header:

```c
#include <mulle-core/mulle-core.h>
```

The purpose of the amalgamation is packaging and consumption convenience: a
consumer can compile and link one library instead of integrating many
individual projects, while constituent include paths and APIs remain
available.

## Repository roles

There are three different roles in this project family:

1. **Constituent projects** own the individual implementations, their APIs,
   behavioral tests, platform-specific test matrices, and normal development
   history.
2. **This amalgamation** assembles versioned constituent sources, headers,
   envelope headers, documentation indexes, and build metadata into a
   consumable distribution.
3. **Consumer projects** use the assembled distribution as a Git submodule or
   another pinned source dependency, normally through CMake.

A source change should normally be made in the relevant constituent project,
not directly in a copied source file here. Direct changes to amalgamated source
files will be overwritten by the next amalgamation.

## Amalgamation lifecycle

The normal development lifecycle is:

1. Constituent projects are developed independently.
2. When a project is built amalgamation is automatically triggered through a mulle-sde hook
3. At release time, the projects are versioned, tested and committedd first and then mulle-core is re-amalgamated
5. The amalgamated snapshot is built and packaged.
6. The snapshot and its constituent revisions are committed and tagged so a
   consumer can pin a known distribution state.

The amalgamated tree is therefore a release artifact, not a second independent
implementation of every constituent. A working tree may legitimately contain
freshly copied files that have not yet been committed; that is a release
workflow state, not evidence that the constituent source is being maintained in
two places.

## Testing and CI ownership

Behavioral testing belongs primarily to the constituent projects. Their test
suites and CI exercise the implementation where it is developed, including
container, allocation, concurrency, portability, sanitizer, and other
constituent-specific coverage.

The amalgamation does not duplicate all of those test suites. Its CI and
release checks have a different responsibility:

- confirm that the selected snapshot compiles across the supported toolchains;
- confirm that the combined library links correctly;
- verify the envelope headers and generated manifests;
- exercise the supported consumer integration, especially Git submodule plus
  CMake `add_subdirectory` consumption; and
- catch accidental source, header, or version skew introduced while creating a
  snapshot.

A release may additionally run selected constituent tests against the
amalgamated snapshot as a consistency check. That is useful defense in depth,
but the absence of duplicated constituent test suites in this distribution is
intentional.

## Consumer contract

A consumer of a released mulle-core library should not need `mulle-sde`.
The supported source integration is a normal Git submodule and CMake project:

```sh
git submodule add https://github.com/mulle-core/mulle-core.git stash/mulle-core
git submodule update --init
```

```cmake
add_subdirectory(stash/mulle-core)
target_link_libraries(my-target PRIVATE mulle-core)
```

The public envelope can then be included as:

```c
#include <mulle-core/mulle-core.h>
```

The exact target and include layout are part of the consumer-facing interface.
The consumer should not have to run amalgamation, dependency fetching,
reflection, or any other mulle-sde operation merely to build the pinned
snapshot as a subdirectory.

## Contribution model

When fixing implementation behavior:

1. identify the constituent that owns the code;
2. make and test the change in that constituent project;
3. update the selected constituent revision here;
4. regenerate the amalgamation and its manifests; and
5. build the resulting snapshot through the standalone consumer path before
   committing the release state.

Documentation about how to consume the distribution belongs here. Detailed
implementation documentation and bug fixes belong in the constituent that
owns the implementation.
