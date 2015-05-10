#include <iostream>
#include <sstream>

#include <cmath>

#include "Solver.h"

using namespace std;


Solver::Solver( string output_prefix,
                IndexType size_x,
                IndexType size_y,
                RealType time_step,
                RealType time_step_order )
    : output_prefix( output_prefix ),
      mesh_cols( size_x ),
      mesh_rows( size_y ),
      tau( time_step ),
      time_step_order( time_step_order )
{}

bool Solver::allocateVectors( void )
{
    bool status = true;

    // parameters
    status &= permeability.setSize( mesh.num_cells() );
    status &=     porosity.setSize( mesh.num_cells() );
    status &=            F.setSize( mesh.num_cells() );
    status &=           qN.setSize( mesh.num_edges() );
    status &=           pD.setSize( mesh.num_edges() );

    // main variables
    status &= pressure.setSize( mesh.num_cells() );
    status &=   ptrace.setSize( mesh.num_edges() );
    status &=      rhs.setSize( mesh.num_edges() );

    // auxiliary variables
    status &=    alpha.setSize( mesh.num_cells() );
    status &=     beta.setSize( mesh.num_cells(), mesh.num_edges() );
    status &= beta.reserve( 4 * mesh.num_cells() );     // reserve memory to avoid reallocations
    status &=   lambda.setSize( mesh.num_cells() );

    return status;
}

bool Solver::init( void )
{
    area_width = 10;
    area_height = 10;
    mesh.setup( area_width, area_height, mesh_rows, mesh_cols );

    if( ! allocateVectors() ) {
        cerr << "Failed to allocate vectors." << endl;
        return false;
    }

    // parameters
    snapshot_period = 1.0;
    initial_time = 0.0;
    final_time = 30.0;
    grav_y = -9.806;
//    grav_y = 0.0;
    const RealType M = 28.96e-3;
    const RealType R = 8.3144621;
    const RealType T = 300;
    idealGasCoefficient = M / R / T;
    viscosity = 18.6e-6;
    permeability.setAllElements( 1e-10 );
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
//    for( IndexType i = (mesh_rows + 1) * mesh_cols; i < mesh.num_edges(); i++ ) {
//        if( i % (mesh_cols + 1) == 0 )
//            pD[ i ] = 1e5;
//    }
//    // constant (higher) pressure on right border
//    for( IndexType i = (mesh_rows + 1) * mesh_cols; i < mesh.num_edges(); i++ ) {
//        if( i % (mesh_cols + 1) == mesh_cols )
//            pD[ i ] = 1.01e5;
//    }

    // initial conditions
    pressure.setAllElements( 1e5 );
    ptrace.setAllElements( 1e5 );

    hxy = mesh.get_hx() / mesh.get_hy();
    hyx = mesh.get_hy() / mesh.get_hx();
    return true;
}

RealType Solver::G_KE( IndexType cell, IndexType edge )
{
    // left or right
    if( mesh.is_vertical_edge( edge ) )
        return 0.0;

    RealType value = 0.5 * idealGasCoefficient * grav_y * mesh.get_hy();

    // bottom
    if( mesh.get_edge_order( cell, edge ) == 1 )
        return value;

    // top
    return -value;
}

bool Solver::update_auxiliary_vectors( const RealType & time, const RealType & tau )
{
    // depends on tau
    for( IndexType cell = 0; cell < mesh.num_cells(); cell++ ) {
        lambda[ cell ] = porosity[ cell ] * idealGasCoefficient * mesh.cell_volume( cell ) / tau;
    }

    // constant in time
    if( time > initial_time )
        return true;

    for( IndexType cell = 0; cell < mesh.num_cells(); cell++ ) {
        alpha[ cell ] = 0.0;
        for( int i = 0; i < 4; i++ ) {
            IndexType edge = mesh.edge_for_cell( cell, i );
            RealType value = 2 * idealGasCoefficient * permeability[ cell ] / viscosity
                    * (( mesh.is_horizontal_edge(edge) ) ? hxy : hyx);
            beta.setElement( cell, edge, value );
            alpha[ cell ] += value;
        }
    }

    return true;
}

bool Solver::update_main_system( const RealType & time )
{
    for( IndexType indexRow = 0; indexRow < mesh.num_edges(); indexRow++ ) {
        rhs[ indexRow ] = 0.0;

        // inner edge or Neumann boundary
        if( ! mesh.is_dirichlet_boundary( indexRow ) ) {
            for( IndexType i = 0; i < 2; i++ ) {
                IndexType cell = mesh.cell_for_edge( indexRow, i );
                // on Neumann boundary only one term/cell contributes
                if( cell < 0 )
                    continue;

                for( IndexType j = 0; j < 4; j++ ) {
                    IndexType indexColumn = mesh.edge_for_cell( cell, j );

                    RealType B_KEF = - beta.getElement( cell, indexRow ) * pressure[ cell ] * beta.getElement( cell, indexColumn ) * pressure[ cell ] / ( lambda[ cell ] + alpha[ cell ] * pressure[ cell ] );
                    if( indexRow == indexColumn )
                        B_KEF += beta.getElement( cell, indexRow ) * pressure[ cell ];

                    if( ! mesh.is_dirichlet_boundary( indexColumn ) ) {
                        // set main matrix element
                        RealType value = mainMatrix.getElement( indexRow, indexColumn );
                        mainMatrix.setElement( indexRow, indexColumn, value + B_KEF );
                    }
                    else {
                        rhs[ indexRow ] -= B_KEF * pD[ indexColumn ];
                    }
                    // right hand side
                    rhs[ indexRow ] += B_KEF * G_KE( cell, indexColumn ) * pressure[ cell ];
                }

                // right-hand-side
                rhs[ indexRow ] += beta.getElement( cell, indexRow ) * pressure[ cell ] / ( lambda[ cell ] + alpha[ cell ] * pressure[ cell ] ) * ( F[ cell ] + lambda[ cell ] * pressure[ cell ] );
            }
        }
        // Dirichlet boundary
        else {
            mainMatrix.setElement( indexRow, indexRow, 1.0 );
            rhs[ indexRow ] = pD[ indexRow ];
        }
        // Neumann boundary
        if( mesh.is_neumann_boundary( indexRow ) ) {
            rhs[ indexRow ] += qN[ indexRow ];
        }
    }
    return true;
}

bool Solver::update_pressure( void )
{
    for( IndexType cell = 0; cell < mesh.num_cells(); cell++ ) {
        RealType p = 0.0;
        for( IndexType i = 0; i < mesh.edges_per_cell(); i++ ) {
            IndexType edge = mesh.edge_for_cell( cell, i );
            p += beta.getElement( cell, edge ) * ( ptrace[ edge ] - G_KE( cell, edge ) * pressure[ cell ] );
        }
        p *= pressure[ cell ];
        p += F[ cell ] + lambda[ cell ] * pressure[ cell ];
        p /= lambda[ cell ] + alpha[ cell ] * pressure[ cell ];
        pressure[ cell ] = p;
    }
    return true;
}

template< typename T >
string Solver::pad_number( const T & number )
{
    stringstream ss;
    ss.fill( '0' );
    ss.width( 5 );
    ss.precision( 1 );
    ss << fixed << number;
    return ss.str();
}

bool Solver::solve( const RealType & time_start, const RealType & time_stop )
{
    RealType time = time_start;
    bool status = true;

    while( time < time_stop ) {
        RealType current_tau = fmin( tau, time_stop - time );

        cout << "Time: " << time << endl;

        // update auxiliary vectors
        update_auxiliary_vectors( time, current_tau );

        // setSize() clears the matrix
        status = mainMatrix.setSize( mesh.num_edges(), mesh.num_edges() );
        if( ! status ) {
            cerr << "Failed to set size of the main matrix." << endl;
            return false;
        }

        // reserve space to avoid reallocation
        status = mainMatrix.reserve( 7 * ( mesh.num_edges() - mesh.num_neumann_edges() - mesh.num_dirichlet_edges() ) + 4 * mesh.num_neumann_edges() + mesh.num_dirichlet_edges() );
        if( ! status ) {
            cerr << "Failed to reserve space for non-zero elements in the main matrix." << endl;
            return false;
        }

        status = update_main_system( time + current_tau );
        if( ! status ) {
            cerr << "Failed to update the main system." << endl;
            return false;
        }

        status = mainMatrix.linear_solve( ptrace, rhs );
        if( ! status ) {
            cerr << "Failed to solve the main system." << endl;
            return false;
        }
        update_pressure();

        time += current_tau;
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

    // update tau according to mesh refinement
    tau = tau * pow( fmin( mesh.get_hx(), mesh.get_hy() ), time_step_order );
    cout << "Refined time step: " << tau << endl;
    // initialize
    RealType time = initial_time;
    IndexType step = 0;
    const IndexType final_step = ceil( (final_time - initial_time) / snapshot_period );

    // save initial condition
    pressure.save( output_prefix + "-" + pad_number( step ) + ".dat" );

    while( step < final_step ) {
        RealType current_tau = fmin( snapshot_period, final_time - time );

        status = solve( time, time + current_tau );
        if( ! status )
            return false;

        step++;
        time += current_tau;

        // make snapshot
        pressure.save( output_prefix + "-" + pad_number( step ) + ".dat" );
    }

    return true;
}

