#pragma once

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
    RealType tau;
    unsigned max_iterations;
    RealType snapshot_period;
    RealType grav_y;
    RealType M;
    RealType R;
    RealType T;
    RealType permeability;
    RealType viscosity;
    Vector* porosity = nullptr;
    Vector* F = nullptr;
    // boundary conditions
    Vector* qN = nullptr;
    Vector* pD = nullptr;
    // variables
    Vector* p = nullptr;
    Vector* ptrace = nullptr;
    // main system matrix + right-hand-side
    SparseMatrix* mainMatrix = nullptr;
    Vector* rhs = nullptr;
    // auxiliary variables
    Vector* alpha = nullptr;
    SparseMatrix* beta = nullptr;
    Vector* lambda = nullptr;

    RealType dxy;
    RealType dyx;

    // auxiliary methods
    RealType G_KE( IndexType cell_K, IndexType edge_E );
    void set_Ak( DenseMatrix & Ak, IndexType k );

public:
    Solver( void );
    ~Solver( void );

    void init( void );
    void update_main_system( void );
    void update_p( void );
    void run( void );
};

