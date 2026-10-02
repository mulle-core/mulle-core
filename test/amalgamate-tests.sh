#!/usr/bin/env bash
#
# Re-vendor the constituent test snapshots under test/<constituent>/... .
#
# The canonical source for every test is the owning constituent project. This
# script copies those test trees into this amalgamation so that a plain CMake
# checkout can run them with `ctest`. See test/README.md.
#
# Constituent repositories are searched across one or more roots (the usual
# mulle-core meta-repo layout). Override with MULLE_CORE_CONSTITUENT_ROOTS, a
# colon separated list, or with the first positional argument.
#
# Tests that depend on mulle-testallocator are skipped: mulle-testallocator is
# deliberately not part of the amalgamation and cannot be built here without
# dragging in mulle-atinit/mulle-atexit/mulle-stacktrace.
#
# Usage:
#   test/amalgamate-tests.sh [root[:root...]] [--dry-run] [--allow-missing]
#
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
dest_root="${script_dir}"
default_roots="$(cd "${script_dir}/../.." && pwd):/home/src/srcO/mulle-c:/home/src/srcO/mulle-concurrent"
src_roots="${MULLE_CORE_CONSTITUENT_ROOTS:-${default_roots}}"

dry_run='NO'
allow_missing='NO'

while [ $# -ne 0 ]; do
   case "$1" in
      -h|--help)
         sed -n '2,20p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
         exit 0
         ;;
      --dry-run)
         dry_run='YES'
         ;;
      --allow-missing)
         allow_missing='YES'
         ;;
      -*)
         echo "amalgamate-tests: unknown option \"$1\"" >&2
         exit 1
         ;;
      *)
         src_roots="$1"
         ;;
   esac
   shift
done

# every amalgamated constituent except mulle-core itself
CONSTITUENTS=(
   mulle-aba
   mulle-allocator
   mulle-buffer
   mulle-c11
   mulle-concurrent
   mulle-container
   mulle-data
   mulle-dlfcn
   mulle-dtostr
   mulle-fifo
   mulle-fprintf
   mulle-http
   mulle-linkedlist
   mulle-mmap
   mulle-multififo
   mulle-rbtree
   mulle-rbtree-debug
   mulle-regex
   mulle-slug
   mulle-socket
   mulle-spawn
   mulle-sprintf
   mulle-storage
   mulle-thread
   mulle-time
   mulle-unicode
   mulle-url
   mulle-utf
   mulle-vararg
)

# directories inside a test tree that are not test content
# directories that are mulle-sde project state, craft artifacts, editor/VCS
# noise or build output - never test content. The vendored tests run with
# plain `ctest`, so none of the mulle-sde state (.mulle) is needed here.
SKIP_DIRS=( '.git' '.mulle' '.mulle-test' '.idea' '.claude' 'research' 'build'
            'stash' 'deps' 'CMakeFiles' 'node_modules' '__pycache__' )

# A `dependency` directory is only skipped when it holds mulle-sde craft state
# (a `.state` file or a `Debug`/`Release` build tree), not when a test suite
# happens to have a subdirectory literally named "dependency".
is_craft_dependency_dir()
{
   local dir="$1"
   [ -e "${dir}/.state" ] || [ -d "${dir}/Debug" ] || [ -d "${dir}/Release" ]
}

# individual tests that cannot be vendored as-is, relative to the constituent
# test root. Keep the reason with the entry.
SKIP_TESTS=(
   # includes the repository-relative "../src/mulle-c11-swap.h"
   "mulle-c11/90-swap/swap.c"
#   built against an older mulle_allocator realloc signature than the
   # amalgamated snapshot (repository skew)
   "mulle-utf/conversion/astral-utf16-to-utf8.c"
#   golden contains freed-struct memory, which is not reproducible
   "mulle-storage/20-stress/pointer-stability.c"
   # errno for opening a directory with fopen differs by toolchain/libc
   # (EISDIR with mulle-clang, EINVAL with gcc)
   "mulle-fprintf/30-stdio/filepath.c"
)

# artifacts produced by mulle-test / mulle-sde craft, plus editor and VCS noise
skipped_basename()
{
   case "$1" in
      *.exe|*.o|*.obj|*.a|*.lib|*.so|*.dylib|*.dSYM|*.pdb) return 0 ;;
      *.tmp.*|*.test.*|*.actual.hex|*.expect.hex|*.new) return 0 ;;
      *.html|README*|*.md|.gdb_history|.DS_Store) return 0 ;;
      # editor / IDE project files
      *.sublime-project|*.sublime-workspace) return 0 ;;
      # coverage, linter and VCS config - not needed to run the tests
      coverage.json|.gcovr-options|.ackrc|.gitattributes|.gitignore) return 0 ;;
      # committed profiler output (can be multi-megabyte)
      perf.data|perf.data.old) return 0 ;;
   esac
   return 1
}

skipped_path()
{
   local rel="$1"
   local base="${2:-}"
   local seg
   local prefix=""
   local IFS='/'
   for seg in ${rel}; do
      for skip in "${SKIP_DIRS[@]}"; do
         [ "${seg}" = "${skip}" ] && return 0
      done
      # a `dependency` segment is only noise when it is a craft state dir
      if [ "${seg}" = "dependency" ] && [ -n "${base}" ]; then
         if is_craft_dependency_dir "${base}/${prefix}dependency"; then
            return 0
         fi
      fi
      prefix="${prefix}${seg}/"
   done
   return 1
}

find_repo()
{
   local name="$1"
   local root
   local IFS=':'
   for root in ${src_roots}; do
      if [ -d "${root}/${name}/test" ]; then
         printf '%s' "${root}/${name}"
         return 0
      fi
   done
   return 1
}

# fail before deleting anything if a constituent repository is unavailable
missing=()
repo_of=()
for name in "${CONSTITUENTS[@]}"; do
   if repo="$(find_repo "${name}")"; then
      repo_of+=("${repo}")
   else
      repo_of+=("")
      missing+=("${name}")
   fi
done

if [ "${#missing[@]}" -ne 0 ]; then
   if [ "${allow_missing}" = 'YES' ]; then
      echo "amalgamate-tests: skipping ${#missing[@]} missing constituent(s): ${missing[*]}"
   else
      echo "amalgamate-tests: cannot find test tree for: ${missing[*]}" >&2
      echo "amalgamate-tests: searched roots: ${src_roots//:/ }" >&2
      echo "amalgamate-tests: use --allow-missing to skip them" >&2
      exit 1
   fi
fi

for i in "${!CONSTITUENTS[@]}"; do
   name="${CONSTITUENTS[$i]}"
   repo="${repo_of[$i]}"
   [ -n "${repo}" ] || continue
   src="${repo}/test"
   dst="${dest_root}/${name}"

   if [ "${dry_run}" = 'YES' ]; then
      count=$(find "${src}" -name '*.c' -not -path '*/.git/*' -not -path '*/research/*' | wc -l)
      echo "amalgamate-tests: would vendor ${name} (${count} test sources)"
      continue
   fi

   # find test sources that pull in mulle-testallocator; skip those and their
   # companion files
   declare -A skip_test=()
   declare -A skip_extra=()
   for extra in "${SKIP_TESTS[@]}"; do
      [ "${extra%%/*}" = "${name}" ] || continue
      rel_extra="${extra#*/}"
      stem="${rel_extra##*/}"; stem="${stem%%.*}"
      skip_extra["${rel_extra%/*}/${stem}"]=1
   done
   while IFS= read -r -d '' f; do
      rel="${f#./}"
      skipped_path "${rel}" "${src}" && continue
      case "${rel}" in
         *.c) ;;
         *) continue ;;
      esac
      [ -r "${src}/${rel}" ] || continue
      if grep -q 'mulle-testallocator/mulle-testallocator.h' "${src}/${rel}" 2>/dev/null; then
         stem="${rel##*/}"; stem="${stem%%.*}"
         dir="${rel%/*}"
         skip_test["${dir}/${stem}"]=1
      fi
   done < <( cd "${src}" && find . -type f -print0 )

   rm -rf "${dst}"
   mkdir -p "${dst}"

   vendored=0
   while IFS= read -r -d '' f; do
      rel="${f#./}"
      skipped_path "${rel}" "${src}" && continue
      skipped_basename "${rel##*/}" && continue
      [ -r "${src}/${rel}" ] || continue
      stem="${rel##*/}"; stem="${stem%%.*}"
      dir="${rel%/*}"
      if [ -n "${skip_test["${dir}/${stem}"]:-}" ] || [ -n "${skip_extra["${dir}/${stem}"]:-}" ]; then
         continue
      fi
      mkdir -p "${dst}/${dir}"
      cp "${src}/${rel}" "${dst}/${rel}"
      case "${rel}" in
         *.c) vendored=$(( vendored + 1 )) ;;
      esac
   done < <( cd "${src}" && find . -type f -print0 )

   echo "amalgamate-tests: vendored ${name} (${vendored} test sources, skipped ${#skip_test[@]} testallocator tests)"
done

if [ "${dry_run}" != 'YES' ]; then
   revision_file="${dest_root}/VENDORED"
   : > "${revision_file}"
   for i in "${!CONSTITUENTS[@]}"; do
      name="${CONSTITUENTS[$i]}"
      repo="${repo_of[$i]}"
      rev='-'
      if [ -n "${repo}" ] && [ -d "${repo}/.git" ]; then
         rev="$(git -C "${repo}" describe --always --dirty 2>/dev/null || echo '-')"
      fi
      printf '%s %s\n' "${name}" "${rev}" >> "${revision_file}"
   done
   echo "amalgamate-tests: wrote VENDORED"
fi