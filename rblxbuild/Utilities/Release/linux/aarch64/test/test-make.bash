# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://rblxbuild.org/licensing for details.

set -e
set -x
mkdir -p /opt/rblxbuild/src/rblxbuild-make
cd /opt/rblxbuild/src/rblxbuild-make
echo >RBLXBuildCache.txt '
RBLXBuild_TEST_IPO_WORKS_C:BOOL=ON
RBLXBuild_TEST_IPO_WORKS_CXX:BOOL=ON
RBLXBuild_TEST_IPO_WORKS_Fortran:BOOL=ON
RBLXBuild_TEST_NO_NETWORK:BOOL=ON
RBLXBuild_TEST_Qt5:BOOL=ON
'
rblxbuild ../rblxbuild -DRBLXBuild_TEST_HOST_RBLXBUILD=1 -G "Unix Makefiles"
make -j $(nproc)
ctest --output-on-failure -j $(nproc)
