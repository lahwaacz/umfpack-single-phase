#pragma once

#include <string>

#include "Matrix.h"
#include "Array.h"

class DenseMatrix
    : public Matrix, private Array
{
private:
    // setSize( const IndexType ) must not be public
    using Array::setSize;

public:
    virtual bool setSize( const IndexType rows, const IndexType cols );

    // expose protected member of Array
    using Array::getData;

    // accessors to matrix elements
    virtual bool setElement( const IndexType row, const IndexType col, const RealType & data );
    virtual RealType getElement( const IndexType row, const IndexType col ) const;

    // subscript operators pair
    RealType & operator() ( const IndexType row, const IndexType col );
    RealType operator() ( const IndexType row, const IndexType col ) const;

    // set all elements to the same value
    void setAllElements( const RealType & value );

    // file saving/loading
    virtual bool save( const std::string & file_name ) const;
    virtual bool load( const std::string & file_name );
};
