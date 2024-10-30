#!/bin/bash

NAME="$(basename -- $PWD)"
SRCDIR="${3:-src}"
INFILE="${SRCDIR}/${1:-${NAME}.h}.in"
OUTFILE="${2:-${INFILE%.in}}"

declare -A processed_files

#
# This script will not handle this well:
#
# #if APPLE
# # include <foo.h>
# #else
# # include <foo.h>   // won't be emitted
# # include <bar.h>
# #endif
#
# Unexpectedly a "precompiled header" will at least
# with gcc or clang not do anything for compile times.
# There was no measurable difference when building
# MulleFoundationBase.
#
strip_leading_comments()
{
    local in_comment_block=0

    while IFS= read -r line; do
        # Skip // comments
        if [[ $line =~ ^[[:space:]]*\/\/ ]]; then
            continue
        fi

#        # Handle /* */ blocks
#        if [ $in_comment_block -eq 1 ]; then
#            if [[ $line =~ \*/ ]]; then
#                in_comment_block=0
#                # Output everything after */ if present
#                echo "${line#*\*/}"
#            fi
#            continue
#        fi
#
#        # Check for start of comment block
#        if [[ $line =~ ^[[:space:]]*\/\* ]]; then
#            in_comment_block=1
#            continue
#        fi

        # Skip empty lines
        if [[ -z "${line// }" ]]; then
            echo "$line"
            continue
        fi
#
#        # skip in comment stuff
#        if [ $in_comment_block -eq 1 ]; then
#            continue
#        fi

        echo "$line"
    done
}


process_header()
{
    local file="$1"
    local base_dir="$2"

    local dir_path=$(dirname "$file")

    [ -n "${processed_files[$file]}" ] && return
    processed_files[$file]=1

    local namespace

    namespace="$(basename -- "$dir_path")"

    regex='^[[:space:]]*#[[:space:]]*(include|import)[[:space:]]*[\"<]([^>\"]+)[\">]'
    while IFS= read -r line; do
        if [[ ! $line =~ $regex ]]; then
            echo "$line"
            continue
        fi

        header="${BASH_REMATCH[2]}"
        # Handle relative includes
        if [[ $line =~ ^[[:space:]]*#[[:space:]]*(include|import)[[:space:]]*\"([^\"]+)\" ]]
        then
            potential_path=$(find "$dir_path" -name "$header" -type f | head -n 1)
            if [ -z "$potential_path" ]
            then
                echo "// mulle-create-precompiled-header.sh: Could not find '${header}' in '${dir_path}'\""
                echo "$line"
                continue
            fi
            tracking_id="$namespace:$header"
        else
            potential_path="${base_dir}/${header}"
            tracking_id=":$header"

            if [ ! -f "${potential_path}" ]
            then
               # got something like #include <stddef.h> we don't emit it again
               processed_files[$tracking_id]=1
               echo "$line"
               continue
            fi
        fi

        if [ -z "${processed_files[$tracking_id]}" ]
        then
            echo "/* Start of contents of $header */"
            processed_files[$tracking_id]=1
            ( cat "${potential_path}" ; echo ) | strip_leading_comments | process_header "${potential_path}" "${base_dir}"
            echo "/* End of contents of $header */"
        fi
    done
}

if [ "${INFILE}" = "${OUTFILE}" ]
then
   echo "INFILE and OUTFILE are the same \"${INFILE}\"" >&2
   exit 1
fi

if [ ! -f "${INFILE}" ]
then
   echo "INFILE \"${INFILE}\" does not exist" >&2
   exit 1
fi

( cat "${INFILE}" ; echo ) | process_header "${INFILE}" "${SRCDIR}" > "${OUTFILE}"
