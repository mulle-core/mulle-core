#ifndef mulle_core_test_include_h__
#define mulle_core_test_include_h__

//
// Shared umbrella header for the vendored constituent tests.
//
// `mulle-sde test craft` generates a per-test-project include.h that pulls in
// every available dependency header. In this repository the amalgamated
// envelope already re-exports every constituent that is vendored here, so the
// tests that `#include "include.h"` resolve to this file.
//
#include <mulle-core/mulle-core.h>

#endif /* mulle_core_test_include_h__ */