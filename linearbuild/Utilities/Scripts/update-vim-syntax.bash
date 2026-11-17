#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="vim-linearbuild-syntax"
readonly ownership="vim-linearbuild-syntax upstream <kwrobot@kitware.com>"
readonly subtree="Auxiliary/vim"
readonly repo="https://github.com/pboettch/vim-linearbuild-syntax.git"
readonly tag="master"
readonly shortlog=true
readonly exact_tree_match=false
readonly paths="
  indent
  syntax
  linearbuild.vim.in
  extract-upper-case.pl
"

extract_source () {
    git_archive
}

. "${BASH_SOURCE%/*}/update-third-party.bash"
