#include "Solver.h"

Solver::Solver( void )
{
}

Solver::~Solver( void )
{
    delete mesh;
}

void Solver::init( int area_width, int area_height, int mesh_rows, int mesh_cols )
{
    // TODO: load problem parameters from config file
    mesh = new RectangularMesh( area_width, area_height, mesh_rows, mesh_cols );

    // TODO: load porosity (element-wise constant)
    porosity = new Vector( mesh->num_cells(), 10e-10 );

    // TODO: load initial conditions
    p = new Vector( mesh->num_cells() );
    ptrace = new Vector( mesh->num_edges() );

    // TODO: calculate Ak matrix elements (sparse)
}

