#include <iostream>
#include <sstream>

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
    status &= pressure.setSize( mesh->num_cells() );
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
    area_width = 10;
    area_height = 10;
    mesh_cols = 100;
    mesh_rows = 100;
    mesh = new RectangularMesh( area_width, area_height, mesh_rows, mesh_cols );

    if( ! allocateVectors() ) {
        cerr << "Failed to allocate vectors." << endl;
        return false;
    }

    // parameters
    tau = 0.1;
    final_time = 30.0;
    snapshot_period = 1.0;
    grav_y = -9.806;
//    grav_y = 0.0;
    const RealType M = 28.96e-3;
    const RealType R = 8.3144621;
    const RealType T = 300;
    idealGasCoefficient = M / R / T;
    permeability = 1e-10;
    viscosity = 18.6e-6;
    porosity.setAllElements( 0.4 );
    F.setAllElements( 0.0 );

    // TODO: use sparse vectors
    // boundary conditions
    qN.setAllElements( 0.0 );
    pD.setAllElements( 0.0 );
    // gradient on Dirichlet boundary
    IndexType col = 0;
    for( IndexType i = mesh_cols * mesh_rows; i < mesh_cols * (mesh_rows + 1); i++ ) {
        pD[ i ] = 1e5 + 1e3 / mesh_cols * (col++ + 1);
    }

//    // constant pressure on left border
//    for( IndexType i = (mesh_rows + 1) * mesh_cols; i < mesh->num_edges(); i++ ) {
//        if( i % (mesh_cols + 1) == 0 )
//            pD[ i ] = 1e5;
//    }
//    // constant (higher) pressure on right border
//    for( IndexType i = (mesh_rows + 1) * mesh_cols; i < mesh->num_edges(); i++ ) {
//        if( i % (mesh_cols + 1) == mesh_cols )
//            pD[ i ] = 1.01e5;
//    }

    // initial conditions
    pressure.setAllElements( 1e5 );
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

    RealType value = 0.5 * idealGasCoefficient * grav_y * mesh->get_dy();

    // bottom
    if( mesh->get_edge_order( cell_K, edge_E ) == 1 )
        return value;

    // top
    return -value;
}

bool Solver::update_auxiliary_vectors( void )
{
    // set auxiliary vectors
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        alpha[ cell_K ] = 0.0;
        for( int i = 0; i < 4; i++ ) {
            IndexType edge_E = mesh->edge_for_cell( cell_K, i );
            RealType value = 2 * idealGasCoefficient * permeability / viscosity
                    * (( mesh->is_horizontal_edge(edge_E) ) ? dxy : dyx);
            beta.setElement( cell_K, edge_E, value );
            alpha[ cell_K ] += value;
        }
        lambda[ cell_K ] = porosity[ cell_K ] * idealGasCoefficient * mesh->measure_cell( cell_K ) / tau;
    }
    return true;
}

bool Solver::update_main_system( void )
{
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

                    RealType A_KEF = - beta.getElement( cell_K, edge_E ) * pressure[ cell_K ] * beta.getElement( cell_K, edge_F ) * pressure[ cell_K ] / ( lambda[ cell_K ] + alpha[ cell_K ] * pressure[ cell_K ] );
                    if( edge_E == edge_F )
                        A_KEF += beta.getElement( cell_K, edge_E ) * pressure[ cell_K ];

                    if( ! mesh->is_dirichlet_boundary( edge_F ) ) {
                        // set main matrix element
                        RealType value = mainMatrix.getElement( edge_E, edge_F );
                        mainMatrix.setElement( edge_E, edge_F, value + A_KEF );
                    }
                    else {
                        rhs[ edge_E ] -= A_KEF * pD[ edge_F ];
                    }
                    // right hand side
                    rhs[ edge_E ] += A_KEF * G_KE( cell_K, edge_F ) * pressure[ cell_K ];
                }

                // right-hand-side
                rhs[ edge_E ] += beta.getElement( cell_K, edge_E ) * pressure[ cell_K ] / ( lambda[ cell_K ] + alpha[ cell_K ] * pressure[ cell_K ] ) * ( F[ cell_K ] + lambda[ cell_K ] * pressure[ cell_K ] );
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
    return true;
}

bool Solver::update_p( void )
{
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        RealType p = 0.0;
        for( IndexType i = 0; i < mesh->edges_per_cell(); i++ ) {
            IndexType edge_F = mesh->edge_for_cell( cell_K, i );
            p += beta.getElement( cell_K, edge_F ) * ( ptrace[ edge_F ] - G_KE( cell_K, edge_F ) * pressure[ cell_K ] );
        }
        p *= pressure[ cell_K ];
        p += F[ cell_K ] + lambda[ cell_K ] * pressure[ cell_K ];
        p /= lambda[ cell_K ] + alpha[ cell_K ] * pressure[ cell_K ];
        pressure[ cell_K ] = p;
    }
    return true;
}

string Solver::getFormattedTime( const float & time )
{
    stringstream ss;
    ss.fill( '0' );
    ss.width( 5 );
    ss.precision( 1 );
    ss << fixed << time;
    return ss.str();
}

bool Solver::run( void )
{
    bool status = init();
    if( ! status ) {
        cerr << "Failed to init the solver." << endl;
        return false;
    }

    int snapshot_period_iter = snapshot_period / tau;
    string snapshot_prefix = string( "out/pressure-vect-gravity-" ) + to_string( mesh_rows ) + "x" + to_string( mesh_cols ) + "-";

    update_auxiliary_vectors();

    for( unsigned i = 0; i * tau < final_time; i++ ) {
        cout << "Time: " << i * tau << endl;

        // make snapshot, starting with initial conditions
        if( i % snapshot_period_iter == 0 ) {
            pressure.save( snapshot_prefix + getFormattedTime( i * tau ) + ".dat" );
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
            break;
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
        cout << "Time: " << final_time << endl;
//        for( IndexType i = 0; i < mesh_rows; i++ ) {
//            for( IndexType j = 0; j < mesh_cols; j++ ) {
//                cout << p[ i * mesh_cols + j ] << " ";
//            }
//            cout << endl;
//        }

        pressure.save( snapshot_prefix + getFormattedTime( final_time ) + ".dat" );
    }
    return status;
}

