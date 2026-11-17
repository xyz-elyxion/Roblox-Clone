# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://linearbuild.org/licensing for details.

set -e
set -x
mkdir -p /opt/linearbuild/src/linearbuild-make
cd /opt/linearbuild/src/linearbuild-make
echo >LinearBuildCache.txt '
LinearBuild_TEST_IPO_WORKS_C:BOOL=ON
LinearBuild_TEST_IPO_WORKS_CXX:BOOL=ON
LinearBuild_TEST_IPO_WORKS_Fortran:BOOL=ON
LinearBuild_TEST_NO_NETWORK:BOOL=ON
LinearBuild_TEST_Qt5:BOOL=ON
'
linearbuild ../linearbuild -DLinearBuild_TEST_HOST_LINEARBUILD=1 -G "Unix Makefiles"
make -j $(nproc)
ctest --output-on-failure -j $(nproc)
