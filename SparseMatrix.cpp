/**
 * @file    SparseMatrix.cc
 * @brief   Implementace funkcí a metod pro třídu @ref SparseMatrix.
 */

#include <fstream>      // file streams (saving/loading CSR files)
#include <sstream>      // string streams
#include <iterator>     // iterators for standard containers and streams
#include <umfpack.h>

#include "SparseMatrix.h"

using namespace std;


/**
 * Inicializační funkce - alokuje interní datové položky.
 */
void
SparseMatrix::_init( void )
{
    for( IndexType i = 0; i < _rows; i++ )
        _row_indexes.push_back(0);
    _row_indexes.push_back(0);  // počet nenulových prvků (NNZ)
}

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


/**
 * Konstruktor - vytvoří obdélníkovou matici zadaných rozměrů.
 */
SparseMatrix::SparseMatrix( IndexType rows, IndexType columns )
    : Matrix(rows, columns)
{
    _init();
}

/**
 * Konstruktor - vytvoří čtvercovou matici zadaného řádu.
 */
SparseMatrix::SparseMatrix( IndexType order )
    : Matrix(order)
{
    _init();
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
SparseMatrix::set( IndexType row, IndexType column, RealType data )
{
    if (row >= _rows or column >= _cols)
        throw string("Indexy mimo rozměry matice");

    RealType current_value = get(row, column);

    // přepsání existujícího
    if (data != 0 and current_value != 0) {
        // hledat index prvku v _column_indexes
        IndexType index = _row_indexes[row];
        while (index < _row_indexes[row+1] and _column_indexes[index] < column)
            index++;
        _values[index] = data;
    }
    // vložení prvku
    else if (data != 0 and current_value == 0) {
        // hledat index následujícího nenulového prvku v _column_indexes
        IndexType index = _row_indexes[row];
        while (index < _row_indexes[row+1] and _column_indexes[index] <= column)
            index++;
        _insert(index, column, data);
        for( IndexType i = row+1; i <= _rows; i++ )    // fix row indexes
            _row_indexes[i]++;
    }
    // mazání prvku
    else if (data == 0 and current_value != 0) {
        // hledat index prvku v _column_indexes
        IndexType index = _row_indexes[row];
        while (index < _row_indexes[row+1] and _column_indexes[index] < column)
            index++;
        _delete(index);
        for( IndexType i = row+1; i <= _rows; i++ )    // fix row indexes
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
SparseMatrix::get( IndexType row, IndexType column ) const
{
    if( row >= _rows or column >= _cols )
        throw string("Indexy mimo rozměry matice");

    // hledat index prvku v _column_indexes
    IndexType index = _row_indexes[row];
    while( index < _row_indexes[row+1] and _column_indexes[index] < column )
        index++;
    if( index < _row_indexes[row+1] and _column_indexes[index] == column )
        return _values[index];
    else
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
    for (vector< RealType >::const_iterator iter = _values.begin();
            iter != _values.end();
            ++iter) {
        outfile << *iter << " ";
    }
    outfile << endl;

    // write column indexes
    for (vector< IndexType >::const_iterator iter = _column_indexes.begin();
            iter != _column_indexes.end();
            ++iter) {
        outfile << *iter << " ";
    }
    outfile << endl;

    // write row indexes
    for (vector<IndexType>::const_iterator iter = _row_indexes.begin();
            iter != _row_indexes.end();
            ++iter) {
        outfile << *iter << " ";
    }
    outfile << endl;

    return true;
}

/**
 * Načte data ze souboru ve formátu CSR.
 * @return  true pokud načtení proběhlo úspěšně
 */
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
    if( _row_indexes.size() != tmp_vect_rows.size() or tmp_vect_rows.back() > _rows * _cols)
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
    if( _rows != _cols )
        throw string("can't solve linear system on non-square matrix");
    if( x.rows() != _rows || rhs.rows() != _rows )
        throw string("passed vectors don't match matrix dimensions");

    void* Symbolic = nullptr;
    void* Numeric = nullptr;

    // umfpack expects Compressed Sparse Column format, we have Compressed Sparse Row
    // so we need to solve  A^T * x = rhs
    int sys = UMFPACK_Aat;

    // symbolic reordering of the sparse matrix
    umfpack_di_symbolic( _rows, _rows, &_row_indexes[0], &_column_indexes[0], &_values[0], &Symbolic, nullptr, nullptr );

    // factorization of Ap,Ai,Ax
    umfpack_di_numeric( &_row_indexes[0], &_column_indexes[0], &_values[0], Symbolic, &Numeric, nullptr, nullptr );
    umfpack_di_free_symbolic( &Symbolic );

    // solve with various rhs
    umfpack_di_solve( sys, &_row_indexes[0], &_column_indexes[0], &_values[0], &x[0], &rhs[0], Numeric, nullptr, nullptr );
    umfpack_di_free_numeric( &Numeric );

    // TODO: check umfpack errors
    return true;
}
