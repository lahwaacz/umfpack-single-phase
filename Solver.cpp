#include <iostream>
using namespace std;

#include "Solver.h"

Solver::Solver( void )
{
}

Solver::~Solver( void )
{
    delete porosity;
    delete F;
    delete qN;
    delete pD;

    delete p;
    delete ptrace;

    delete mainMatrix;
    delete rhs;

    for( int k = 0; k < mesh->num_cells(); k++ )
        delete Ak[ k ];
    delete[] Ak;
    delete alpha;
    delete alpha_KE;
    delete lambda;

    delete mesh;
}

void Solver::init( void )
{
    area_width = 2.0;
    area_height = 1.0;
//    mesh_cols = 27;
//    mesh_rows = 27;
    mesh_cols = 100;
    mesh_rows = 100;
    mesh = new RectangularMesh( area_width, area_height, mesh_rows, mesh_cols );
    // TODO: load problem parameters from config file
    tau = 0.1;
    M = 28.96;
    R = 8.3144621;
    T = 300;
    permeability = 1e-3;
    viscosity = 18.6e-6;
    porosity = new Vector( mesh->num_cells(), 1e-10 );
    F = new Vector( mesh->num_cells() );

    // TODO: set boundary conditions
    // TODO: use sparse vectors
    qN = new Vector( mesh->num_edges() );
    pD = new Vector( mesh->num_cells(), 1e5 );
    // gradient on Dirichlet boundary
    for( IndexType i = 0; i < mesh_cols; i++ )
        (*pD)[ i ] += 1e4 / mesh_cols * (i + 1);

    // TODO: set initial conditions
    p = new Vector( mesh->num_cells(), 1e5 );
    ptrace = new Vector( mesh->num_edges() );

    mainMatrix = new SparseMatrix( mesh->num_edges(), mesh->num_edges() );
    rhs = new Vector( mesh->num_edges() );

    // auxiliary variables
    Ak = new DenseMatrix* [ mesh->num_cells() ];
    alpha = new Vector( mesh->num_cells() );
    alpha_KE = new SparseMatrix( mesh->num_cells(), mesh->num_edges() );
    lambda = new Vector( mesh->num_cells() );

    dxy = mesh->get_dx() / mesh->get_dy();
    dyx = mesh->get_dy() / mesh->get_dx();
}

void Solver::set_Ak( DenseMatrix & Ak, IndexType k )
{
    for( int i = 0; i < 4; i++ ) {
        for( int j = 0; j < 4; j++ ) {
            IndexType edge_E = mesh->edge_for_cell( k, i );
            IndexType edge_F = mesh->edge_for_cell( k, j );

            Ak( i, j ) = - alpha_KE->get( k, edge_E ) * alpha_KE->get( k, edge_F ) / ( (*lambda)[ k ] + (*alpha)[ k ] );

            // diagonal of b_k
            if( i == j )
                Ak( i, j) += alpha_KE->get( k, edge_E );
        }
    }
}

void Solver::set_main_matrix( void )
{
    // set auxiliary vectors + local matrices Ak on each element
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        // set alpha_KE, alpha_K coefficients
        for( int i = 0; i < 4; i++ ) {
            IndexType edge_E = mesh->edge_for_cell( cell_K, i );
            RealType value = 6 * (*p)[ cell_K ] * M / R / T * permeability / viscosity
                    * (( mesh->is_horizontal_edge(edge_E) ) ? dxy : dyx);
            alpha_KE->set( cell_K, edge_E, value );
            (*alpha)[ cell_K ] += value;
        }
        (*lambda)[ cell_K ] = (*porosity)[ cell_K ] * M / R / T * mesh->measure_cell( cell_K ) / tau;
        Ak[ cell_K ] = new DenseMatrix( 4, 4 );
        set_Ak( *Ak[ cell_K ], cell_K );
    }
    
    // set main matrix elements
    for( IndexType edge_E = 0; edge_E < mesh->num_edges(); edge_E++ ) {
        // inner edge or Neumann boundary
        if( ! mesh->is_dirichlet_boundary( edge_E ) ) {
            for( IndexType i = 0; i < 2; i++ ) {
                IndexType cell_K = mesh->cell_for_edge( edge_E, i );
                // on Neumann boundary only one term/cell contributes
                if( cell_K < 0 )
                    continue;
                for( IndexType j = 0; j < 4; j++ ) {
                    IndexType edge_F = mesh->edge_for_cell( cell_K, j );
                    RealType value = mainMatrix->get( edge_E, edge_F);
                    value += (*(Ak[ cell_K ]))( mesh->get_edge_order( cell_K, edge_E ), j );
                    mainMatrix->set( edge_E, edge_F, value );
                }
            }
        }
        // Dirichlet boundary
        else {
            mainMatrix->set( edge_E, edge_E, 1.0 );
        }
    }
}

void Solver::update_rhs( void )
{
    for( IndexType edge_E = 0; edge_E < mesh->num_edges(); edge_E++ ) {
        (*rhs)[ edge_E ] = 0.0;
        // inner edge or Neumann boundary
        if( ! mesh->is_dirichlet_boundary( edge_E ) ) {
            for( IndexType i = 0; i < 2; i++ ) {
                IndexType cell_K = mesh->cell_for_edge( edge_E, i );
                if( cell_K < 0 )
                    continue;
                // TODO: optimize as linear function of p[cell_K]
                (*rhs)[ edge_E ] += alpha_KE->get( cell_K, edge_E ) / ( (*lambda)[ cell_K ] + (*alpha)[ cell_K ] ) * ( (*F)[ cell_K ] + (*p)[ cell_K ] );
            }
        }
        // Dirichlet boundary
        else {
            (*rhs)[ edge_E ] = (*pD)[ edge_E ];
        }
        // Neumann boundary
        if( mesh->is_neumann_boundary( edge_E ) ) {
            (*rhs)[ edge_E ] += (*qN)[ edge_E ];
        }
    }
}

void Solver::update_p( void )
{
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        (*p)[ cell_K ] = (*F)[ cell_K ] + (*lambda)[ cell_K ] * (*p)[ cell_K ];
        for( IndexType i = 0; i < mesh->edges_per_cell(); i++ ) {
            IndexType edge_F = mesh->edge_for_cell( cell_K, i );
            (*p)[ cell_K ] += alpha_KE->get( cell_K, edge_F ) * (*ptrace)[ edge_F ];
        }
        (*p)[ cell_K ] /= (*lambda)[ cell_K ] + (*alpha)[ cell_K ];
    }
}

void Solver::run( void )
{
    set_main_matrix();
//    mainMatrix->save( "main-matrix.dat" );
//    // print matrix
//    for( IndexType i = 0; i < mesh->num_edges(); i++ ) {
//        for( IndexType j = 0; j < mesh->num_edges(); j++ ) {
//            cout << mainMatrix->get( i, j ) << " ";
//        }
//        cout << endl;
//    }

    // print initial conditions
    cout << "Time: 0.0" << endl;
//    for( IndexType i = 0; i < mesh_rows; i++ ) {
//        for( IndexType j = 0; j < mesh_cols; j++ ) {
//            cout << (*p)[ i * mesh_cols + j ] << " ";
//        }
//        cout << endl;
//    }

    for( unsigned i = 0; i < 100; i++ ) {
        update_rhs();
        bool status = mainMatrix->linear_solve( *ptrace, *rhs );
        if( status == false )
            break;
        update_p();

        // print actual p
        cout << "Time: " << (i+1) * tau << endl;
//        for( IndexType i = 0; i < mesh_rows; i++ ) {
//            for( IndexType j = 0; j < mesh_cols; j++ ) {
//                cout << (*p)[ i * mesh_cols + j ] << " ";
//            }
//            cout << endl;
//        }
    }

    p->save( "pressure-vect.dat" );
}

