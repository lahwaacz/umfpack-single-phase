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
    DenseMatrix** Ak = nullptr;
    Vector* alpha = nullptr;
    SparseMatrix* alpha_KE = nullptr;
    Vector* lambda = nullptr;

    RealType dxy;
    RealType dyx;

public:
    Solver( void );
    ~Solver( void );

    void init( void );
    void set_Ak( DenseMatrix & Ak, IndexType k );
    void update_rhs( void );
    void update_p( void );
    void set_main_matrix( void );
    void run( void );
};

