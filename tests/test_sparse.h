#pragma once

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "SparseMatrix.h"
#include "Vector.h"
#include "SOR.h"

using namespace CPPUNIT_NS;

class test_sparse
    : public TestFixture
{
    CPPUNIT_TEST_SUITE( test_sparse );
    CPPUNIT_TEST( test_matrix_creation );
    CPPUNIT_TEST( test_matrix_save_load );
    CPPUNIT_TEST( test_solve );
    CPPUNIT_TEST_SUITE_END();

protected:
    void test_matrix_creation( void );
    void test_matrix_save_load( void );
    void test_solve( void );
};
