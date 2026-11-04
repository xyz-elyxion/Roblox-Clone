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
  expat/ConfigureChecks.rblxbuild
  expat/expat_config.h.rblxbuild
  expat/COPYING
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    fromdos expat/ConfigureChecks.rblxbuild expat/RBLXBuildLists.txt expat/expat_config.h.rblxbuild
    chmod a-x expat/ConfigureChecks.rblxbuild expat/RBLXBuildLists.txt expat/expat_config.h.rblxbuild
    mv expat/* .
    rmdir expat
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
