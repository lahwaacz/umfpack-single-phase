#include <iostream>

#include "Solver.h"

using namespace std;


Solver::~Solver( void )
{
    delete mesh;
}

bool Solver::allocateVectors( void )
{
    bool status = true;

    // parameters
    status &= porosity.setSize( mesh->num_cells() );
    status &=        F.setSize( mesh->num_cells() );
    status &=       qN.setSize( mesh->num_edges() );
    status &=       pD.setSize( mesh->num_edges() );

    // main variables
    status &=        p.setSize( mesh->num_cells() );
    status &=   ptrace.setSize( mesh->num_edges() );
    status &=      rhs.setSize( mesh->num_edges() );

    // auxiliary variables
    status &=    alpha.setSize( mesh->num_cells() );
    status &=     beta.setSize( mesh->num_cells(), mesh->num_edges() );
    status &= beta.reserve( 4 * mesh->num_cells() );     // reserve memory to avoid reallocations
    status &=   lambda.setSize( mesh->num_cells() );

    return status;
}

bool Solver::init( void )
{
    area_width = 0.01;
    area_height = 0.01;
    mesh_cols = 100;
    mesh_rows = 100;
    mesh = new RectangularMesh( area_width, area_height, mesh_rows, mesh_cols );

    if( ! allocateVectors() ) {
        cerr << "Failed to allocate vectors." << endl;
        return false;
    }

    // parameters
    tau = 0.1;
    max_iterations = 10;
    snapshot_period = 0.1;
    grav_y = -9.806;
//    grav_y = 0.0;
    M = 28.96;
    R = 8.3144621;
    T = 300;
    permeability = 1e-3;
    viscosity = 18.6e-6;
    porosity.setAllElements( 1e-10 );
    F.setAllElements( 0.0 );

    // TODO: use sparse vectors
    // boundary conditions
    qN.setAllElements( 0.0 );
    pD.setAllElements( 0.0 );
    // gradient on Dirichlet boundary
    for( IndexType i = 0; i < mesh_cols; i++ )
        pD[ i ] = 1e5 + 1e4 / mesh_cols * (i + 1);

    // initial conditions
    p.setAllElements( 1e5 );
    ptrace.setAllElements( 1e5 );

    dxy = mesh->get_dx() / mesh->get_dy();
    dyx = mesh->get_dy() / mesh->get_dx();
    return true;
}

RealType Solver::G_KE( IndexType cell_K, IndexType edge_E )
{
    // left or right
    if( mesh->is_vertical_edge( edge_E ) )
        return 0.0;

    RealType value = 0.5 * p[ cell_K ] * M / R / T * grav_y * mesh->get_dy();

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

            Ak( i, j ) = - beta.getElement( k, edge_E ) * beta.getElement( k, edge_F ) / ( lambda[ k ] + alpha[ k ] );

            // diagonal of b_k
            if( i == j )
                Ak( i, j ) += beta.getElement( k, edge_E );
        }
    }
}

bool Solver::update_main_system( void )
{
    DenseMatrix** Ak = new DenseMatrix* [ mesh->num_cells() ];
    if( ! Ak ) {
        cerr << "Failed to allocate array for local matrices Ak." << endl;
        return false;
    }

    bool status = true;

    // set auxiliary vectors + local matrices Ak on each element
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        alpha[ cell_K ] = 0.0;
        for( int i = 0; i < 4; i++ ) {
            IndexType edge_E = mesh->edge_for_cell( cell_K, i );
            RealType value = 6 * p[ cell_K ] * M / R / T * permeability / viscosity
                    * (( mesh->is_horizontal_edge(edge_E) ) ? dxy : dyx);
            beta.setElement( cell_K, edge_E, value );
            alpha[ cell_K ] += value;
        }
        lambda[ cell_K ] = porosity[ cell_K ] * M / R / T * mesh->measure_cell( cell_K ) / tau;

        Ak[ cell_K ] = new DenseMatrix();
        if( ! Ak[ cell_K ] ) {
            cerr << "Failed to allocate local matrix Ak[ " << cell_K << " ]." << endl;
            status = false;
            break;
        }
        if( ! Ak[ cell_K ]->setSize( 4, 4 ) ) {
            cerr << "Failed to set size of local matrix Ak[ " << cell_K << " ]." << endl;
            status = false;
            break;
        }
        set_Ak( *Ak[ cell_K ], cell_K );
    }

    if( ! status )
        goto update_main_system_cleanup;
    
    // set main system elements
    for( IndexType edge_E = 0; edge_E < mesh->num_edges(); edge_E++ ) {
        rhs[ edge_E ] = 0.0;

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
                    if( ! mesh->is_dirichlet_boundary( edge_F ) ) {
                        // set main matrix element
                        RealType value = mainMatrix.getElement( edge_E, edge_F );
                        mainMatrix.setElement( edge_E, edge_F, value + A_KEF );
                    }
                    else {
                        rhs[ edge_E ] -= A_KEF * pD[ edge_F ];
                    }
                    // right hand side
                    rhs[ edge_E ] += A_KEF * G_KE( cell_K, edge_F );
                }

                // right-hand-side
                rhs[ edge_E ] += beta.getElement( cell_K, edge_E ) / ( lambda[ cell_K ] + alpha[ cell_K ] ) * ( F[ cell_K ] + lambda[ cell_K ] * p[ cell_K ] );
            }
        }
        // Dirichlet boundary
        else {
            mainMatrix.setElement( edge_E, edge_E, 1.0 );
            rhs[ edge_E ] = pD[ edge_E ];
        }
        // Neumann boundary
        if( mesh->is_neumann_boundary( edge_E ) ) {
            rhs[ edge_E ] += qN[ edge_E ];
        }
    }

update_main_system_cleanup:
    // release memory
    for( int k = 0; k < mesh->num_cells(); k++ )
        delete Ak[ k ];
    delete[] Ak;

    return status;
}

bool Solver::update_p( void )
{
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        p[ cell_K ] = F[ cell_K ] + lambda[ cell_K ] * p[ cell_K ];
        for( IndexType i = 0; i < mesh->edges_per_cell(); i++ ) {
            IndexType edge_F = mesh->edge_for_cell( cell_K, i );
            p[ cell_K ] += beta.getElement( cell_K, edge_F ) * ( ptrace[ edge_F ] - G_KE( cell_K, edge_F ) );
        }
        p[ cell_K ] /= lambda[ cell_K ] + alpha[ cell_K ];
    }
    return true;
}

bool Solver::run( void )
{
    bool status = init();
    if( ! status ) {
        cerr << "Failed to init the solver." << endl;
        return false;
    }

    int snapshot_period_iter = snapshot_period / tau;
    string snapshot_prefix = string( "pressure-vect-gravity-" ) + to_string( mesh_rows ) + "x" + to_string( mesh_cols ) + "-";

    for( unsigned i = 0; i < max_iterations; i++ ) {
        cout << "Time: " << i * tau << endl;

        // make snapshot, starting with initial conditions
        if( i % snapshot_period_iter == 0 ) {
            p.save( snapshot_prefix + to_string( i * tau ).substr( 0, 3 ) + ".dat" );
//            for( IndexType i = 0; i < mesh_rows; i++ ) {
//                for( IndexType j = 0; j < mesh_cols; j++ ) {
//                    cout << p[ i * mesh_cols + j ] << " ";
//                }
//                cout << endl;
//            }
        }

        // setSize() clears the matrix
        status = mainMatrix.setSize( mesh->num_edges(), mesh->num_edges() );
        if( ! status ) {
            cerr << "Failed to set size of the main matrix." << endl;
            break;
        }

        // reserve space to avoid reallocation
        status = mainMatrix.reserve( 7 * ( mesh->num_edges() - mesh->num_neumann_edges() - mesh->num_dirichlet_edges() ) + 4 * mesh->num_neumann_edges() + mesh->num_dirichlet_edges() );
        if( ! status ) {
            cerr << "Failed to reserve space for non-zero elements in the main matrix." << endl;
            break;
        }

        status = update_main_system();
        if( ! status ) {
            cerr << "Failed to update the main system." << endl;
        }

        status = mainMatrix.linear_solve( ptrace, rhs );
        if( ! status ) {
            cerr << "Failed to solve the main system." << endl;
            break;
        }
        update_p();
    }

    if( status ) {
        // print final p
        cout << "Time: " << max_iterations * tau << endl;
//        for( IndexType i = 0; i < mesh_rows; i++ ) {
//            for( IndexType j = 0; j < mesh_cols; j++ ) {
//                cout << p[ i * mesh_cols + j ] << " ";
//            }
//            cout << endl;
//        }

        p.save( snapshot_prefix + to_string( max_iterations * tau ).substr( 0, 3 ) + ".dat" );
    }
    return status;
}

