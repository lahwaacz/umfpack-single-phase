#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "SharedArray.h"

using namespace CPPUNIT_NS;

class TestSharedArray
    : public TestFixture
{
    CPPUNIT_TEST_SUITE( TestSharedArray );
    CPPUNIT_TEST( test_constructors );
    CPPUNIT_TEST( test_bind );
    CPPUNIT_TEST( test_setsize_disabled );
    CPPUNIT_TEST( test_pointers );
    CPPUNIT_TEST( test_copy_assignment );
    CPPUNIT_TEST_SUITE_END();

protected:
    void test_constructors( void );
    void test_bind( void );
    void test_setsize_disabled( void );
    void test_pointers( void );
    void test_copy_assignment( void );
};
