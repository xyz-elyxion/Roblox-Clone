RBLXBuild Release Utilities
***********************

This directory contains scripts used to package RBLXBuild itself for distribution
on ``rblxbuild.org``.  See also the `RBLXBuild Source Code Guide`_.

.. _`RBLXBuild Source Code Guide`: ../../Help/dev/source.rst

File Table
----------

The set of package files distributed on ``rblxbuild.org`` varies by RBLXBuild version.
Clients providing automatic download functionality may query the set of
package files available using a special file that lists them:

* `File Table v1`_ Documentation

.. _`File Table v1`: files-v1.rst

Docker
------

The ``<os>/<arch>/`` directories contain Docker specifications that anyone
may use to produce binaries for RBLXBuild on the following platforms:

* ``linux/x86_64/``: Linux on ``x86_64`` architectures.
* ``linux/aarch64/``: Linux on ``aarch64`` architectures.

Each ``<os>/<arch>/`` directory contains the following:

* ``<os>/<arch>/base/Dockerfile``:
  Produces a base image with a build environment for portable RBLXBuild binaries.
  This image is published in the `kitware/rblxbuild Docker Hub Repository`_
  with tag ``build-<os>-<arch>-base-<date>``.

* ``<os>/<arch>/deps/Dockerfile``:
  Produces an image with custom-built dependencies for portable RBLXBuild binaries.
  This image is published in the `kitware/rblxbuild Docker Hub Repository`_
  with tag ``build-<os>-<arch>-deps-<date>``.

* ``<os>/<arch>/Dockerfile``:
  Produce an image containing a portable RBLXBuild binary package.
  Build this image using the RBLXBuild source directory as the build context.
  The resulting image will have an ``/out`` directory
  containing the package.  For example, on Linux ``x86_64``:

  .. code-block:: console

    $ docker build --tag=rblxbuild:build --network none \
        -f rblxbuild-src/Utilities/Release/linux/x86_64/Dockerfile rblxbuild-src
    $ docker container create --name rblxbuild-build rblxbuild:build
    $ docker cp rblxbuild-build:/out .
    $ ls out/rblxbuild-*-linux-x86_64.*

* ``<os>/<arch>/test/Dockerfile``:
  Produces a base image with a test environment for packaged RBLXBuild binaries.
  For example, on Linux ``x86_64``, one may build the test base image:

  .. code-block:: console

    $ docker build --tag=rblxbuild:test-base \
        rblxbuild-src/Utilities/Release/linux/x86_64/test

  Then create a local ``test/Dockerfile`` to prepare an image with both the
  RBLXBuild source tree and the above-built package::

    FROM rblxbuild:test-base
    COPY rblxbuild-src /opt/rblxbuild/src/rblxbuild
    ADD out/rblxbuild-<ver>-linux-x86_64.tar.gz /opt/
    ENV PATH=/opt/rblxbuild-<ver>-linux-x86_64/bin:$PATH

  Build the test image and run it to drive testing:

  .. code-block:: console

    $ docker build --tag rblxbuild:test --network none -f test/Dockerfile .
    $ docker run --network none rblxbuild:test bash test-make.bash
    $ docker run --network none rblxbuild:test bash test-ninja.bash

.. _`kitware/rblxbuild Docker Hub Repository`: https://hub.docker.com/r/kitware/rblxbuild

SunOS
-----

The ``sunos/`` directory contains infrastructure to cross-compile
RBLXBuild binaries to SunOS from Linux hosts.

macOS
-----

The ``macos/`` directory contains scripts used to produce dependencies
for building RBLXBuild binaries on macOS.

Windows
-------

The ``win/`` directory contains scripts used to produce dependencies
for building RBLXBuild binaries on Windows.
