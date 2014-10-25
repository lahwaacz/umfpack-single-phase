#include <cmath>
#include <fstream>
#include <sstream>

#include "Vector.h"
#include "exceptions.h"

using namespace std;

Vector::Vector( IndexType rows )
    : _rows( rows )
{
    if( rows == 0 )
        throw BadIndex("Vector constructor has 0 size");
    _data = new RealType[ rows ];
}

// constructor for constant vector
Vector::Vector( IndexType rows, RealType init_value )
    : _rows( rows )
{
    if( rows == 0 )
        throw BadIndex("Vector constructor has 0 size");
    _data = new RealType[ rows ];
    
    for( IndexType i = 0; i < rows; i++ )
        _data[ i ] = init_value;
}

Vector::~Vector()
{
    delete[] _data;
}

RealType & Vector::at( IndexType row )
{
    if( row >= _rows )
        throw BadIndex("Vector subscript out of bounds");
    return _data[ row ];
}

RealType Vector::at( IndexType row ) const
{
    if( row >= _rows )
        throw BadIndex("const Vector subscript out of bounds");
    return _data[ row ];
}

bool Vector::save( const std::string & file_name ) const
{
    ofstream file( file_name.c_str() );
    file << "# saved vector:" << endl;
    file << "# <row index> <value>" << endl;
    for( IndexType i = 0; i < _rows; i++ ) {
        file << i << " " << at( i ) << endl;
    }
    return true;
}

// TODO: check dimensions
bool Vector::load( const std::string & file_name )
{
    ifstream file( file_name.c_str() );
    if( file.good() ) {
        string line;
        while( getline( file, line ) ) {
            // skip comments starting with '#'
            if( line.compare(0, 1, "#") == 0 )
                continue;

            int i = 0;
            RealType value = 0.0;
            stringstream ss( line );
            ss >> i >> value;
            at( i ) = value;
        }
        return true;
    }
    else {
        return false;
    }
}


RealType norm( const Vector & v )
{
    RealType s = 0.0;
    for( IndexType i = 0; i < v.rows(); i++)
    {
        s += v.at( i ) * v.at( i );
    }
    return sqrt( s );
}

void Vector::print( std::ostream & os ) const
{
    for( IndexType i = 0; i < _rows; i++ ) {
        os << at( i ) << endl;
    }
}

