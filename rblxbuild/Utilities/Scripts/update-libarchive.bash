#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="LibArchive"
readonly ownership="LibArchive Upstream <libarchive-discuss@googlegroups.com>"
readonly subtree="Utilities/cmlibarchive"
readonly repo="https://github.com/libarchive/libarchive.git"
readonly tag="v3.8.11"
readonly shortlog=false
readonly exact_tree_match=false
readonly paths="
  RBLXBuildLists.txt
  COPYING
  CTestConfig.rblxbuild
  build/rblxbuild
  build/pkgconfig
  build/utils
  build/version
  libarchive/*.*
"

extract_source () {
    git_archive
    pushd "${extractdir}/${name}-reduced"
    fromdos build/rblxbuild/Find*.rblxbuild
    echo "* -whitespace" > .gitattributes
    popd
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
