#include <fstream>
#include <sstream>

#include "Array.h"
#include "exceptions.h"

using namespace std;

bool Array::allocateMemory( RealType* & data, const IndexType & size )
{
    data = new RealType[ size ];
    if( ! data )
        return false;
    return true;
}

bool Array::freeMemory( RealType* & data )
{
    delete[] data;
    return true;
}

bool Array::copyMemory( const RealType* source, RealType* destination, const IndexType size )
{
    // TODO: memcpy is probably faster
    for( IndexType i = 0; i < size; i++ )
        destination[ i ] = source[ i ];
    return true;
}

Array::Array( void )
{
}

Array::Array( const IndexType size )
{
    // TODO: handle allocation errors?
    setSize( size );
}

// copy constructor
Array::Array( const Array & array )
{
    // TODO: handle allocation errors?
    setSize( array.size );
    copyMemory( array.data, data, size );
}

// move constructor
Array::Array( Array && array ) noexcept
{
    (*this) = std::move( array );
}

Array::~Array( void )
{
    freeMemory( data );
}

// copy assignment
Array & Array::operator=( const Array & array )
{
    if( size != array.getSize() )
        throw BadArraySize("Array.operator=(): the assigned-to object needs to have the same size as the assigned-from object.");

    copyMemory( array.getData(), data, size );
    return *this;
}

// move assignment
Array & Array::operator=( Array && array ) noexcept
{
    // The state of the src object after the move must be valid but indeterminate.
    // Easiest solution is to swap the states of the two objects.
    std::swap( this->size, array.size );
    std::swap( this->data, array.data );
    return *this;
}

bool Array::setSize( const IndexType size )
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

IndexType Array::getSize( void ) const
{
    return size;
}

const RealType* Array::getData( void ) const
{
    return data;
}

RealType* Array::getData( void )
{
    return data;
}

void Array::setElement( const IndexType index, const RealType & value )
{
    data[ index ] = value;
}

RealType Array::getElement( IndexType index ) const
{
    return data[ index ];
}

RealType & Array::operator[]( IndexType index )
{
    if( index < 0 || index >= size )
        throw BadIndex("Array subscript out of bounds");
    return data[ index ];
}

RealType Array::operator[]( IndexType index ) const
{
    if( index < 0 || index >= size )
        throw BadIndex("const Array subscript out of bounds");
    return data[ index ];
}

void Array::setAllElements( const RealType & value )
{
    for( IndexType i = 0; i < size; i++ )
        data[ i ] = value;
}

bool Array::save( const std::string & file_name ) const
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
bool Array::load( const std::string & file_name )
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

void Array::print( std::ostream & os ) const
{
    for( IndexType i = 0; i < size; i++ ) {
        os << getElement( i ) << endl;
    }
}

