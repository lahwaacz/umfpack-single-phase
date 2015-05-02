#include <cmath>

#include "Vector.h"

// source TNL headers (used only as export format)
#include <core/vectors/tnlVector.h>

RealType Vector::norm( void ) const
{
    RealType s = 0.0;
    for( IndexType i = 0; i < getSize(); i++)
    {
        s += getElement( i ) * getElement( i );
    }
    return sqrt( s );
}

bool Vector::save_tnl( const std::string & filename ) const
{
    tnlVector< RealType, tnlHost, IndexType > v;
    if( ! v.setSize( this->getSize() ) )
        return false;
    for( IndexType i = 0; i < this->getSize(); i++ ) {
        v[ i ] = this->operator[]( i );
    }
    return v.save( filename.c_str() );
}

