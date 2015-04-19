#pragma once

#include <string>

#include "RectangularMesh.h"
#include "Vector.h"
#include "DenseMatrix.h"
#include "SparseMatrix.h"

class Solver
{
private:
    RectangularMesh* mesh = nullptr;
    // parameters
    RealType area_width;
    RealType area_height;
    IndexType mesh_cols;
    IndexType mesh_rows;
    float tau;
    float final_time;
    float snapshot_period;
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
    RealType G_KE( IndexType cell_K, IndexType edge_E );
    std::string getFormattedTime( const float & time );

public:
//    Solver( void );
    ~Solver( void );

    bool init( void );
    bool update_auxiliary_vectors( void );
    bool update_main_system( void );
    bool update_p( void );
    bool run( void );
};

