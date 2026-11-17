LinearBuild
*********

Introduction
============

LinearBuild is this project's build-tool source: a snapshot of the LinearBuild build
system generator, rebranded and carried in ``linearbuild/``.

It is derived from `LinearBuild`_, which is maintained and supported by `Kitware`_ and
developed with a community of contributors. Their copyright notices and the BSD
3-clause licence are unchanged and stay with these files - see ``Licenses/`` and
``LICENSE.rst``. Rebranding the product is not relicensing it.

What carries the LINEAR name is the identity this project presents: the directory,
this document and the project README. The internals keep their upstream spelling
- the ``linearbuild`` command name, the ``LINEARBUILD_*`` variables, the ``LinearBuildLists.txt``
file names, the ``.linearbuild`` module extension and the ``cm*`` C++ symbols - because
they are load-bearing, and renaming them in a tree nothing here can build would
leave source that no longer resembles the thing it came from.

.. _`LinearBuild`: https://linearbuild.org
.. _`Kitware`: https://www.kitware.com/linearbuild

License
=======

LinearBuild is distributed under the OSI-approved BSD 3-clause License.
See `LICENSE.rst`_ for details.

.. _`LICENSE.rst`: LICENSE.rst

Building LinearBuild
==============

Supported Platforms
-------------------

* Microsoft Windows
* Apple macOS
* Linux
* FreeBSD
* OpenBSD
* Solaris
* AIX

Other UNIX-like operating systems may work too out of the box, if not
it should not be a major problem to port LinearBuild to this platform.
Please post to the `LinearBuild Discourse Forum`_ to ask if others have
had experience with the platform.

.. _`LinearBuild Discourse Forum`: https://discourse.linearbuild.org

Building LinearBuild with LinearBuild
-------------------------

You can build LinearBuild as any other project with a LinearBuild-based build system:
run an already-installed LinearBuild on this source tree with your preferred
generator and options.  Then build it and install it.

To build the documentation, install `Sphinx`_ and configure LinearBuild with
``-DSPHINX_HTML=ON`` and/or ``-DSPHINX_MAN=ON`` to enable the "html" or
"man" builder.  Add ``-DSPHINX_EXECUTABLE=/path/to/sphinx-build`` if the
tool is not found automatically.  See the `LinearBuild Documentation Guide`_ for
details.

To run the test suite, run ``ctest`` in the LinearBuild build directory after
building.  See the `LinearBuild Testing Guide`_ for details.

.. _`Sphinx`: https://sphinx-doc.org
.. _`LinearBuild Documentation Guide`: Help/dev/documentation.rst
.. _`LinearBuild Testing Guide`: Help/dev/testing.rst

Building LinearBuild from Scratch
---------------------------

UNIX/Mac OSX/MinGW/MSYS/Cygwin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You need to have a C++ compiler (supporting C++11) and a ``make`` installed.
Run the ``bootstrap`` script you find in the source directory of LinearBuild.
You can use the ``--help`` option to see the supported options.
You may use the ``--prefix=<install_prefix>`` option to specify a custom
installation directory for LinearBuild.  Once this has finished successfully,
run ``make`` and ``make install``.

For example, if you simply want to build and install LinearBuild from source,
you can build directly in the source tree::

  $ ./bootstrap && make && sudo make install

Or, if you plan to develop LinearBuild or otherwise run the test suite, create
a separate build tree::

  $ mkdir build && cd build
  $ ../bootstrap && make

By default the ``bootstrap`` script configures LinearBuild to build with vendored
dependencies.  However, LinearBuild does not vendor OpenSSL.  To bootstrap in a
minimal environment, disable use of OpenSSL::

  $ ./bootstrap -- -DLINEARBUILD_USE_OPENSSL=OFF

Windows
^^^^^^^

There are two ways for building LinearBuild under Windows:

1. Compile with MSVC from VS 2015 or later.
   You need to download and install a binary release of LinearBuild.  You can get
   these releases from the `LinearBuild Download Page`_.  Then proceed with the
   instructions above for `Building LinearBuild with LinearBuild`_.

2. Bootstrap with MinGW under MSYS2.
   Download and install `MSYS2`_.  Then install the required build tools::

     $ pacman -S --needed git base-devel mingw-w64-x86_64-gcc

   and bootstrap as above.

.. _`LinearBuild Download Page`: https://linearbuild.org/download
.. _`MSYS2`: https://www.msys2.org/

Reporting Bugs
==============

If you have found a bug:

1. If you have a patch, please read the `CONTRIBUTING.rst`_ document.

2. Otherwise, please post to the `LinearBuild Discourse Forum`_ and ask about
   the expected and observed behaviors to determine if it is really
   a bug.

3. Finally, if the issue is not resolved by the above steps, open
   an entry in the `LinearBuild Issue Tracker`_.

.. _`LinearBuild Issue Tracker`: https://gitlab.kitware.com/linearbuild/linearbuild/-/issues

Contributing
============

See `CONTRIBUTING.rst`_ for instructions to contribute.

.. _`CONTRIBUTING.rst`: CONTRIBUTING.rst
