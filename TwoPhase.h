#pragma once

#include "RectangularMesh.h"
#include "Vector.h"
#include "DenseMatrix.h"
#include "SparseMatrix.h"

class TwoPhaseSolver
{
private:
    const int numberOfPhases = 2;
    RectangularMesh* mesh = nullptr;
    // parameters
    RealType area_width;
    RealType area_height;
    IndexType mesh_cols;
    IndexType mesh_rows;
    RealType tau;
    unsigned max_iterations;
    RealType snapshot_period;
    RealType grav_y;
    RealType* M;
    RealType R;
    RealType T;
    RealType permeability;
    RealType viscosity;
    Vector porosity;
    Vector F;
    // boundary conditions
    Vector qN;
    Vector pD;
    // variables
    Vector saturation;
    Vector pressure;
    Vector ptrace;
    // main system matrix + right-hand-side
    SparseMatrix mainMatrix;
    Vector rhs;
    // auxiliary variables
    Vector kappa;
    DenseMatrix* AK_array = nullptr;
    DenseMatrix* BKE_array = nullptr;
    DenseMatrix* CK_array = nullptr;

    RealType dxy;
    RealType dyx;

    // auxiliary methods
    bool allocateVectors( void );
    RealType G_KE( int phaseIndex, IndexType K, IndexType E );
    RealType SwDerivative( IndexType cell );
    void set_AK( DenseMatrix & A, IndexType K );
    RealType massMatrixElement( int phaseIndex, IndexType k, IndexType E );
    void set_BKE( DenseMatrix & B, IndexType K, IndexType E );
    bool update_local_matrices( void );
    bool update_main_system( void );
    bool update_p( void );

public:
    ~TwoPhaseSolver( void );

    bool init( void );
    bool run( void );
};

