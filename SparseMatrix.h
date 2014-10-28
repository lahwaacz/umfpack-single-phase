/**
 * @file    SparseMatrix.hh
 * @brief   Hlavičkový soubor pro třídu @ref SparseMatrix.
 */

#pragma once

#include <vector>
#include <string>

#include "Matrix.h"
#include "Vector.h"


/**
 * @brief   Řídká matice, prvky uloženy ve formátu <a href="http://netlib.org/linalg/html_templates/node91.html">CSR</a>.
 */
class SparseMatrix
    : public Matrix
{
private:
    std::vector<RealType> _values;              ///< hodnoty nenulových prvků v matici, řazeny zleva doprava a shora dolů
    std::vector<IndexType> _column_indexes;    ///< sloupcové indexy prvků z @ref _values
    std::vector<IndexType> _row_indexes;       ///< indexy do @ref _column_indexes, kde začíná daný řádek

    virtual void _init( void );       // inicializace, pro konstruktory
    void _delete( IndexType i );    // smazat i-tý prvek z _values a _column_indexes
    void _insert( IndexType i, IndexType column, RealType data );    // vložit data na i-tou pozici do _values, nastavit column v _column_indexes

    // UMFPACK Numeric object
    void* Numeric = nullptr;

public:
    SparseMatrix( IndexType rows, IndexType columns );
    SparseMatrix( IndexType order );
    ~SparseMatrix( void );

    // accessors to matrix elements
    virtual bool set( IndexType row, IndexType column, RealType data );
    virtual RealType get( IndexType row, IndexType column ) const;

    // file saving/loading
    virtual bool save( const std::string & filename ) const;
    virtual bool load( const std::string & filename );

    // solve linear system with UMFPACK
    bool factorize( void );
    bool linear_solve( Vector & x, Vector & rhs );

    // reserve space for 'n' non-zero elements
    void reserve( unsigned n );
};
