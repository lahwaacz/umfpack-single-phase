#include <iostream>

#include "Solver.h"

using namespace std;


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

    delete rhs;

    delete alpha;
    delete alpha_KE;
    delete lambda;

    delete mesh;
}

void Solver::init( void )
{
    area_width = 0.01;
    area_height = 0.01;
    mesh_cols = 100;
    mesh_rows = 100;
    mesh = new RectangularMesh( area_width, area_height, mesh_rows, mesh_cols );
    // TODO: load problem parameters from config file
    tau = 0.1;
    max_iterations = 30;
    snapshot_period = 1.0;
    grav_y = -9.806;
//    grav_y = 0.0;
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
    pD = new Vector( mesh->num_edges(), 1e5 );
    // gradient on Dirichlet boundary
    for( IndexType i = 0; i < mesh_cols; i++ )
        pD->at( i ) += 1e4 / mesh_cols * (i + 1);

    // TODO: set initial conditions
    p = new Vector( mesh->num_cells(), 1e5 );
    ptrace = new Vector( mesh->num_edges() );

    rhs = new Vector( mesh->num_edges() );

    // auxiliary variables
    alpha = new Vector( mesh->num_cells() );
    alpha_KE = new SparseMatrix( mesh->num_cells(), mesh->num_edges() );
    lambda = new Vector( mesh->num_cells() );

    dxy = mesh->get_dx() / mesh->get_dy();
    dyx = mesh->get_dy() / mesh->get_dx();
}

RealType Solver::G_KE( IndexType cell_K, IndexType edge_E )
{
    // left or right
    if( mesh->is_vertical_edge( edge_E ) )
        return 0.0;

    RealType value = 0.5 * p->at( cell_K ) * M / R / T * grav_y * mesh->get_dy();

    // bottom
    if( mesh->get_edge_order( cell_K, edge_E ) == 1 )
        return value;

    // top
    return -value;
}

void Solver::set_Ak( DenseMatrix & Ak, IndexType k )
{
    for( int i = 0; i < 4; i++ ) {
        for( int j = 0; j < 4; j++ ) {
            IndexType edge_E = mesh->edge_for_cell( k, i );
            IndexType edge_F = mesh->edge_for_cell( k, j );

            Ak( i, j ) = - alpha_KE->get( k, edge_E ) * alpha_KE->get( k, edge_F ) / ( lambda->at( k ) + alpha->at( k ) );

            // diagonal of b_k
            if( i == j )
                Ak( i, j ) += alpha_KE->get( k, edge_E );
        }
    }
}

void Solver::update_main_system( void )
{
    DenseMatrix** Ak = new DenseMatrix* [ mesh->num_cells() ];

    // set auxiliary vectors + local matrices Ak on each element
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        // set alpha_KE, alpha_K coefficients
        alpha->at( cell_K ) = 0.0;
        for( int i = 0; i < 4; i++ ) {
            IndexType edge_E = mesh->edge_for_cell( cell_K, i );
            RealType value = 6 * p->at( cell_K ) * M / R / T * permeability / viscosity
                    * (( mesh->is_horizontal_edge(edge_E) ) ? dxy : dyx);
            alpha_KE->set( cell_K, edge_E, value );
            alpha->at( cell_K ) += value;
        }
        lambda->at( cell_K ) = porosity->at( cell_K ) * M / R / T * mesh->measure_cell( cell_K ) / tau;
        Ak[ cell_K ] = new DenseMatrix( 4, 4 );
        set_Ak( *Ak[ cell_K ], cell_K );
    }
    
    // set main system elements
    for( IndexType edge_E = 0; edge_E < mesh->num_edges(); edge_E++ ) {
        rhs->at( edge_E ) = 0.0;

        // inner edge or Neumann boundary
        if( ! mesh->is_dirichlet_boundary( edge_E ) ) {
            for( IndexType i = 0; i < 2; i++ ) {
                IndexType cell_K = mesh->cell_for_edge( edge_E, i );
                // on Neumann boundary only one term/cell contributes
                if( cell_K < 0 )
                    continue;

                for( IndexType j = 0; j < 4; j++ ) {
                    IndexType edge_F = mesh->edge_for_cell( cell_K, j );
                    RealType A_KEF = (*(Ak[ cell_K ]))( mesh->get_edge_order( cell_K, edge_E ), j );
                    // set main matrix element
                    RealType value = mainMatrix->get( edge_E, edge_F);
                    mainMatrix->set( edge_E, edge_F, value + A_KEF );
                    // right hand side
                    rhs->at( edge_E ) += A_KEF * G_KE( cell_K, edge_F );
                }

                // right-hand-side
                rhs->at( edge_E ) += alpha_KE->get( cell_K, edge_E ) / ( lambda->at( cell_K ) + alpha->at( cell_K ) ) * ( F->at( cell_K ) + lambda->at( cell_K ) * p->at( cell_K ) );
            }
        }
        // Dirichlet boundary
        else {
            mainMatrix->set( edge_E, edge_E, 1.0 );
            rhs->at( edge_E ) = pD->at( edge_E );
        }
        // Neumann boundary
        if( mesh->is_neumann_boundary( edge_E ) ) {
            rhs->at( edge_E ) += qN->at( edge_E );
        }
    }

    // release memory
    for( int k = 0; k < mesh->num_cells(); k++ )
        delete Ak[ k ];
    delete[] Ak;
}

void Solver::update_p( void )
{
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        p->at( cell_K ) = F->at( cell_K ) + lambda->at( cell_K ) * p->at( cell_K );
        for( IndexType i = 0; i < mesh->edges_per_cell(); i++ ) {
            IndexType edge_F = mesh->edge_for_cell( cell_K, i );
            p->at( cell_K ) += alpha_KE->get( cell_K, edge_F ) * ( ptrace->at( edge_F ) - G_KE( cell_K, edge_F ) );
        }
        p->at( cell_K ) /= lambda->at( cell_K ) + alpha->at( cell_K );
    }
}

void Solver::run( void )
{
    bool status = true;
    int snapshot_period_iter = snapshot_period / tau;
    string snapshot_prefix = string( "pressure-vect-" ) + to_string( mesh_rows ) + "x" + to_string( mesh_cols ) + "-";

    for( unsigned i = 0; i < max_iterations; i++ ) {
        cout << "Time: " << i * tau << endl;

        // make snapshot, starting with initial conditions
        if( i % snapshot_period_iter == 0 ) {
            p->save( snapshot_prefix + to_string( i * tau ).substr( 0, 3 ) + ".dat" );
//            for( IndexType i = 0; i < mesh_rows; i++ ) {
//                for( IndexType j = 0; j < mesh_cols; j++ ) {
//                    cout << p->at( i * mesh_cols + j ) << " ";
//                }
//                cout << endl;
//            }
        }

        mainMatrix = new SparseMatrix( mesh->num_edges(), mesh->num_edges() );
        update_main_system();
        status = mainMatrix->linear_solve( *ptrace, *rhs );
        if( status == false )
            break;
        update_p();

        delete mainMatrix;
    }

    if( status ) {
        // print final p
        cout << "Time: " << max_iterations * tau << endl;
//        for( IndexType i = 0; i < mesh_rows; i++ ) {
//            for( IndexType j = 0; j < mesh_cols; j++ ) {
//                cout << p->at( i * mesh_cols + j ) << " ";
//            }
//            cout << endl;
//        }

        p->save( snapshot_prefix + to_string( max_iterations * tau ).substr( 0, 3 ) + ".dat" );
    }
}

