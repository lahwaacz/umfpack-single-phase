/**
 * @file    Matrix.hh
 * @brief   Hlavičkový soubor pro třídu @ref Matrix.
 */

#pragma once

#include <iostream>
#include <string>

// TODO: move into config.h
typedef double RealType;
typedef int IndexType;

/**
 * @brief   Abstraktní třída pro matice @ref DenseMatrix a @ref SparseMatrix.
 */
class Matrix
{
protected:
    IndexType rows = 0;
    IndexType cols = 0;

public:
    virtual ~Matrix( void );

    // set/get vector size
    virtual bool setSize( const IndexType rows, const IndexType cols ) = 0;
    IndexType getRows( void ) const;
    IndexType getCols( void ) const;

    // accessors to matrix elements
    virtual bool setElement( const IndexType row, const IndexType col, const RealType & data ) = 0;
    virtual RealType getElement( const IndexType row, const IndexType col ) const = 0;

    // file saving/loading
    virtual bool save( const std::string & file_name ) const = 0;
    virtual bool load( const std::string & file_name ) = 0;

    // simple output
    void print( std::ostream & os = std::cout ) const;
};

