#pragma once

#include <string>

#include "Matrix.h"

class DenseMatrix
    : public Matrix
{
private:
    RealType* _data;

    virtual void _init( void );

public:
    DenseMatrix( IndexType rows, IndexType cols );
    DenseMatrix( DenseMatrix const & m );
    ~DenseMatrix();

    // subscript operators pair
    RealType & operator() ( IndexType row, IndexType col );
    RealType operator() ( IndexType row, IndexType col ) const;
    // accessors to matrix elements
    virtual bool set( IndexType row, IndexType col, RealType data );
    virtual RealType get( IndexType row, IndexType col ) const;

    // file saving/loading
    virtual bool save( const std::string & file_name ) const;
    virtual bool load( const std::string & file_name );
};
