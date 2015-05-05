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
    void setup( double area_width, double area_height, int rows, int columns );

    virtual int num_edges( void ) const;
    virtual int num_cells( void ) const;
    virtual int edges_per_cell( void ) const;

    virtual bool is_inner_edge( int edge ) const;
    virtual bool is_outer_edge( int edge ) const;

    virtual int edge_for_cell( int cell, int edgeOrder ) const;
    virtual int cell_for_edge( int edge, int cellOrder ) const;
    int get_edge_order( int cell, int edge ) const;

    virtual double measure_cell( int cell ) const;
    virtual double measure_edge( int edge ) const;

    double get_dx( void ) const { return _delta_x; };
    double get_dy( void ) const { return _delta_y; };

    bool is_horizontal_edge( int edge ) const;
    bool is_vertical_edge( int edge ) const;

    // TODO: refactoring (specific to problem)
    bool is_neumann_boundary( int edge ) const;
    bool is_dirichlet_boundary( int edge ) const;

    // TODO: refactoring (very specific to problem)
    int num_neumann_edges( void ) const;
    int num_dirichlet_edges( void ) const;
};
