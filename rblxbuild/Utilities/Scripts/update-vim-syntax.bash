#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="vim-rblxbuild-syntax"
readonly ownership="vim-rblxbuild-syntax upstream <kwrobot@kitware.com>"
readonly subtree="Auxiliary/vim"
readonly repo="https://github.com/pboettch/vim-rblxbuild-syntax.git"
readonly tag="master"
readonly shortlog=true
readonly exact_tree_match=false
readonly paths="
  indent
  syntax
  rblxbuild.vim.in
  extract-upper-case.pl
"

extract_source () {
    git_archive
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
