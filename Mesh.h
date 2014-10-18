#pragma once

class Mesh
{
public:
    virtual ~Mesh( void ) {};

    virtual int num_edges( void ) const = 0;
    virtual int num_cells( void ) const = 0;
    virtual int edges_per_cell( void ) const = 0;

    virtual bool is_inner_edge( int edge ) const = 0;
    virtual bool is_outer_edge( int edge ) const = 0;

    virtual int edge_for_cell( int cell, int edgeOrder ) const = 0;
    virtual int cell_for_edge( int edge, int cellOrder ) const = 0;

    virtual double measure_cell( int cell ) const = 0;
    virtual double measure_edge( int edge ) const = 0;
};

