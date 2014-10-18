#include "RectangularMesh.h"

// cell numbering: by rows, left to right
// edge numbering: by rows, first horizontal and then vertical

RectangularMesh::RectangularMesh( int area_width, int area_height, int rows, int columns )
    : _rows(rows), _cols(columns)
{
    _delta_x = area_width / columns;
    _delta_y = area_height / rows;
}

int RectangularMesh::num_edges( void ) const
{
    return 2 * _rows * _cols + _rows + _cols;
}

int RectangularMesh::num_cells( void ) const
{
    return _rows * _cols;
}

int RectangularMesh::edges_per_cell( void ) const
{
    return 4;
}

bool RectangularMesh::is_inner_edge( int edge ) const
{
    // test horizontal edges
    if( edge < _cols || (_rows * _cols <= edge && edge < (_rows + 1) * _cols) )
        return false;
    // test vertical edges
    edge -= (_rows + 1) * _cols;
    if( edge >= 0 && (edge % (_cols + 1) == 0 || edge % (_cols + 1) == _cols) )
        return false;
    return true;
}

bool RectangularMesh::is_outer_edge( int edge ) const
{
    return !is_inner_edge(edge);
}

/*
 * int cell - cell index
 * int edgeOrder - for which edge the index should be returned:
 *          0 - top
 *          1 - right
 *          2 - bottom
 *          3 - left
 */
int RectangularMesh::edge_for_cell( int cell, int edgeOrder ) const
{
    // start with cell coordinates
    int row = cell / _cols;
    int col = cell % _cols;

    // increment for right/bottom edge
    if( edgeOrder == 1 )
        col++;
    if( edgeOrder == 2 )
        row++;

    if( edgeOrder == 0 || edgeOrder == 2 )
        // top/bottom: index in (_rows+1) by _cols row-major matrix
        return row * _cols + col;
    // left/right: index in _rows by (_cols+1) row-major matrix, plus number of horizontal edges
    return (_rows + 1) * _cols + row * (_cols + 1) + col;
}

/*
 * int edge - edge index
 * int cellOrder - for which adjacent cell the index should be returned:
 *          0 - top/left (horizontal/vertical edge)
 *          1 - bottom/right
 * returns: int >= 0 ... valid cell index
 *          int  < 0 ... error for outer edge
 */
int RectangularMesh::cell_for_edge( int edge, int cellOrder ) const
{
    int row = 0;
    int col = 0;

    // horizontal edge
    if( edge < (_rows + 1) * _cols ) {
        row = edge / _cols;
        col = edge % _cols;

        if( cellOrder == 0 )
            row--;

        // cell index for outer edge might get out of bounds
        if( row < 0 || row >= _rows )
            return -1;
    }
    // vertical edge
    else {
        edge -= (_rows + 1) * _cols;
        row = edge / (_cols + 1);
        col = edge % (_cols + 1);

        if( cellOrder == 0 )
            col--;

        // cell index for outer edge might get out of bounds
        if( col < 0 || col >= _cols )
            return -1;
    }

    return row * _cols + col;
}

double RectangularMesh::measure_cell( int cell ) const
{
    return _delta_x * _delta_y;
}

double RectangularMesh::measure_edge( int edge ) const
{
    // horizontal edge
    if( edge < (_rows + 1) * _cols )
        return _delta_x;
    // vertical edge
    return _delta_y;
}

