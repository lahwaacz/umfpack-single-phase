#include <iostream>
#include <cmath>

#include "TwoPhase.h"
#include "SharedVector.h"

using namespace std;


TwoPhaseSolver::~TwoPhaseSolver( void )
{
    delete mesh;
    delete[] AK_array;
    delete[] BKE_array;
    delete[] CK_array;
}

bool TwoPhaseSolver::allocateVectors( void )
{
    bool status = true;

    // parameters
    status &=   porosity.setSize( mesh->num_cells() );
    status &=          F.setSize( mesh->num_cells() );
    status &=         qN.setSize( mesh->num_edges() * numberOfPhases );
    status &=         pD.setSize( mesh->num_edges() * numberOfPhases );

    // main variables
    status &= saturation.setSize( mesh->num_cells() * numberOfPhases );
    status &=   pressure.setSize( mesh->num_cells() * numberOfPhases );
    status &=     ptrace.setSize( mesh->num_edges() * numberOfPhases );
    status &=        rhs.setSize( mesh->num_edges() * numberOfPhases );

    // auxiliary variables
    // TODO
    AK_array = new DenseMatrix[ mesh->num_cells() ];
    status &= (bool) AKE_array;
    BKE_array = new DenseMatrix[ mesh->num_cells() * 4 ];
    status &= (bool) BKE_array;
    CK_array = new DenseMatrix[ mesh->num_cells() ];
    status &= (bool) CK_array;

    return status;
}

bool TwoPhaseSolver::init( void )
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
    RealType M[ numberOfPhases ] = { 18.0, 28.96 };
    R = 8.3144621;
    T = 300;
    // TODO: set kappa
    permeability = 1e-10;
    RealType viscosity[ numberOfPhases ] = { 0.89e-3, 18.6e-6 };
    porosity.setAllElements( 0.4 );
    F.setAllElements( 0.0 );

    // TODO: use sparse vectors
    // boundary conditions
    qN.setAllElements( 0.0 );
    pD.setAllElements( 0.0 );
    // gradient on Dirichlet boundary
    for( IndexType i = 0; i < mesh_cols; i++ )
        pD[ i ] = 1e5 + 1e3 / mesh_cols * (i + 1);

    // initial conditions
    // TODO: set initial saturation
    pressure.setAllElements( 1e5 );
    ptrace.setAllElements( 1e5 );

    dxy = mesh->get_dx() / mesh->get_dy();
    dyx = mesh->get_dy() / mesh->get_dx();
    return true;
}

RealType TwoPhaseSolver::G_KE( int phaseIndex, IndexType K, IndexType E )
{
    // left or right
    if( mesh->is_vertical_edge( E ) )
        return 0.0;

    RealType value = 0.5 * pressure[ phaseIndex * mesh->num_cells() + K ] * kappa[ phaseIndex ] * grav_y * mesh->get_dy();

    // bottom
    if( mesh->get_edge_order( K, E ) == 1 )
        return value;

    // top
    return -value;
}

RealType TwoPhaseSolver::SwDerivative( IndexType cell )
{
    // p_c = p_n - p_w
    RealType pc = pressure[ cell * numberOfPhases + 1 ] - pressure[ cell * numberOfPhases ];
    // TODO: find correct values
    const RealType pd = 1.0;
    const RealType lambda = 1.0;
    return pow( pc / pd, -lambda );
}

void TwoPhaseSolver::set_AK( DenseMatrix & A, IndexType K )
{
    SharedVector _pressure;
    SharedVector _saturation;
    const RealType SwDer = SwDerivative( K );

    for( int i = 0; i < numberOfPhases; i++ ) {
        _pressure.bind( pressure[ i * mesh->num_cells() ], mesh->num_cells() );
        _saturation.bind( saturation[ i * mesh->num_cells() ], mesh->num_cells() );

        // TODO: density of incompressible fluid is NOT pressure * kappa
        RealType value = kappa[ i ] * _pressure[ K ] * SwDer;
        A( i, i ) = _saturation[ K ] * kappa[ i ] - value;
        A( i, 1 - i ) = value;
    }
}

RealType TwoPhaseSolver::massMatrixElement( int phaseIndex, IndexType K, IndexType E )
{
    // Mass Matrix on element K is the matrix subjected to mass lumping
    // this method returns its element belonging to edge E
    
    // TODO: this must be multiplied by lambda_T (total mobility)
    return 6 * pressure[ K ] * kappa[ phaseIndex ] * permeability / viscosity[ phaseIndex ]
                    * (( mesh->is_horizontal_edge(E) ) ? dxy : dyx);
}

void TwoPhaseSolver::set_BKE( DenseMatrix & B, IndexType K, IndexType E )
{
    for( int phaseIndex = 0; phaseIndex < numberOfPhases; phaseIndex++ ) {
        B( phaseIndex, phaseIndex ) = f_KE_upwind( phaseIndex, K, E ) * massMatrixElement( phaseIndex, K, E );
    }
}

bool TwoPhaseSolver::update_local_matrices( void )
{
    bool status = true;
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        // FVM part
        status = AK_array[ cell_K ].setSize( numberOfPhases, numberOfPhases );
        if( ! status ) {
            cerr << "Failed to set size of local matrix AK[ " << cell_K << " ]." << endl;
            return false;
        }
        set_AK( AK_array[ cell_K ], cell_K );

        // FEM part
        for( int i = 0; i < 4; i++ ) {
            IndexType edge_E = mesh->edge_for_cell( cell_K, i );
            status = BKE_array[ cell_K * 4 + i ].setSize( numberOfPhases, numberOfPhases );
            if( ! status ) {
                cerr << "Failed to set size of local matrix BKE[ " << cell_K << ", " << edge_E << " ]." << endl;
                return false;
            }
            set_BKE( BKE_array[ cell_K * 4 + i ], cell_K, edge_E );
        }

        // main local matrix
        status = CK_array[ cell_K ].setSize( numberOfPhases, numberOfPhases );
        if( ! status ) {
            cerr << "Failed to set size of local matrix CK[ " << cell_K << " ]." << endl;
            return false;
        }
        // TODO: avoid copy assignment
        CK_array[ cell_K ] = porosity[ cell_K ] / tau * AK_array[ cell_K ]
                            + 1 / mesh->measure_cell( cell_K ) * BKE_array[ cell_K ];
        // TODO: implement matrix inversion
        // TODO: needs checking for regularity
        CK_array[ cell_K ].invert();
    }
    return true;
}

bool TwoPhaseSolver::update_main_system( void )
{
    // set main system elements
    for( IndexType edge_E = 0; edge_E < mesh->num_edges(); edge_E++ ) {
        SharedVector _rhs = SharedVector( rhs[ edge_E * numberOfPhases ], numberOfPhases );
        _rhs.setAllElements( 0.0 );

        // inner edge or Neumann boundary
        // TODO: need to separate for phases
        if( ! mesh->is_dirichlet_boundary( edge_E ) ) {
            for( IndexType i = 0; i < 2; i++ ) {
                IndexType cell_K = mesh->cell_for_edge( edge_E, i );
                // on Neumann boundary only one term/cell contributes
                if( cell_K < 0 )
                    continue;

                for( IndexType j = 0; j < 4; j++ ) {
                    IndexType edge_F = mesh->edge_for_cell( cell_K, j );
                    DenseMatrix & B_KE = BKE_array[ cell_K * 4 + mesh->get_edge_order( cell_K, edge_E ) ];
                    DenseMatrix & B_KF = BKE_array[ cell_K * 4 + j ];

                    DenseMatrix M_KEF;
                    if( ! M_KEF.setSize( numberOfPhases, numberOfPhases ) ) {
                        cerr << "Failed to set size of local matrix M_KEF." << endl;
                        return false;
                    }
                    // TODO: this should be separate method
                    // TODO: avoid copy assignment
                    M_KEF = 1 / mesh->measure_cell( cell_K ) * B_KE * CK[ cell_K ] * B_KF;
                    if( edge_E == edge_F )
                        M_KEF += B_KE;

                    // set main matrix element
                    // FIXME: this might not ensure symmetricity due to upwind
                    if( ! mesh->is_dirichlet_boundary( edge_F ) ) {
                        // set main matrix block
                        // TODO: universal method to set matrix block?
                        mainMatrix.addElement( edge_E * numberOfPhases, edge_F * numberOfPhases,
                                               M_KEF( 0, 0 ) );
                        mainMatrix.addElement( edge_E * numberOfPhases, edge_F * numberOfPhases + 1,
                                               M_KEF( 0, 1 ) );
                        mainMatrix.addElement( edge_E * numberOfPhases + 1, edge_F * numberOfPhases,
                                               M_KEF( 1, 0 ) );
                        mainMatrix.addElement( edge_E * numberOfPhases + 1, edge_F * numberOfPhases + 1,
                                               M_KEF( 1, 1 ) );
                    }
                    else {
                        SharedVector _pD = SharedVector( pD[ edge_F * numberOfPhases ], numberOfPhases );
                        // TODO: avoid copy assignment
                        _rhs -= M_KEF * _pd;
                    }
                    // right hand side
                    // TODO: avoid copy assignment
                    _rhs += M_KEF * G_KE( cell_K, edge_F );
                }

                // right-hand-side
                // TODO
                rhs[ edge_E ] += b_KE.getElement( cell_K, edge_E ) / ( lambda[ cell_K ] + b_K[ cell_K ] ) * ( F[ cell_K ] + lambda[ cell_K ] * pressure[ cell_K ] );
            }
        }
        // Dirichlet boundary
        // TODO: need to separate for phases
        else {
            mainMatrix.setElement( edge_E, edge_E, 1.0 );
            rhs[ edge_E ] = pD[ edge_E ];
        }
        // Neumann boundary
        // TODO: need to separate for phases
        if( mesh->is_neumann_boundary( edge_E ) ) {
            rhs[ edge_E ] += qN[ edge_E ];
        }
    }
    return true;
}

bool TwoPhaseSolver::update_p( void )
{
    // TODO
    for( IndexType cell_K = 0; cell_K < mesh->num_cells(); cell_K++ ) {
        pressure[ cell_K ] = F[ cell_K ] + lambda[ cell_K ] * pressure[ cell_K ];
        for( IndexType i = 0; i < mesh->edges_per_cell(); i++ ) {
            IndexType edge_F = mesh->edge_for_cell( cell_K, i );
            pressure[ cell_K ] += b_KE.getElement( cell_K, edge_F ) * ( ptrace[ edge_F ] - G_KE( cell_K, edge_F ) );
        }
        pressure[ cell_K ] /= lambda[ cell_K ] + b_K[ cell_K ];
    }
    return true;
}

bool TwoPhaseSolver::run( void )
{
    bool status = init();
    if( ! status ) {
        cerr << "Failed to init the solver." << endl;
        return false;
    }

    int snapshot_period_iter = snapshot_period / tau;
    string snapshot_prefix = string( "out/pressure-vect-gravity-" ) + to_string( mesh_rows ) + "x" + to_string( mesh_cols ) + "-";

    for( unsigned i = 0; i < max_iterations; i++ ) {
        cout << "Time: " << i * tau << endl;

        // make snapshot, starting with initial conditions
        if( i % snapshot_period_iter == 0 ) {
            pressure.save( snapshot_prefix + to_string( i * tau ).substr( 0, 3 ) + ".dat" );
//            for( IndexType i = 0; i < mesh_rows; i++ ) {
//                for( IndexType j = 0; j < mesh_cols; j++ ) {
//                    cout << pressure[ i * mesh_cols + j ] << " ";
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
        // FIXME: the matrix is no longer filled row-by-row, so it is necessary to specify lengths of each row
        status = mainMatrix.reserve( 7 * ( mesh->num_edges() - mesh->num_neumann_edges() - mesh->num_dirichlet_edges() ) + 4 * mesh->num_neumann_edges() + mesh->num_dirichlet_edges() );
        if( ! status ) {
            cerr << "Failed to reserve space for non-zero elements in the main matrix." << endl;
            break;
        }

        status = update_local_matrices();
        if( ! status ) {
            cerr << "Failed to update local matrices." << endl;
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
        cout << "Time: " << max_iterations * tau << endl;
//        for( IndexType i = 0; i < mesh_rows; i++ ) {
//            for( IndexType j = 0; j < mesh_cols; j++ ) {
//                cout << pressure[ i * mesh_cols + j ] << " ";
//            }
//            cout << endl;
//        }

        pressure.save( snapshot_prefix + to_string( max_iterations * tau ).substr( 0, 3 ) + ".dat" );
    }
    return status;
}

