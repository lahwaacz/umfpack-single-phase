#include <cmath>
#include <fstream>
#include <sstream>

#include "Vector.h"
#include "exceptions.h"

using namespace std;

bool Vector::allocateMemory( RealType* & data, const IndexType & size )
{
    data = new RealType[ size ];
    if( ! data )
        return false;
    return true;
}

bool Vector::freeMemory( RealType* & data )
{
    delete[] data;
    return true;
}

// basic constructor
Vector::Vector()
    : size( 0 ), data( nullptr )
{
}

Vector::~Vector()
{
    freeMemory( data );
}

bool Vector::setSize( const IndexType size )
{
    if( size < 0 )
        throw BadIndex("Attempted to set negative vector size");
    
    if( data ) {
        freeMemory( data );
        data = nullptr;
    }
    this->size = size;
    if( ! allocateMemory( data, this->size ) )
        return false;
    return true;
}

IndexType Vector::getSize( void ) const
{
    return size;
}

const RealType* Vector::getData( void ) const
{
    return data;
}

RealType* Vector::getData( void )
{
    return data;
}

void Vector::setElement( const IndexType index, const RealType & value )
{
    data[ index ] = value;
}

RealType Vector::getElement( IndexType index ) const
{
    return data[ index ];
}

RealType & Vector::operator[]( IndexType index )
{
    if( index < 0 || index >= size )
        throw BadIndex("Vector subscript out of bounds");
    return data[ index ];
}

RealType Vector::operator[]( IndexType index ) const
{
    if( index < 0 || index >= size )
        throw BadIndex("const Vector subscript out of bounds");
    return data[ index ];
}

void Vector::setAllElements( const RealType & value )
{
    for( IndexType i = 0; i < size; i++ )
        data[ i ] = value;
}

bool Vector::save( const std::string & file_name ) const
{
    ofstream file( file_name.c_str() );
    file << "# saved vector:" << endl;
    file << "# <row index> <value>" << endl;
    for( IndexType i = 0; i < size; i++ ) {
        file << i << " " << getElement( i ) << endl;
    }
    return true;
}

// TODO: check dimensions
// FIXME: call setSize() first
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
            setElement( i, value );
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
    for( IndexType i = 0; i < v.getSize(); i++)
    {
        s += v.getElement( i ) * v.getElement( i );
    }
    return sqrt( s );
}

void Vector::print( std::ostream & os ) const
{
    for( IndexType i = 0; i < size; i++ ) {
        os << getElement( i ) << endl;
    }
}

