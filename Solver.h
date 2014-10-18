#pragma once

#include "RectangularMesh.h"
#include "Vector.h"

class Solver
{
private:
    RectangularMesh* mesh = nullptr;
    // parameters
    Vector* porosity = nullptr;
    // variables
    Vector* p = nullptr;
    Vector* ptrace = nullptr;
public:
    Solver( void );
    ~Solver( void );

    void init( int area_width, int area_height, int mesh_rows, int mesh_cols );
};

