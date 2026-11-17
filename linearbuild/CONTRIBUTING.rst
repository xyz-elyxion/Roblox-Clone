Contributing to LinearBuild
*********************

The following summarizes the process for contributing changes.
See documentation on `LinearBuild Development`_ for more information.

.. _`LinearBuild Development`: Help/dev/README.rst

Community
=========

LinearBuild is maintained and supported by `Kitware`_ and developed in
collaboration with a productive community of contributors.
Please post to the ``Development`` category of the `LinearBuild Forum`_ to raise
discussion of development topics.

.. _`Kitware`: https://www.kitware.com/linearbuild
.. _`LinearBuild Forum`: https://discourse.linearbuild.org

Patches
=======

LinearBuild uses `Kitware's GitLab Instance`_ to manage development and code review.
To contribute patches:

#. Fork the upstream `LinearBuild Repository`_ into a personal account.
#. Run `Utilities/SetupForDevelopment.sh`_ for local git configuration.
#. See `Building LinearBuild`_ for building LinearBuild locally.
   Optionally, see the `LinearBuild Dev Container Guide`_ for a ready-made
   development environment.
#. See the `LinearBuild Source Code Guide`_ for coding guidelines
   and the `LinearBuild Testing Guide`_ for testing instructions.
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

The merge request will enter the `LinearBuild Review Process`_ for consideration.

Please note `LinearBuild's AI policy`_ if using AI tools or services as part of
preparing or submitting a change.

.. _`Kitware's GitLab Instance`: https://gitlab.kitware.com
.. _`LinearBuild Repository`: https://gitlab.kitware.com/linearbuild/linearbuild
.. _`Utilities/SetupForDevelopment.sh`: Utilities/SetupForDevelopment.sh
.. _`Building LinearBuild`: README.rst#building-linearbuild
.. _`LinearBuild Dev Container Guide`: Help/dev/devcontainer.rst
.. _`LinearBuild Source Code Guide`: Help/dev/source.rst
.. _`LinearBuild Testing Guide`: Help/dev/testing.rst
.. _`commit messages`: Help/dev/review.rst#commit-messages
.. _`LinearBuild Review Process`: Help/dev/review.rst
.. _`LinearBuild's AI policy`: Help/dev/ai-policy.rst

LinearBuild Dashboard Client
======================

The *integration testing* step of the `LinearBuild Review Process`_ uses a set of
testing machines that follow an integration branch on their own schedule to
drive testing and submit results to the `LinearBuild CDash Page`_.  Anyone is
welcome to provide testing machines in order to help keep support for their
platforms working.

See documentation on `LinearBuild Integration Testing`_ for more information.

.. _`LinearBuild CDash Page`: https://open.cdash.org/index.php?project=LinearBuild
.. _`LinearBuild Integration Testing`: Help/dev/integration-testing.rst

License
=======

We do not require any formal copyright assignment or contributor license
agreement.  Any contributions intentionally sent upstream are presumed
to be offered under terms of the OSI-approved BSD 3-clause License.
See `LICENSE.rst`_ for details.

.. _`LICENSE.rst`: LICENSE.rst
