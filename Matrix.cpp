/**
 * @file    Matrix.cc
 * @brief   Implementace funkcí a metod pro třídu @ref Matrix.
 */

#include "exceptions.h"
#include "Matrix.h"


Matrix::~Matrix( void )
{
}

IndexType Matrix::getRows( void ) const
{
    return rows;
}

IndexType Matrix::getCols( void ) const
{
    return cols;
}

void Matrix::print( std::ostream & os ) const
{
    for( IndexType i = 0; i < getRows(); i++ ) {
        for( IndexType j = 0; j < getCols(); j++ ) {
            os << getElement( i, j ) << " ";
        }
        os << std::endl;
    }
}

