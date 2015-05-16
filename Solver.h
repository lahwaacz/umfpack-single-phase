#pragma once

#include <string>

#include "RectangularMesh.h"
#include "Vector.h"
#include "SparseMatrix.h"

class Solver
{
private:
    // parameters configurable from command line
    std::string output_prefix;
    IndexType mesh_cols;
    IndexType mesh_rows;
    RealType tau;
    RealType time_step_order;

    RectangularMesh mesh;
    // parameters
    RealType area_width;
    RealType area_height;
    RealType snapshot_period;
    RealType initial_time;
    RealType final_time;
    RealType grav_y;
    RealType idealGasCoefficient;
    RealType permeability;
    RealType viscosity;
    Vector porosity;
    Vector F;
    // boundary conditions
    Vector qN;
    Vector pD;
    // variables
    Vector pressure;
    Vector ptrace;
    // main system matrix + right-hand-side
    SparseMatrix mainMatrix;
    Vector rhs;
    // auxiliary variables
    Vector alpha;
    SparseMatrix beta;
    Vector lambda;

    RealType dxy;
    RealType dyx;

    // auxiliary methods
    bool allocateVectors( void );
    bool init( void );
    RealType G_KE( IndexType cell_K, IndexType edge_E );
    bool update_auxiliary_vectors( const RealType & time, const RealType & tau );
    bool update_main_system( const RealType & time );
    bool update_p( void );
    bool solve( const RealType & time_start, const RealType & time_stop );

    template< typename T >
    std::string pad_number( const T & number );

public:
    Solver( std::string output_prefix,
            IndexType size_x,
            IndexType size_y,
            RealType time_step,
            RealType time_step_order );

    bool run( void );
};

