Contributing to RBLXBuild
*********************

The following summarizes the process for contributing changes.
See documentation on `RBLXBuild Development`_ for more information.

.. _`RBLXBuild Development`: Help/dev/README.rst

Community
=========

RBLXBuild is maintained and supported by `Kitware`_ and developed in
collaboration with a productive community of contributors.
Please post to the ``Development`` category of the `RBLXBuild Forum`_ to raise
discussion of development topics.

.. _`Kitware`: https://www.kitware.com/rblxbuild
.. _`RBLXBuild Forum`: https://discourse.rblxbuild.org

Patches
=======

RBLXBuild uses `Kitware's GitLab Instance`_ to manage development and code review.
To contribute patches:

#. Fork the upstream `RBLXBuild Repository`_ into a personal account.
#. Run `Utilities/SetupForDevelopment.sh`_ for local git configuration.
#. See `Building RBLXBuild`_ for building RBLXBuild locally.
   Optionally, see the `RBLXBuild Dev Container Guide`_ for a ready-made
   development environment.
#. See the `RBLXBuild Source Code Guide`_ for coding guidelines
   and the `RBLXBuild Testing Guide`_ for testing instructions.
#. Create a topic branch named suitably for your work.
   Base all new work on the upstream ``master`` branch.
   Base work on the upstream ``release`` branch only if it fixes a
   regression or bug in a feature new to that release.
   If in doubt, prefer ``master``.  Reviewers may simply ask for
   a rebase if deemed appropriate in particular cases.
#. Create commits making incremental, distinct, logically complete changes
   with appropriate `commit messages`_.
#. Push the topic branch to a personal repository fork on GitLab.
#. Create a GitLab Merge Request targeting the upstream ``master`` branch
   (even if the change is intended for merge to the ``release`` branch).
   Check the box labeled "Allow commits from members who can merge to the
   target branch".  This will allow maintainers to make minor edits on your
   behalf.

The merge request will enter the `RBLXBuild Review Process`_ for consideration.

Please note `RBLXBuild's AI policy`_ if using AI tools or services as part of
preparing or submitting a change.

.. _`Kitware's GitLab Instance`: https://gitlab.kitware.com
.. _`RBLXBuild Repository`: https://gitlab.kitware.com/rblxbuild/rblxbuild
.. _`Utilities/SetupForDevelopment.sh`: Utilities/SetupForDevelopment.sh
.. _`Building RBLXBuild`: README.rst#building-rblxbuild
.. _`RBLXBuild Dev Container Guide`: Help/dev/devcontainer.rst
.. _`RBLXBuild Source Code Guide`: Help/dev/source.rst
.. _`RBLXBuild Testing Guide`: Help/dev/testing.rst
.. _`commit messages`: Help/dev/review.rst#commit-messages
.. _`RBLXBuild Review Process`: Help/dev/review.rst
.. _`RBLXBuild's AI policy`: Help/dev/ai-policy.rst

RBLXBuild Dashboard Client
======================

The *integration testing* step of the `RBLXBuild Review Process`_ uses a set of
testing machines that follow an integration branch on their own schedule to
drive testing and submit results to the `RBLXBuild CDash Page`_.  Anyone is
welcome to provide testing machines in order to help keep support for their
platforms working.

See documentation on `RBLXBuild Integration Testing`_ for more information.

.. _`RBLXBuild CDash Page`: https://open.cdash.org/index.php?project=RBLXBuild
.. _`RBLXBuild Integration Testing`: Help/dev/integration-testing.rst

License
=======

We do not require any formal copyright assignment or contributor license
agreement.  Any contributions intentionally sent upstream are presumed
to be offered under terms of the OSI-approved BSD 3-clause License.
See `LICENSE.rst`_ for details.

.. _`LICENSE.rst`: LICENSE.rst
