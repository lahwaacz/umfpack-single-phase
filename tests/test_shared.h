#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "SharedVector.h"

using namespace CPPUNIT_NS;

class test_shared
    : public TestFixture
{
    CPPUNIT_TEST_SUITE( test_shared );
    CPPUNIT_TEST( test_bind );
    CPPUNIT_TEST( test_setsize_disabled );
    CPPUNIT_TEST_SUITE_END();

protected:
    void test_bind( void );
    void test_setsize_disabled( void );
};
