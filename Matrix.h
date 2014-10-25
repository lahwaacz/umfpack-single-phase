/**
 * @file    Matrix.hh
 * @brief   Hlavičkový soubor pro třídu @ref Matrix.
 */

#pragma once

#include <iostream>
#include <string>

typedef double RealType;
typedef int IndexType;

/**
 * @brief   Abstraktní třída pro matice @ref StdMatrix a @ref Matrix.
 */
class Matrix
{
protected:
    const IndexType _rows;  ///< počet řádků matice
    const IndexType _cols;  ///< počet sloupců matice

    // initialization method (to be called from constructor)
    virtual void _init( void ) = 0;

public:
    Matrix( IndexType rows, IndexType cols );
    Matrix( IndexType order );
    virtual ~Matrix( void );

    // accessors to matrix elements
    virtual bool set( IndexType row, IndexType col, RealType data ) = 0;
    virtual RealType get( IndexType row, IndexType col ) const = 0;

    // accessors to matrix size
    IndexType rows( void ) const { return _rows; };
    IndexType cols( void ) const { return _cols; };

    // file saving/loading
    virtual bool save( const std::string & file_name ) const = 0;
    virtual bool load( const std::string & file_name ) = 0;

    // simple output
    void print( std::ostream & os = std::cout ) const;
};

