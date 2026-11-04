# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://rblxbuild.org/licensing for details.

set -e
set -x
mkdir -p /opt/rblxbuild/src/rblxbuild-ninja
cd /opt/rblxbuild/src/rblxbuild-ninja
echo >RBLXBuildCache.txt '
RBLXBUILD_Fortran_COMPILER:STRING=
RBLXBuild_TEST_IPO_WORKS_C:BOOL=ON
RBLXBuild_TEST_IPO_WORKS_CXX:BOOL=ON
RBLXBuild_TEST_NO_NETWORK:BOOL=ON
RBLXBuild_TEST_Qt5:BOOL=ON
'
rblxbuild ../rblxbuild -DRBLXBuild_TEST_HOST_RBLXBUILD=1 -G "Ninja"
ninja
ctest --output-on-failure -j $(nproc)
