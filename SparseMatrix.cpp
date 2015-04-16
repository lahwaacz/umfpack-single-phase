/**
 * @file    SparseMatrix.cc
 * @brief   Implementace funkcí a metod pro třídu @ref SparseMatrix.
 */

#include <fstream>      // file streams (saving/loading CSR files)
#include <sstream>      // string streams
#include <iterator>     // iterators for standard containers and streams
#include <umfpack.h>

#include "SparseMatrix.h"
#include "exceptions.h"

using namespace std;


/**
 * Smaže i-tý prvek z polí _values a _column_indexes.
 */
void
SparseMatrix::_delete( IndexType i )
{
    vector< RealType >::iterator val_position = _values.begin() + i;
    _values.erase( val_position );
    vector< IndexType >::iterator col_position = _column_indexes.begin() + i;
    _column_indexes.erase( col_position );
}

/**
 * Vloží data na i-tou pozici do _values, na i-tou pozici do _column_indexes uloží column.
 */
void
SparseMatrix::_insert( IndexType i, IndexType column, RealType data )
{
    vector< RealType >::iterator val_position = _values.begin() + i;
    _values.insert(val_position, data);
    vector< IndexType >::iterator col_position = _column_indexes.begin() + i;
    _column_indexes.insert( col_position, column );
}

SparseMatrix::~SparseMatrix( void )
{
    if( Symbolic )
        umfpack_di_free_symbolic( &Symbolic );
    if( Numeric )
        umfpack_di_free_numeric( &Numeric );
}


bool SparseMatrix::setSize( const IndexType rows, const IndexType cols )
{
    if( rows < 0 || cols < 0 )
        throw BadIndex("Attempted to set negative matrix size");

    // clear content
    _values.clear();
    _column_indexes.clear();
    _row_indexes.clear();
    if( Symbolic ) {
        umfpack_di_free_symbolic( &Symbolic );
        Symbolic = nullptr;
    }
    if( Numeric ) {
        umfpack_di_free_numeric( &Numeric );
        Numeric = nullptr;
    }
    
    // update size
    this->rows = rows;
    this->cols = cols;

    try {
        _row_indexes.reserve( rows + 1 );
    } catch (...) {
        return false;
    }

    _row_indexes.push_back( 0 );  // number of non-zero elements
    return true;
}

/**
 * Nastaví prvek matice na zadanou hodnotu.
 * Vyvolá vyjímku, pokud jsou indexy mimo rozměry matice.
 * @param row       index řádku (číslováno od 0)
 * @param column    index sloupce (číslováno od 0)
 * @param data      hodnota která se uloží do matice
 * @return          false if setting failed
 */
bool
SparseMatrix::setElement( const IndexType row, const IndexType column, const RealType & data )
{
    if( row < 0 || row >= rows || column < 0 || column >= cols )
        throw BadIndex("matrix indexes out of bounds");

    RealType current_value = getElement(row, column);

    // overwrite existing element
    if (data != 0 and current_value != 0) {
        // hledat index prvku v _column_indexes
        IndexType index = _row_indexes[row];
        while (index < _row_indexes[row+1] and _column_indexes[index] < column)
            index++;
        _values[index] = data;
    }
    // insert new element
    else if (data != 0 and current_value == 0) {
        // ensure that _row_indexes has size of the index of the last non-zero row + 1
        while( (unsigned) row + 1 >= _row_indexes.size() )
            _row_indexes.push_back( _row_indexes.back() );

        // for next non-zero element find its index in _column_indexes
        IndexType index = _row_indexes[row];
        while (index < _row_indexes[row+1] and _column_indexes[index] <= column)
            index++;

        // insert before the next non-zero element
        _insert(index, column, data);

        // fix row indexes
        for( unsigned i = row+1; i < _row_indexes.size(); i++ )
            _row_indexes[i]++;
    }
    // remove element (reset to zero)
    else if (data == 0 and current_value != 0) {
        // find element index in _column_indexes
        IndexType index = _row_indexes[row];
        while (index < _row_indexes[row+1] and _column_indexes[index] < column)
            index++;

        // reset element to zero
        _delete(index);

        // fix row indexes
        for( unsigned i = row+1; i < _row_indexes.size(); i++ )
            _row_indexes[i]--;
    }

    // TODO: check for errors
    return true;
}

/**
 * Přečte hodnotu prvku matice.
 * Vyvolá vyjímku, pokud jsou indexy mimo rozměry matice.
 * @param row       index řádku (číslováno od 0)
 * @param column    index sloupce (číslováno od 0)
 * @return          hodnota prvku matice
 */
RealType
SparseMatrix::getElement( const IndexType row, const IndexType column ) const
{
    if( row >= rows or column >= cols )
        throw string("Indexy mimo rozměry matice");

    // check if row has any non-zero element
    if( (unsigned) row + 1 >= _row_indexes.size() )
        return 0;

    // find element index in _column_indexes
    IndexType index = _row_indexes[row];
    while( index < _row_indexes[row+1] and _column_indexes[index] < column )
        index++;

    // check if element was found
    if( index < _row_indexes[row+1] and _column_indexes[index] == column )
        return _values[index];

    // default value
    return 0;
}

/**
 * Uloží matici do souboru ve formátu CSR.
 * @return  true pokud uložení proběhlo úspěšně
 */
bool SparseMatrix::save( const string & filename ) const
{
    ofstream outfile(filename.c_str());
    if (not outfile.is_open())
        return false;
    
    // write values
    for( vector< RealType >::const_iterator iter = _values.begin();
            iter != _values.end();
            ++iter ) {
        outfile << *iter << " ";
    }
    outfile << endl;

    // write column indexes
    for( vector< IndexType >::const_iterator iter = _column_indexes.begin();
            iter != _column_indexes.end();
            ++iter ) {
        outfile << *iter << " ";
    }
    outfile << endl;

    // write row indexes
    for( vector<IndexType>::const_iterator iter = _row_indexes.begin();
            iter != _row_indexes.end();
            ++iter ) {
        outfile << *iter << " ";
    }
    // write "non-allocated" row indexes to satisfy CSR format
    for( IndexType i = _row_indexes.size(); i <= rows; i++ ) {
        outfile << _row_indexes.back() << " ";
    }
    outfile << endl;

    return true;
}

/**
 * Načte data ze souboru ve formátu CSR.
 * @return  true pokud načtení proběhlo úspěšně
 */
// FIXME: needs to call setSize() first
bool SparseMatrix::load( const string & filename )
{
    ifstream infile(filename.c_str());
    if (not infile.good())
        return false;
    
    string str_values;  // first line of file
    string str_columns; // second line of file
    string str_rows;    // third line of file

    // test if we can read three lines, other lines are ignored
    if (not (getline(infile, str_values) and getline(infile, str_columns) and getline(infile, str_rows)))
        return false;

    // temporary vectors for CSR format
    vector<RealType> tmp_vect_values;
    vector<IndexType> tmp_vect_columns;
    vector<IndexType> tmp_vect_rows;

    // parse row indexes
    {
        stringstream ss(str_rows);
        int tmp = 0;
        while (ss >> tmp) {
            if (tmp < 0)
                return false;
            tmp_vect_rows.push_back(tmp);
        }
    }
    if( _row_indexes.capacity() != tmp_vect_rows.size() or tmp_vect_rows.back() > rows * cols )
        return false;

    // parse column indexes
    {
        stringstream ss(str_columns);
        int tmp = 0;
        while (ss >> tmp) {
            if (tmp < 0)
                return false;
            tmp_vect_columns.push_back(tmp);
        }
    }
    if (tmp_vect_rows.back() != (IndexType) tmp_vect_columns.size())
        return false;

    // parse values
    {
        stringstream ss(str_values);
        RealType tmp = 0.0;
        while (ss >> tmp) {
            if (tmp == 0.0)
                return false;
            tmp_vect_values.push_back(tmp);
        }
    }
    if (tmp_vect_values.size() != tmp_vect_columns.size())
        return false;

    // copy vectors
    _values = tmp_vect_values;
    _column_indexes = tmp_vect_columns;
    _row_indexes = tmp_vect_rows;
    return true;
}

// solve linear system  A*x=rhs using UMFPACK
bool SparseMatrix::linear_solve( Vector & x, Vector & rhs )
{
    if( rows != cols )
        throw string("can't solve linear system on non-square matrix");
    if( x.getSize() != rows || rhs.getSize() != rows )
        throw string("passed vectors don't match matrix dimensions");

    int status = UMFPACK_OK;
    double Control[ UMFPACK_CONTROL ];
    double Info[ UMFPACK_INFO ];
//    Control[ UMFPACK_PRL ] = 2;

    // symbolic reordering of the sparse matrix
    // (only needed when we're going to do numeric factorization)
    if( Symbolic == nullptr && Numeric == nullptr ) {
        status = umfpack_di_symbolic( rows, rows, &_row_indexes[0], &_column_indexes[0], &_values[0], &Symbolic, Control, Info );
        if( status != UMFPACK_OK ) {
            cerr << "error: symbolic reordering failed" << endl;
            umfpack_di_report_status( Control, status );
//           umfpack_di_report_control( Control );
//           umfpack_di_report_info( Control, Info );
            return false;
        }
    }

    // numeric factorization
    if( Numeric == nullptr ) {
        status = umfpack_di_numeric( &_row_indexes[0], &_column_indexes[0], &_values[0], Symbolic, &Numeric, Control, Info );
        if( status != UMFPACK_OK ) {
            cerr << "error: numeric factorization failed" << endl;
            umfpack_di_report_status( Control, status );
//           umfpack_di_report_control( Control );
//           umfpack_di_report_info( Control, Info );
            return false;
        }
    }

    // umfpack expects Compressed Sparse Column format, we have Compressed Sparse Row
    // so we need to solve  A^T * x = rhs
    int sys = UMFPACK_Aat;

    // solve with specified right-hand-side
    status = umfpack_di_solve( sys, &_row_indexes[0], &_column_indexes[0], &_values[0], &x[0], &rhs[0], Numeric, Control, Info );
    if( status != UMFPACK_OK ) {
        cerr << "error: umfpack_di_solve failed" << endl;
            umfpack_di_report_status( Control, status );
//           umfpack_di_report_control( Control );
//           umfpack_di_report_info( Control, Info );
        return false;
    }

//    umfpack_di_report_info( Control, Info );

    return true;
}

bool SparseMatrix::reserve( unsigned n )
{
    try {
        _values.reserve( n );
        _column_indexes.reserve( n );
        return true;
    } catch (...) {
        return false;
    }
}

