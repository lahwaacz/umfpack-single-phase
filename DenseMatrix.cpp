#include <fstream>
#include <sstream>

#include "DenseMatrix.h"
#include "exceptions.h"

using namespace std;

// TODO: split into Array.h
bool DenseMatrix::allocateMemory( RealType* & data, const IndexType & size )
{
    data = new RealType[ size ];
    if( ! data )
        return false;
    return true;
}

// TODO: split into Array.h
bool DenseMatrix::freeMemory( RealType* & data )
{
    delete[] data;
    return true;
}

DenseMatrix::~DenseMatrix()
{
    freeMemory( data );
}

bool DenseMatrix::setSize( const IndexType rows, const IndexType cols )
{
    if( rows < 0 || cols < 0 )
        throw BadIndex("Attempted to set negative matrix size");
    
    if( data ) {
        freeMemory( data );
        data = nullptr;
    }
    this->rows = rows;
    this->cols = cols;
    if( ! allocateMemory( data, rows * cols ) )
        return false;
    return true;
}

const RealType* DenseMatrix::getData( void ) const
{
    return data;
}

RealType* DenseMatrix::getData( void )
{
    return data;
}

bool DenseMatrix::setElement( const IndexType row, const IndexType col, const RealType & data )
{
    this->operator()( row, col ) = data;
    return true;
}

RealType DenseMatrix::getElement( const IndexType row, const IndexType col ) const
{
    return this->operator()( row, col );
}

RealType & DenseMatrix::operator() ( const IndexType row, const IndexType col )
{
    if( row >= rows || col >= cols )
        throw BadIndex("DenseMatrix subscript out of bounds");
    return data[ getCols() * row + col ];
}

RealType DenseMatrix::operator() ( const IndexType row, const IndexType col ) const
{
    if( row >= rows || col >= cols )
        throw BadIndex("const DenseMatrix subscript out of bounds");
    return data[ getCols() * row + col ];
}

void DenseMatrix::setAllElements( const RealType & value )
{
    for( IndexType i = 0; i < rows * cols; i++ )
        data[ i ] = value;
}

bool DenseMatrix::save( const std::string & file_name ) const
{
    ofstream file( file_name.c_str() );
    file << "# saved dense matrix:" << endl;
    file << "# <row index> <column index> <value>" << endl;
    for( IndexType i = 0; i < rows; i++ ) {
        for( IndexType j = 0; j < cols; j++ ) {
            file << i << " " << j << " " << getElement( i, j ) << endl;
        }
    }
    return true;
}
    
// TODO: check dimensions
bool DenseMatrix::load( const std::string & file_name )
{
    ifstream file( file_name.c_str() );
    if( file.good() ) {
        string line;
        while( getline( file, line ) ) {
            // skip comments starting with '#'
            if( line.compare(0, 1, "#") == 0 )
                continue;

            int i = 0;
            int j = 0;
            RealType value = 0.0;
            stringstream ss( line );
            ss >> i >> j >> value;
            setElement( i, j, value );
        }
        return true;
    }
    else {
        return false;
    }
}

