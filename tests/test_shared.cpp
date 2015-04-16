#include "test_shared.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( TestSharedArray );


void TestSharedArray::test_constructors( void )
{
    Array a( 10 );

    // basic constructor
    SharedArray s1;
    CPPUNIT_ASSERT_EQUAL( 0, s1.getSize() );
    CPPUNIT_ASSERT_EQUAL( (RealType*) nullptr, s1.getData() );

    // constructor with initialization Array
    SharedArray s2( a );
    CPPUNIT_ASSERT_EQUAL( a.getSize(), s2.getSize() );
    CPPUNIT_ASSERT_EQUAL( a.getData(), s2.getData() );

    // constructor with initialization data pointer + size
    SharedArray s3( a.getData(), a.getSize() );
    CPPUNIT_ASSERT_EQUAL( a.getSize(), s3.getSize() );
    CPPUNIT_ASSERT_EQUAL( a.getData(), s3.getData() );

    // constructor with initialization data reference + size
    SharedArray s4( *a.getData(), a.getSize() );
    CPPUNIT_ASSERT_EQUAL( a.getSize(), s4.getSize() );
    CPPUNIT_ASSERT_EQUAL( a.getData(), s4.getData() );
}

void TestSharedArray::test_bind( void )
{
    unsigned rows = 10;

    Array a;
    a.setSize( rows );
    a.setAllElements( 0.0 );

    // test first-level sharing
    SharedArray s1;
    s1.bind( a[ 1 ], rows - 2 );
    s1.setAllElements( 1.0 );

    CPPUNIT_ASSERT_EQUAL( 0.0, a[ 0 ] );
    for( IndexType i = 1; i < a.getSize() - 1; i++ ) {
        CPPUNIT_ASSERT_EQUAL( 1.0, a[ i ] );
    }
    CPPUNIT_ASSERT_EQUAL( 0.0, a[ a.getSize() - 1 ] );

    // test second-level sharing
    SharedArray s2;
    s2.bind( s1[ 1 ], s1.getSize() - 2 );
    s2.setAllElements( 2.0 );

    CPPUNIT_ASSERT_EQUAL( 0.0, a[ 0 ] );
    CPPUNIT_ASSERT_EQUAL( 1.0, a[ 1 ] );
    for( IndexType i = 2; i < a.getSize() - 2; i++ ) {
        CPPUNIT_ASSERT_EQUAL( 2.0, a[ i ] );
    }
    CPPUNIT_ASSERT_EQUAL( 1.0, a[ a.getSize() - 2 ] );
    CPPUNIT_ASSERT_EQUAL( 0.0, a[ a.getSize() - 1 ] );
}

void TestSharedArray::test_setsize_disabled( void )
{
    SharedArray s;
    s.setSize( 5 );

    // shared vectors cannot be resized
    CPPUNIT_ASSERT_EQUAL( false, s.setSize( 10 ) );

    // call it also from reference to Array to make sure setSize is overridden correctly
    Array & a = s;
    CPPUNIT_ASSERT_EQUAL( false, a.setSize( 10 ) );
}

void TestSharedArray::test_pointers( void )
{
    // Test to ensure that base class has virtual destructor.
    // Deleting object of polymorphic class which has non-virtual destructor
    // might cause undefined behaviour [-Wdelete-non-virtual-dtor]
    //
    // See http://stackoverflow.com/a/461224/4180822

    Array* a = new Array();
    a->setSize( 10 );
    SharedArray* s = new SharedArray();
    s->bind( a->getData()[ 5 ], 5 );

    delete s;
    delete a;
}

void TestSharedArray::test_copy_assignment( void )
{
    Array a( 20 );
    a.setAllElements( 0.0 );

    SharedArray s;
    s.bind( a[ 10 ], 10 );

    Array b( 10 );
    b.setAllElements( 1.0 );

    s = b;

    for( int i = 0; i < 10; i++ )
        CPPUNIT_ASSERT_EQUAL( 0.0, a[ i ] );
    for( int i = 10; i < 20; i++ )
        CPPUNIT_ASSERT_EQUAL( 1.0, a[ i ] );
}

