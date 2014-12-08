#include <fstream>
#include <sstream>
#include <string>

#include "test_shared.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( test_shared );


void test_shared::test_bind( void )
{
    unsigned rows = 10;

    Vector v;
    v.setSize( rows );
    v.setAllElements( 0.0 );

    // test first-level sharing
    SharedVector s1;
    s1.bind( v[ 1 ], rows - 2 );
    s1.setAllElements( 1.0 );

    CPPUNIT_ASSERT_EQUAL( 0.0, v[ 0 ] );
    for( IndexType i = 1; i < v.getSize() - 1; i++ ) {
        CPPUNIT_ASSERT_EQUAL( 1.0, v[ i ] );
    }
    CPPUNIT_ASSERT_EQUAL( 0.0, v[ v.getSize() - 1 ] );

    // test second-level sharing
    SharedVector s2;
    s2.bind( s1[ 1 ], s1.getSize() - 2 );
    s2.setAllElements( 2.0 );

    CPPUNIT_ASSERT_EQUAL( 0.0, v[ 0 ] );
    CPPUNIT_ASSERT_EQUAL( 1.0, v[ 1 ] );
    for( IndexType i = 2; i < v.getSize() - 2; i++ ) {
        CPPUNIT_ASSERT_EQUAL( 2.0, v[ i ] );
    }
    CPPUNIT_ASSERT_EQUAL( 1.0, v[ v.getSize() - 2 ] );
    CPPUNIT_ASSERT_EQUAL( 0.0, v[ v.getSize() - 1 ] );
}

void test_shared::test_setsize_disabled( void )
{
    SharedVector s;
    s.setSize( 5 );

    // shared vectors cannot be resized
    CPPUNIT_ASSERT_EQUAL( false, s.setSize( 10 ) );

    // call it also from reference to Vector to make sure setSize is overridden correctly
    Vector & v = s;
    CPPUNIT_ASSERT_EQUAL( false, v.setSize( 10 ) );
}

void test_shared::test_pointers( void )
{
    // Test to ensure that base class has virtual destructor.
    // Deleting object of polymorphic class which has non-virtual destructor
    // might cause undefined behaviour [-Wdelete-non-virtual-dtor]
    //
    // See http://stackoverflow.com/a/461224/4180822

    Vector* v = new Vector();
    v->setSize( 10 );
    SharedVector* s = new SharedVector();
    s->bind( v->getData()[ 5 ], 5 );

    delete s;
    delete v;
}

