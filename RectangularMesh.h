#pragma once

#include "Mesh.h"

class RectangularMesh
    : public Mesh
{
private:
    int _rows = 0;
    int _cols = 0;
    double _delta_x = 0.0;
    double _delta_y = 0.0;

public:
    RectangularMesh( int area_width, int area_height, int rows, int columns );

    virtual int num_edges( void ) const;
    virtual int num_cells( void ) const;
    virtual int edges_per_cell( void ) const;

    virtual bool is_inner_edge( int edge ) const;
    virtual bool is_outer_edge( int edge ) const;

    virtual int edge_for_cell( int cell, int edgeOrder ) const;
    virtual int cell_for_edge( int edge, int cellOrder ) const;

    virtual double measure_cell( int cell ) const;
    virtual double measure_edge( int edge ) const;
};
