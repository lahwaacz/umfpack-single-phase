#include <fstream>
#include <sstream>
#include <string>

#include "test_array.h"
#include "exceptions.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( TestArray );


void TestArray::test_constructors( void )
{
    // basic constructor
    Array a;
    CPPUNIT_ASSERT_EQUAL( 0, a.getSize() );
    CPPUNIT_ASSERT_EQUAL( (RealType*) nullptr, a.getData() );

    // constructor with array size
    Array b( 10 );
    CPPUNIT_ASSERT_EQUAL( 10, b.getSize() );

    // copy constructor
    Array c( b );
    CPPUNIT_ASSERT_EQUAL( b.getSize(), c.getSize() );
    for( int i = 0; i < b.getSize(); i++ )
        CPPUNIT_ASSERT_EQUAL( b[ i ], c[ i ] );

    // move constructor
    RealType* data = b.getData();
    Array d( std::move( b ) );
    CPPUNIT_ASSERT_EQUAL( 10, d.getSize() );
    CPPUNIT_ASSERT_EQUAL( 0, b.getSize() );
    CPPUNIT_ASSERT_EQUAL( data, d.getData() );
}

void TestArray::test_element_access( void )
{
    int size = 4;

    Array a( size );
    a.setAllElements( 0.0 );
    for( int i = 0; i < size; i++ )
        CPPUNIT_ASSERT_EQUAL( 0.0, a[ i ] );

    for( int i = 0; i < size; i++ ) {
        a.setElement( i, 1.0 );
        CPPUNIT_ASSERT_EQUAL( 1.0, a.getElement( i ) );
        CPPUNIT_ASSERT_EQUAL( 1.0, a[ i ] );
        a[ i ] = 2.0;
        CPPUNIT_ASSERT_EQUAL( 2.0, a[ i ] );
    }
}

void TestArray::test_copy_assignment( void )
{
    Array a( 10 );
    a.setAllElements( 0.0 );

    Array b;
    CPPUNIT_ASSERT_THROW( b = a, BadArraySize );
    b.setSize( a.getSize() );
    b = a;

    CPPUNIT_ASSERT_EQUAL( a.getSize(), b.getSize() );
    for( int i = 0; i < a.getSize(); i++ )
        CPPUNIT_ASSERT_EQUAL( a[ i ], b[ i ] );

    // make sure it was copied, not moved
    a.setAllElements( 1.0 );
    for( int i = 0; i < a.getSize(); i++ ) {
        CPPUNIT_ASSERT_EQUAL( 1.0, a[ i ] );
        CPPUNIT_ASSERT_EQUAL( 0.0, b[ i ] );
    }
}

void TestArray::test_move_assignment( void )
{
    Array a( 10 );
    a.setAllElements( 0.0 );

    Array b( 20 );
    b.setAllElements( 1.0 );

    for( int i = 0; i < a.getSize(); i++ )
        CPPUNIT_ASSERT_EQUAL( 0.0, a[ i ] );
    for( int i = 0; i < b.getSize(); i++ )
        CPPUNIT_ASSERT_EQUAL( 1.0, b[ i ] );

    b = std::move( a );

    // check size
    CPPUNIT_ASSERT_EQUAL( 20, a.getSize() );
    CPPUNIT_ASSERT_EQUAL( 10, b.getSize() );

    // check elements
    for( int i = 0; i < a.getSize(); i++ )
        CPPUNIT_ASSERT_EQUAL( 1.0, a[ i ] );
    for( int i = 0; i < b.getSize(); i++ )
        CPPUNIT_ASSERT_EQUAL( 0.0, b[ i ] );
}

void TestArray::test_save_load( void )
{
    string fname("test-array.dat");
    int size = 4;

    Array a( size );
    a[ 0 ] = 1.1;
    a[ 1 ] = 2.2;
    a[ 2 ] = 3.3;
    a[ 3 ] = 4.4;
    a.save( fname );

    // test created file
    ifstream f( fname.c_str() );
    stringstream buffer;
    buffer << f.rdbuf();
    CPPUNIT_ASSERT_EQUAL(
            string(
"# saved vector:\n"
"# <row index> <value>\n"
"0 1.1\n"
"1 2.2\n"
"2 3.3\n"
"3 4.4\n"),
            buffer.str() );

    // test file loading
    Array b;
    b.setSize( size );
    b.load( fname );
    for( int i = 0; i < size; i++ )
        CPPUNIT_ASSERT_EQUAL( a[ i ], b[ i ] );
}

void TestArray::test_print( void )
{
    Array a( 5 );
    for( int i = 0; i < a.getSize(); i++ )
        a[ i ] = i;

    stringstream buffer;
    a.print( buffer );

    CPPUNIT_ASSERT_EQUAL(
            string(
"0\n"
"1\n"
"2\n"
"3\n"
"4\n"),
            buffer.str() );
}

