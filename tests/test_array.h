#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Array.h"

using namespace CPPUNIT_NS;

class TestArray
    : public TestFixture
{
    CPPUNIT_TEST_SUITE( TestArray );
    CPPUNIT_TEST( test_constructors );
    CPPUNIT_TEST( test_element_access );
    CPPUNIT_TEST( test_copy_assignment );
    CPPUNIT_TEST( test_move_assignment );
    CPPUNIT_TEST( test_save_load );
    CPPUNIT_TEST( test_print );
    CPPUNIT_TEST_SUITE_END();

protected:
    void test_constructors( void );
    void test_element_access( void );
    void test_copy_assignment( void );
    void test_move_assignment( void );
    void test_save_load( void );
    void test_print( void );
};
