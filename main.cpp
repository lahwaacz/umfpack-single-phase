#include <iostream>
#include <string>

#include "DenseMatrix.h"
#include "SparseMatrix.h"
#include "Vector.h"
#include "SOR.h"

#include "RectangularMesh.h"
#include "Solver.h"

using namespace std;

void test_SOR( void )
{
    DenseMatrix m(3,3);
    m(0,0) = 1;
    m(0,1) = 2;
    m(0,2) = 3;
    m(1,1) = 1;
    m(2,2) = 2;

    Vector v(3);
    v[0] = 1;
    v[1] = 1;
    v[2] = 1;

    Vector x(3);
    
    bool converged = SORMethod(m, v, x, 0.001, 1.0);
    cout << "SOR method converged: " << converged << endl;

    for( int i = 0; i < 3; i++ )
    {
        for( int j = 0; j < 3; j++ )
        {
            cout << m( i, j ) << "*" << x[ j ];
            if( j < 2 )
                cout << " + ";
        }
        cout << " = " << v[ i ] << endl;
    }
}

void test_mesh(void)
{
    RectangularMesh mesh(1, 1, 3, 4);
    cout << "#cells: " << mesh.num_cells() << endl;
    cout << "#edges: " << mesh.num_edges() << endl;

    cout << "is_inner_edge" << endl;
    int inner_count = 0;
    for( int i = 0; i < mesh.num_edges(); i++ ) {
        cout << "index " << i << " is ";
        bool inner = mesh.is_inner_edge(i);
        if( inner == true ) {
            cout << "inner" << endl;
            inner_count++;
        }
        else
            cout << "outer" << endl;
    }
    cout << "counted " << inner_count << " inner edges, " << mesh.num_edges() - inner_count << " outer edges" << endl;

    cout << "testing edge_for_cell()" << endl;
    for( int cell = 0; cell < mesh.num_cells(); cell++ ) {
        cout << "cell " << cell << ", edges: ";
        for( int j = 0; j < mesh.edges_per_cell(); j++ ) {
            cout << mesh.edge_for_cell( cell, j ) << " ";
        }
        cout << endl;
    }

    cout << "testing cell_for_edge()" << endl;
    for( int edge = 0; edge < mesh.num_edges(); edge++ ) {
        cout << "edge " << edge << ", cells: ";
        for( int j = 0; j < 2; j++ ) {
            cout << mesh.cell_for_edge( edge, j ) << " ";
        }
        cout << endl;
    }
}

void test_sparse_matrix( void )
{
    string fname("test-sparse-matrix.dat");
    unsigned rows = 4;
    unsigned cols = 3;

    SparseMatrix m( rows, cols );
    m.set( 0, 0, 1.1 );
    m.set( 0, 1, 2.2 );
    m.set( 0, 2, 3.3 );
    m.set( 3, 0, 4.4 );
    m.save( fname );
    SparseMatrix b( rows, cols );
    b.load( fname );
    for( unsigned i = 0; i < rows; i++ ) {
        for( unsigned j = 0; j < cols; j++ ) {
            cout << b.get( i, j ) << " ";
        }
        cout << endl;
    }
}

void test_umfpack_solve( void )
{
    unsigned order = 3;
    SparseMatrix m( order );
    m.set( 0, 0, 1.1 );
    m.set( 2, 1, 2.2 );
    m.set( 1, 2, 3.3 );

    Vector x( order );
    Vector b( order );
    b[ 0 ] = 4.0;
    b[ 1 ] = 5.0;
    b[ 2 ] = 6.0;

    m.linear_solve( x, b );

    // výpis řešení
    cout << "x = [";
    for( unsigned i = 0; i < order; i++ ) {
        cout << x[i];
        if( i < order - 1 )
            cout << ",";
    }
    cout << "]" << endl;
}

int main( int argc, char** argv )
{
    Solver s;
    s.init();
    s.run();
}

