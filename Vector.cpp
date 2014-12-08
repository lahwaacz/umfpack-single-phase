#include <cmath>

#include "Vector.h"

RealType Vector::norm( void ) const
{
    RealType s = 0.0;
    for( IndexType i = 0; i < getSize(); i++)
    {
        s += getElement( i ) * getElement( i );
    }
    return sqrt( s );
}

