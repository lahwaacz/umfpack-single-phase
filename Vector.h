#pragma once

#include <string>

// TODO: required only for IndexType and RealType
#include "Matrix.h"

class Vector
{
private:
    IndexType _rows;
    RealType* _data;
public:
    Vector( IndexType rows );
    Vector( IndexType rows, RealType init_value );
    Vector( Vector const & v );
    ~Vector();

    // subscript operators pair
    RealType & operator[] ( IndexType row );
    RealType operator[] ( IndexType row ) const;

    // accessor to vector size
    IndexType rows( void ) const { return _rows; };

    // file saving/loading
    bool save( const std::string & file_name ) const;
    bool load( const std::string & file_name );
};

RealType norm( const Vector & v );
