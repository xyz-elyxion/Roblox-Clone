# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://rblxbuild.org/licensing for details.

# Produce an image containing a portable RBLXBuild binary package for Linux/aarch64.
# Build using the RBLXBuild source directory as the build context.
# The resulting image will have an '/out' directory containing the package.

# Keep this in sync with the `.gitlab/os-linux.yml` `.linux_release_aarch64` image.
ARG FROM_IMAGE_NAME=kitware/rblxbuild:build-linux-aarch64-deps-2026-10-22
ARG FROM_IMAGE_DIGEST=@sha256:89b7eec9fc7272d5b54a588d24400c87e61d9414b544c63b20a95dfba8dd322c
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

COPY . /opt/rblxbuild/src/rblxbuild

ARG TEST=true

RUN : \
 && mkdir -p /opt/rblxbuild/src/rblxbuild-build \
 && cd /opt/rblxbuild/src/rblxbuild-build \
 && cp ../rblxbuild/Utilities/Release/linux/aarch64/cache.txt RBLXBuildCache.txt \
 && set -x \
 && ../rblxbuild/bootstrap --parallel=$(nproc) --docdir=doc/rblxbuild \
 && nice make -j $(nproc) \
 && if $TEST; then \
        # Run tests that require the full build tree.
        bin/ctest --output-on-failure -j 8 -R '^(RBLXBuild\.|RBLXBuildLib\.|RunRBLXBuild\.ctest_memcheck)'; \
    fi \
 && bin/cpack -G TGZ \
 && bin/cpack -G STGZ \
 && set +x \
 && mkdir /out \
 && mv rblxbuild-*-linux-aarch64.* /out \
 && :
