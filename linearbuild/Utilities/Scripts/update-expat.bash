#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="expat"
readonly ownership="Expat Upstream <kwrobot@kitware.com>"
readonly subtree="Utilities/cmexpat"
readonly repo="https://github.com/libexpat/libexpat.git"
readonly tag="R_2_10_0"
readonly shortlog=false
readonly exact_tree_match=false
readonly paths="
  expat/lib/*.h
  expat/lib/*.c
  expat/README.md
  expat/ConfigureChecks.linearbuild
  expat/expat_config.h.linearbuild
  expat/COPYING
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    fromdos expat/ConfigureChecks.linearbuild expat/LinearBuildLists.txt expat/expat_config.h.linearbuild
    chmod a-x expat/ConfigureChecks.linearbuild expat/LinearBuildLists.txt expat/expat_config.h.linearbuild
    mv expat/* .
    rmdir expat
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
