#include <fstream>
#include <sstream>
#include <string>

#include "test_dense.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( test_dense );


void test_dense::test_vector_creation( void )
{
    unsigned rows = 4;

    Vector v;
    v.setSize( rows );
    v.setAllElements( 0.0 );
    for( unsigned i = 0; i < rows; i++ )
        CPPUNIT_ASSERT_EQUAL( 0.0, v[ i ] );
}

void test_dense::test_vector_save_load( void )
{
    string fname("test-vector.dat");
    unsigned rows = 4;

    Vector v;
    v.setSize( rows );
    v[ 0 ] = 1.1;
    v[ 1 ] = 2.2;
    v[ 2 ] = 3.3;
    v[ 3 ] = 4.4;
    v.save( fname );

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
    Vector b;
    b.setSize( rows );
    b.load( fname );
    for( unsigned i = 0; i < rows; i++ ) {
        CPPUNIT_ASSERT_EQUAL( v[ i ], b[ i ] );
    }
}

void test_dense::test_matrix_creation( void )
{
    unsigned rows = 4;
    unsigned cols = 3;

    DenseMatrix m;
    m.setSize( rows, cols );
    m.setAllElements( 0.0 );
    for( unsigned i = 0; i < rows; i++ )
        for( unsigned j = 0; j < cols; j++ )
            CPPUNIT_ASSERT_EQUAL( 0.0, m( i, j ) );
}

void test_dense::test_matrix_save_load( void )
{
    string fname("test-matrix.dat");
    unsigned rows = 4;
    unsigned cols = 3;

    DenseMatrix m;
    m.setSize( rows, cols );
    m.setAllElements( 0.0 );
    m( 0, 0 ) = 1.1;
    m( 0, 1 ) = 2.2;
    m( 0, 2 ) = 3.3;
    m( 3, 0 ) = 4.4;
    m.save( fname );

    // test created file
    ifstream f( fname.c_str() );
    stringstream buffer;
    buffer << f.rdbuf();
    CPPUNIT_ASSERT_EQUAL(
            string(
"# saved dense matrix:\n"
"# <row index> <column index> <value>\n"
"0 0 1.1\n"
"0 1 2.2\n"
"0 2 3.3\n"
"1 0 0\n"
"1 1 0\n"
"1 2 0\n"
"2 0 0\n"
"2 1 0\n"
"2 2 0\n"
"3 0 4.4\n"
"3 1 0\n"
"3 2 0\n"),
            buffer.str() );

    // test file loading
    DenseMatrix b;
    b.setSize( rows, cols );
    b.load( fname );
    for( unsigned i = 0; i < rows; i++ )
        for( unsigned j = 0; j < cols; j++ )
            CPPUNIT_ASSERT_EQUAL( m( i, j ), b( i, j ) );
}

void test_dense::test_solve( void )
{
    unsigned order = 3;
    DenseMatrix m;
    m.setSize( order, order );
    m.setAllElements( 0.0 );
    m(0,0) = 1;
    m(0,1) = 2;
    m(0,2) = 3;
    m(1,1) = 1;
    m(2,2) = 2;

    Vector v;
    v.setSize( order );
    v[ 0 ] = 1;
    v[ 1 ] = 1;
    v[ 2 ] = 1;

    Vector x;
    x.setSize( order );
    
    bool converged = SORMethod(m, v, x, 0.001, 1.0);
    CPPUNIT_ASSERT_EQUAL( true, converged );

    CPPUNIT_ASSERT_EQUAL( -2.5, x[ 0 ] );
    CPPUNIT_ASSERT_EQUAL( 1.0, x[ 1 ] );
    CPPUNIT_ASSERT_EQUAL( 0.5, x[ 2 ] );
}

