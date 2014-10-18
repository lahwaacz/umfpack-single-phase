/**
 * @file    Matrix.cc
 * @brief   Implementace funkcí a metod pro třídu @ref Matrix.
 */

#include "exceptions.h"
#include "Matrix.h"


/**
 * Konstruktor - vytvoří obdélníkovou matici zadaných rozměrů.
 */
Matrix::Matrix( IndexType rows, IndexType cols )
    : _rows(rows)
    , _cols(cols)
{
    if( rows == 0 || cols == 0 )
        throw BadIndex("Matrix constructor called with 0 size");
}

/**
 * Konstruktor - vytvoří čtvercovou matici zadaného řádu.
 */
Matrix::Matrix( IndexType order )
    : _rows(order)
    , _cols(order)
{
    if( order == 0 )
        throw BadIndex("Matrix constructor called with 0 size");
}

