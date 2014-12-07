#include <fstream>
#include <sstream>
#include <string>

#include "test_sparse.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( test_sparse );


void test_sparse::test_matrix_creation( void )
{
    unsigned rows = 4;
    unsigned cols = 3;

    SparseMatrix m;
    m.setSize( rows, cols );
    for( unsigned i = 0; i < rows; i++ )
        for( unsigned j = 0; j < cols; j++ )
            CPPUNIT_ASSERT_EQUAL( 0.0, m.getElement( i, j ) );

    m.setElement( 3, 0, 4.4);
    m.setElement( 0, 0, 1.1);
    m.setElement( 0, 1, 2.2);
    m.setElement( 0, 2, 3.3);

    CPPUNIT_ASSERT_EQUAL( 1.1, m.getElement( 0, 0 ) );
    CPPUNIT_ASSERT_EQUAL( 2.2, m.getElement( 0, 1 ) );
    CPPUNIT_ASSERT_EQUAL( 3.3, m.getElement( 0, 2 ) );
    CPPUNIT_ASSERT_EQUAL( 4.4, m.getElement( 3, 0 ) );
}

void test_sparse::test_matrix_save_load( void )
{
    string fname("test-sparse-matrix.dat");
    unsigned rows = 4;
    unsigned cols = 3;

    SparseMatrix m;
    m.setSize( rows, cols );
    m.setElement( 0, 0, 1.1);
    m.setElement( 0, 1, 2.2);
    m.setElement( 0, 2, 3.3);
    m.setElement( 3, 0, 4.4);
    m.save( fname );

    // test created file
    ifstream f( fname.c_str() );
    stringstream buffer;
    buffer << f.rdbuf();
    CPPUNIT_ASSERT_EQUAL(
            string(
"1.1 2.2 3.3 4.4 \n"
"0 1 2 0 \n"
"0 3 3 3 4 \n"),
            buffer.str() );

    // test file loading
    SparseMatrix b;
    b.setSize( rows, cols );
    b.load( fname );
    for( unsigned i = 0; i < rows; i++ )
        for( unsigned j = 0; j < cols; j++ )
            CPPUNIT_ASSERT_EQUAL( m.getElement( i, j ), b.getElement( i, j ) );
}

void test_sparse::test_solve( void )
{
    unsigned order = 3;
    SparseMatrix m;
    m.setSize( order, order );
    m.setElement( 0, 0, 1.1 );
    m.setElement( 2, 1, 2.2 );
    m.setElement( 1, 2, 3.3 );

    Vector x;
    x.setSize( order );
    Vector b;
    b.setSize( order );
    b[ 0 ] = 4.0;
    b[ 1 ] = 5.0;
    b[ 2 ] = 6.0;

    m.linear_solve( x, b );

    // test solution
    CPPUNIT_ASSERT_EQUAL( 3.6363636363636362, x[ 0 ] );
    CPPUNIT_ASSERT_EQUAL( 2.7272727272727271, x[ 1 ] );
    CPPUNIT_ASSERT_EQUAL( 1.5151515151515151, x[ 2 ] );
}

