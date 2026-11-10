#!/bin/bash -eu
# This script is meant to be run by
# https://github.com/google/oss-fuzz/blob/master/projects/rblxstb/Dockerfile

$CXX $CXXFLAGS -std=c++11 -I. -DSTBI_ONLY_PNG  \
    $SRC/rblxstb/tests/rblxstbi_read_fuzzer.c \
    -o $OUT/rblxstb_png_read_fuzzer $LIB_FUZZING_ENGINE

$CXX $CXXFLAGS -std=c++11 -I. \
    $SRC/rblxstb/tests/rblxstbi_read_fuzzer.c \
    -o $OUT/rblxstbi_read_fuzzer $LIB_FUZZING_ENGINE

find $SRC/rblxstb/tests/pngsuite -name "*.png" | \
     xargs zip $OUT/rblxstb_png_read_fuzzer_seed_corpus.zip

cp $SRC/rblxstb/tests/rblxstb_png.dict $OUT/rblxstb_png_read_fuzzer.dict

tar xvzf $SRC/rblxstbi/jpg.tar.gz --directory $SRC/rblxstb/tests
tar xvzf $SRC/rblxstbi/gif.tar.gz --directory $SRC/rblxstb/tests
unzip    $SRC/rblxstbi/bmp.zip    -d $SRC/rblxstb/tests
unzip    $SRC/rblxstbi/tga.zip    -d $SRC/rblxstb/tests

find $SRC/rblxstb/tests -name "*.png" -o -name "*.jpg" -o -name "*.gif" \
                 -o -name "*.bmp" -o -name "*.tga" -o -name "*.TGA" \
                 -o -name "*.ppm" -o -name "*.pgm" \
    | xargs zip $OUT/rblxstbi_read_fuzzer_seed_corpus.zip

echo "" >> $SRC/rblxstbi/gif.dict
cat $SRC/rblxstbi/gif.dict $SRC/rblxstb/tests/rblxstb_png.dict > $OUT/rblxstbi_read_fuzzer.dict
