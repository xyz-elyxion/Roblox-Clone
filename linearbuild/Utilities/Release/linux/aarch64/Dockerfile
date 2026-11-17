# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://linearbuild.org/licensing for details.

# Produce an image containing a portable LinearBuild binary package for Linux/aarch64.
# Build using the LinearBuild source directory as the build context.
# The resulting image will have an '/out' directory containing the package.

# Keep this in sync with the `.gitlab/os-linux.yml` `.linux_release_aarch64` image.
ARG FROM_IMAGE_NAME=kitware/linearbuild:build-linux-aarch64-deps-2026-10-22
ARG FROM_IMAGE_DIGEST=@sha256:89b7eec9fc7272d5b54a588d24400c87e61d9414b544c63b20a95dfba8dd322c
ARG FROM_IMAGE=$FROM_IMAGE_NAME$FROM_IMAGE_DIGEST
FROM $FROM_IMAGE

COPY . /opt/linearbuild/src/linearbuild

ARG TEST=true

RUN : \
 && mkdir -p /opt/linearbuild/src/linearbuild-build \
 && cd /opt/linearbuild/src/linearbuild-build \
 && cp ../linearbuild/Utilities/Release/linux/aarch64/cache.txt LinearBuildCache.txt \
 && set -x \
 && ../linearbuild/bootstrap --parallel=$(nproc) --docdir=doc/linearbuild \
 && nice make -j $(nproc) \
 && if $TEST; then \
        # Run tests that require the full build tree.
        bin/ctest --output-on-failure -j 8 -R '^(LinearBuild\.|LinearBuildLib\.|RunLinearBuild\.ctest_memcheck)'; \
    fi \
 && bin/cpack -G TGZ \
 && bin/cpack -G STGZ \
 && set +x \
 && mkdir /out \
 && mv linearbuild-*-linux-aarch64.* /out \
 && :
