#pragma once

#include <iostream>
#include <string>

// TODO: required only for IndexType and RealType
#include "Matrix.h"

class Vector
{
private:
    IndexType size;
    RealType* data;

    bool allocateMemory( RealType* & data, const IndexType & size );
    bool freeMemory( RealType* & data );
public:
    Vector();
    ~Vector();

    // set/get vector size
    bool setSize( const IndexType size );
    IndexType getSize( void ) const;

    // accessors to data storage
    const RealType* getData() const;
    RealType* getData();

    // element setters/getters
    void setElement( const IndexType index, const RealType & value );
    RealType getElement( IndexType index ) const;

    // subscript operators pair
    RealType & operator[] ( IndexType row );
    RealType operator[] ( IndexType row ) const;

    // set all elements to the same value
    void setAllElements( const RealType & value );

    // file saving/loading
    bool save( const std::string & file_name ) const;
    bool load( const std::string & file_name );

    // simple output
    void print( std::ostream & os = std::cout ) const;
};

RealType norm( const Vector & v );
