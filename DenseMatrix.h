#pragma once

#include <string>

#include "Matrix.h"

class DenseMatrix
    : public Matrix
{
private:
    RealType* data = nullptr;

    // TODO: split into Array.h
    bool allocateMemory( RealType* & data, const IndexType & size );
    bool freeMemory( RealType* & data );

public:
    ~DenseMatrix();

    virtual bool setSize( const IndexType rows, const IndexType cols );

    // accessors to data storage
    const RealType* getData( void ) const;
    RealType* getData( void );

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
