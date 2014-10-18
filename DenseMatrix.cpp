#include <fstream>
#include <sstream>

#include "DenseMatrix.h"
#include "exceptions.h"

using namespace std;

void DenseMatrix::_init( void )
{
    _data = new RealType[ rows() * cols() ];
}

DenseMatrix::DenseMatrix( IndexType rows, IndexType cols )
    : Matrix( rows, cols )
{
    _init();
}

DenseMatrix::~DenseMatrix()
{
    delete[] _data;
}

RealType & DenseMatrix::operator() ( IndexType row, IndexType col )
{
    if( row >= _rows || col >= _cols )
        throw BadIndex("DenseMatrix subscript out of bounds");
    return _data[ _cols * row + col ];
}

RealType DenseMatrix::operator() ( IndexType row, IndexType col ) const
{
    if( row >= _rows || col >= _cols )
        throw BadIndex("const DenseMatrix subscript out of bounds");
    return _data[ _cols * row + col ];
}

bool DenseMatrix::set( IndexType row, IndexType col, RealType data )
{
    this->operator()( row, col ) = data;
    return true;
}

RealType DenseMatrix::get( IndexType row, IndexType col ) const
{
    return this->operator()( row, col );
}

bool DenseMatrix::save( const std::string & file_name ) const
{
    ofstream file( file_name.c_str() );
    file << "# saved dense matrix:" << endl;
    file << "# <row index> <column index> <value>" << endl;
    for( IndexType i = 0; i < _rows; i++ ) {
        for( IndexType j = 0; j < _cols; j++ ) {
            file << i << " " << j << " " << this->operator()( i, j ) << endl;
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
            this->operator()( i, j ) = value;
        }
        return true;
    }
    else {
        return false;
    }
}

