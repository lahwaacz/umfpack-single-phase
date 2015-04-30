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
    status &= porosity.setSize( mesh.num_cells() );
    status &=        F.setSize( mesh.num_cells() );

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
    area_width = 100;
    area_height = 20;
    mesh.setup( area_width, area_height, mesh_rows, mesh_cols );

    if( ! allocateVectors() ) {
        cerr << "Failed to allocate vectors." << endl;
        return false;
    }

    // parameters
    snapshot_period = 500.0;
    initial_time = 1e4;
    final_time = 1e5;
    grav_y = 0.0;
    const RealType M = 1.0;
    const RealType R = 1.0;
    const RealType T = 1.0;
    idealGasCoefficient = M / R / T;
    permeability = 1.0;
    viscosity = 0.5;
    porosity.setAllElements( 1.0 );
    F.setAllElements( 0.0 );

    // initial conditions
    for( IndexType indexCell = 0; indexCell < mesh.num_cells(); indexCell++ ) {
        const IndexType i = indexCell % mesh_cols;
        const RealType x = - area_width / 2.0 + (i + 0.5) * mesh.get_dx();
        pressure[ indexCell ] = barenblatt( x, initial_time );
    }

    ptrace.setAllElements( 1.0 );

    dxy = mesh.get_dx() / mesh.get_dy();
    dyx = mesh.get_dy() / mesh.get_dx();
    return true;
}

RealType Solver::G_KE( IndexType cell_K, IndexType edge_E )
{
    // left or right
    if( mesh.is_vertical_edge( edge_E ) )
        return 0.0;

    RealType value = 0.5 * idealGasCoefficient * grav_y * mesh.get_dy();

    // bottom
    if( mesh.get_edge_order( cell_K, edge_E ) == 1 )
        return value;

    // top
    return -value;
}

RealType Solver::barenblatt( const RealType & x, const RealType & time )
{
    const IndexType m = 2;
    const RealType k = 1.0 / (m + 1);
    RealType B = 1 - k * (m - 1) / (2.0 * m) * x * x / pow( time, 2 * k );
    if( B <= 0.0 )
        return 0.0;
    return pow( time, -k ) * pow( B, 1.0 / (m - 1) );
}

RealType Solver::get_dirichlet_value( const IndexType & indexEdge, const RealType & time )
{
    const IndexType i = indexEdge % (mesh_cols + 1);
    const RealType x = - area_width / 2.0 + i * mesh.get_dx();
    return barenblatt( x, initial_time + time );
}

RealType Solver::get_neumann_value( const IndexType & indexEdge, const RealType & time )
{
    return 0.0;
}

bool Solver::update_auxiliary_vectors( const RealType & time, const RealType & tau )
{
    // depends on tau
    for( IndexType cell_K = 0; cell_K < mesh.num_cells(); cell_K++ ) {
        lambda[ cell_K ] = porosity[ cell_K ] * idealGasCoefficient * mesh.measure_cell( cell_K ) / tau;
    }

    // constant in time
    if( time > initial_time )
        return true;

    for( IndexType cell_K = 0; cell_K < mesh.num_cells(); cell_K++ ) {
        alpha[ cell_K ] = 0.0;
        for( int i = 0; i < 4; i++ ) {
            IndexType edge_E = mesh.edge_for_cell( cell_K, i );
            RealType value = 2 * idealGasCoefficient * permeability / viscosity
                    * (( mesh.is_horizontal_edge(edge_E) ) ? dxy : dyx);
            beta.setElement( cell_K, edge_E, value );
            alpha[ cell_K ] += value;
        }
    }

    return true;
}

bool Solver::update_main_system( const RealType & time )
{
    // set main system elements
    for( IndexType edge_E = 0; edge_E < mesh.num_edges(); edge_E++ ) {
        rhs[ edge_E ] = 0.0;

        // inner edge or Neumann boundary
        if( ! mesh.is_dirichlet_boundary( edge_E ) ) {
            for( IndexType i = 0; i < 2; i++ ) {
                IndexType cell_K = mesh.cell_for_edge( edge_E, i );
                // on Neumann boundary only one term/cell contributes
                if( cell_K < 0 )
                    continue;

                for( IndexType j = 0; j < 4; j++ ) {
                    IndexType edge_F = mesh.edge_for_cell( cell_K, j );

                    RealType A_KEF = - beta.getElement( cell_K, edge_E ) * pressure[ cell_K ] * beta.getElement( cell_K, edge_F ) * pressure[ cell_K ] / ( lambda[ cell_K ] + alpha[ cell_K ] * pressure[ cell_K ] );
                    if( edge_E == edge_F )
                        A_KEF += beta.getElement( cell_K, edge_E ) * pressure[ cell_K ];

                    if( ! mesh.is_dirichlet_boundary( edge_F ) ) {
                        // set main matrix element
                        RealType value = mainMatrix.getElement( edge_E, edge_F );
                        mainMatrix.setElement( edge_E, edge_F, value + A_KEF );
                    }
                    else {
                        rhs[ edge_E ] -= A_KEF * get_dirichlet_value( edge_F, time );
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
            rhs[ edge_E ] = get_dirichlet_value( edge_E, time );
        }
        // Neumann boundary
        if( mesh.is_neumann_boundary( edge_E ) ) {
            rhs[ edge_E ] += get_neumann_value( edge_E, time );
        }
    }
    return true;
}

bool Solver::update_p( void )
{
    for( IndexType cell_K = 0; cell_K < mesh.num_cells(); cell_K++ ) {
        RealType p = 0.0;
        for( IndexType i = 0; i < mesh.edges_per_cell(); i++ ) {
            IndexType edge_F = mesh.edge_for_cell( cell_K, i );
            p += beta.getElement( cell_K, edge_F ) * ( ptrace[ edge_F ] - G_KE( cell_K, edge_F ) * pressure[ cell_K ] );
        }
        p *= pressure[ cell_K ];
        p += F[ cell_K ] + lambda[ cell_K ] * pressure[ cell_K ];
        p /= lambda[ cell_K ] + alpha[ cell_K ] * pressure[ cell_K ];
        pressure[ cell_K ] = p;
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
        update_p();

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
    tau = tau * pow( fmin( mesh.get_dx(), mesh.get_dy() ), time_step_order );
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

