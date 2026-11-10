/********************************************************************************
* RblxPhysics physics library, http://www.rblxphysics.com                 *
* Copyright (c) 2010-2016 Daniel Chappuis                                       *
*********************************************************************************
*                                                                               *
* This software is provided 'as-is', without any express or implied warranty.   *
* In no event will the authors be held liable for any damages arising from the  *
* use of this software.                                                         *
*                                                                               *
* Permission is granted to anyone to use this software for any purpose,         *
* including commercial applications, and to alter it and redistribute it        *
* freely, subject to the following restrictions:                                *
*                                                                               *
* 1. The origin of this software must not be misrepresented; you must not claim *
*    that you wrote the original software. If you use this software in a        *
*    product, an acknowledgment in the product documentation would be           *
*    appreciated but is not required.                                           *
*                                                                               *
* 2. Altered source versions must be plainly marked as such, and must not be    *
*    misrepresented as being the original software.                             *
*                                                                               *
* 3. This notice may not be removed or altered from any source distribution.    *
*                                                                               *
********************************************************************************/

#ifndef TEST_SET_H
#define TEST_SET_H

// Libraries
#include "Test.h"
#include <rblxphysics/containers/Set.h>
#include <rblxphysics/memory/DefaultAllocator.h>

// Key to test map with always same hash values
namespace rblxphysics {
    struct TestValueSet {
        int key;

        TestValueSet(int k) :key(k) {}

        bool operator==(const TestValueSet& testValue) const {
            return key == testValue.key;
        }
    };
}

// Hash function for struct VerticesPair
namespace std {

  template <> struct hash<rblxphysics::TestValueSet> {

    size_t operator()(const rblxphysics::TestValueSet& /*value*/) const {
        return 1;
    }
  };
}

/// Reactphysics3D namespace
namespace rblxphysics {

// Class TestSet
/**
 * Unit test for the Set class
 */
class TestSet : public Test {

    private :

        // ---------- Atributes ---------- //

        DefaultAllocator mAllocator;

    public :

        // ---------- Methods ---------- //

        /// Constructor
        TestSet(const std::string& name) : Test(name) {

        }

        /// Run the tests
        void run() {

            testConstructors();
            testReserve();
            testAddRemoveClear();
            testContains();
            testFind();
            testEquality();
            testAssignment();
            testIterators();
            testConverters();
        }

        void testConstructors() {

            // ----- Constructors ----- //

            Set<std::string> set1(mAllocator);
            rblxphysics_test(set1.capacity() == 0);
            rblxphysics_test(set1.size() == 0);

            Set<std::string> set2(mAllocator, 100);
            rblxphysics_test(set2.capacity() >= 100);
            rblxphysics_test(set2.size() == 0);

            // ----- Copy Constructors ----- //
            Set<std::string> set3(set1);
            rblxphysics_test(set3.capacity() == set1.capacity());
            rblxphysics_test(set3.size() == set1.size());

            Set<int> set4(mAllocator);
            set4.add(10);
            set4.add(20);
            set4.add(30);
            rblxphysics_test(set4.capacity() >= 3);
            rblxphysics_test(set4.size() == 3);
            set4.add(30);
            rblxphysics_test(set4.size() == 3);

            Set<int> set5(set4);
            rblxphysics_test(set5.capacity() == set4.capacity());
            rblxphysics_test(set5.size() == set4.size());
            rblxphysics_test(set5.contains(10));
            rblxphysics_test(set5.contains(20));
            rblxphysics_test(set5.contains(30));
        }

        void testReserve() {

            Set<std::string> set1(mAllocator);
            set1.reserve(15);
            rblxphysics_test(set1.capacity() >= 15);
            set1.add("test1");
            set1.add("test2");
            rblxphysics_test(set1.capacity() >= 15);

            set1.reserve(10);
            rblxphysics_test(set1.capacity() >= 15);

            set1.reserve(100);
            rblxphysics_test(set1.capacity() >= 100);
            rblxphysics_test(set1.contains("test1"));
            rblxphysics_test(set1.contains("test2"));
        }

        void testAddRemoveClear() {

            // ----- Test add() ----- //

            Set<int> set1(mAllocator);
            bool add1 = set1.add(10);
            bool add2 = set1.add(80);
            bool add3 = set1.add(130);
            rblxphysics_test(add1);
            rblxphysics_test(add2);
            rblxphysics_test(add3);
            rblxphysics_test(set1.contains(10));
            rblxphysics_test(set1.contains(80));
            rblxphysics_test(set1.contains(130));
            rblxphysics_test(set1.size() == 3);

            bool add4 = set1.add(80);
            rblxphysics_test(!add4);
            rblxphysics_test(set1.contains(80));
            rblxphysics_test(set1.size() == 3);

            Set<int> set2(mAllocator, 15);
            for (int i = 0; i < 1000000; i++) {
                set2.add(i);
            }
            bool isValid = true;
            for (int i = 0; i < 1000000; i++) {
                if (!set2.contains(i)) isValid = false;
            }
            rblxphysics_test(isValid);

            set1.remove(10);
            bool add = set1.add(10);
            rblxphysics_test(add);
            rblxphysics_test(set1.size() == 3);
            rblxphysics_test(set1.contains(10));

            set1.add(34);
            rblxphysics_test(set1.contains(34));
            rblxphysics_test(set1.size() == 4);

            // ----- Test remove() ----- //

            set1.remove(10);
            rblxphysics_test(!set1.contains(10));
            rblxphysics_test(set1.contains(80));
            rblxphysics_test(set1.contains(130));
            rblxphysics_test(set1.contains(34));
            rblxphysics_test(set1.size() == 3);

            set1.remove(80);
            rblxphysics_test(!set1.contains(80));
            rblxphysics_test(set1.contains(130));
            rblxphysics_test(set1.contains(34));
            rblxphysics_test(set1.size() == 2);

            set1.remove(130);
            rblxphysics_test(!set1.contains(130));
            rblxphysics_test(set1.contains(34));
            rblxphysics_test(set1.size() == 1);

            set1.remove(34);
            rblxphysics_test(!set1.contains(34));
            rblxphysics_test(set1.size() == 0);

            isValid = true;
            for (int i = 0; i < 1000000; i++) {
                set2.remove(i);
            }
            for (int i = 0; i < 1000000; i++) {
                if (set2.contains(i)) isValid = false;
            }
            rblxphysics_test(isValid);
            rblxphysics_test(set2.size() == 0);

            Set<int> set3(mAllocator);
            for (int i=0; i < 1000000; i++) {
                set3.add(i);
                set3.remove(i);
            }

            set3.add(1);
            set3.add(2);
            set3.add(3);
            rblxphysics_test(set3.size() == 3);
            auto it = set3.begin();
            it = set3.remove(it);
            rblxphysics_test(set3.size() == 2);
            it = set3.remove(it);
            rblxphysics_test(set3.size() == 1);
            it = set3.remove(it);
            rblxphysics_test(set3.size() == 0);

            set3.add(6);
            set3.add(7);
            set3.add(8);
            for (it = set3.begin(); it != set3.end();) {
               it = set3.remove(it);
            }
            rblxphysics_test(set3.size() == 0);

            // ----- Test clear() ----- //

            Set<int> set4(mAllocator);
            set4.add(2);
            set4.add(4);
            set4.add(6);
            set4.clear();
            rblxphysics_test(set4.size() == 0);
            set4.add(2);
            rblxphysics_test(set4.size() == 1);
            rblxphysics_test(set4.contains(2));
            set4.clear();
            rblxphysics_test(set4.size() == 0);

            Set<int> set5(mAllocator);
            set5.clear();
            rblxphysics_test(set5.size() == 0);

            // ----- Test map with always same hash value for keys ----- //

            Set<TestValueSet> set6(mAllocator);
            for (int i=0; i < 1000; i++) {
                set6.add(TestValueSet(i));
            }
            bool isTestValid = true;
            for (int i=0; i < 1000; i++) {
                if (!set6.contains(TestValueSet(i))) {
                    isTestValid = false;
                }
            }
            rblxphysics_test(isTestValid);
            for (int i=0; i < 1000; i++) {
                set6.remove(TestValueSet(i));
            }
            rblxphysics_test(set6.size() == 0);
        }

        void testContains() {

            Set<int> set1(mAllocator);

            rblxphysics_test(!set1.contains(2));
            rblxphysics_test(!set1.contains(4));
            rblxphysics_test(!set1.contains(6));

            set1.add(2);
            set1.add(4);
            set1.add(6);

            rblxphysics_test(set1.contains(2));
            rblxphysics_test(set1.contains(4));
            rblxphysics_test(set1.contains(6));

            set1.remove(4);
            rblxphysics_test(!set1.contains(4));
            rblxphysics_test(set1.contains(2));
            rblxphysics_test(set1.contains(6));

            set1.clear();
            rblxphysics_test(!set1.contains(2));
            rblxphysics_test(!set1.contains(6));
        }

        void testFind() {

            Set<int> set1(mAllocator);
            set1.add(2);
            set1.add(4);
            set1.add(6);
            rblxphysics_test(set1.find(2) != set1.end());
            rblxphysics_test(set1.find(4) != set1.end());
            rblxphysics_test(set1.find(6) != set1.end());
            rblxphysics_test(set1.find(45) == set1.end());

            set1.remove(2);

            rblxphysics_test(set1.find(2) == set1.end());
        }

        void testEquality() {

            Set<std::string> set1(mAllocator, 10);
            Set<std::string> set2(mAllocator, 2);

            rblxphysics_test(set1 == set2);

            set1.add("a");
            set1.add("b");
            set1.add("c");

            set2.add("a");
            set2.add("b");
            set2.add("h");

            rblxphysics_test(set1 == set1);
            rblxphysics_test(set2 == set2);
            rblxphysics_test(set1 != set2);
            rblxphysics_test(set2 != set1);

            set1.add("a");
            set2.remove("h");
            set2.add("c");

            rblxphysics_test(set1 == set2);
            rblxphysics_test(set2 == set1);

            Set<std::string> set3(mAllocator);
            set3.add("a");

            rblxphysics_test(set1 != set3);
            rblxphysics_test(set2 != set3);
            rblxphysics_test(set3 != set1);
            rblxphysics_test(set3 != set2);
        }

        void testAssignment() {

           Set<int> set1(mAllocator);
           set1.add(1);
           set1.add(2);
           set1.add(10);

           Set<int> set2(mAllocator);
           set2 = set1;
           rblxphysics_test(set2.size() == set1.size());
           rblxphysics_test(set2.contains(1));
           rblxphysics_test(set2.contains(2));
           rblxphysics_test(set2.contains(10));
           rblxphysics_test(set1 == set2);

           Set<int> set3(mAllocator, 100);
           set3 = set1;
           rblxphysics_test(set3.size() == set1.size());
           rblxphysics_test(set3 == set1);
           rblxphysics_test(set3.contains(1));
           rblxphysics_test(set3.contains(2));
           rblxphysics_test(set3.contains(10));

           Set<int> set4(mAllocator);
           set3 = set4;
           rblxphysics_test(set3.size() == 0);
           rblxphysics_test(set3 == set4);

           Set<int> set5(mAllocator);
           set5.add(7);
           set5.add(19);
           set1 = set5;
           rblxphysics_test(set5.size() == set1.size());
           rblxphysics_test(set1 == set5);
           rblxphysics_test(set1.contains(7));
           rblxphysics_test(set1.contains(19));
        }

        void testIterators() {

            Set<int> set1(mAllocator);

            rblxphysics_test(set1.begin() == set1.end());

            set1.add(1);
            set1.add(2);
            set1.add(3);
            set1.add(4);

            Set<int>::Iterator itBegin = set1.begin();
            Set<int>::Iterator it = set1.begin();

            rblxphysics_test(itBegin == it);

            size_t size = 0;
            for (auto it = set1.begin(); it != set1.end(); ++it) {
                rblxphysics_test(set1.contains(*it));
                size++;
            }
            rblxphysics_test(set1.size() == size);
        }

        void testConverters() {

            Set<int> set1(mAllocator);

            rblxphysics_test(set1.begin() == set1.end());

            set1.add(1);
            set1.add(2);
            set1.add(3);
            set1.add(4);

            Array<int> array1 = set1.toArray(mAllocator);
            rblxphysics_test(array1.size() == 4);
            rblxphysics_test(array1.find(1) != array1.end());
            rblxphysics_test(array1.find(2) != array1.end());
            rblxphysics_test(array1.find(3) != array1.end());
            rblxphysics_test(array1.find(4) != array1.end());
            rblxphysics_test(array1.find(5) == array1.end());
            rblxphysics_test(array1.find(6) == array1.end());

            Set<int> set2(mAllocator);
            Array<int> array2 = set2.toArray(mAllocator);
            rblxphysics_test(array2.size() == 0);
        }
 };

}

#endif
