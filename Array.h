#pragma once

#include <iostream>
#include <string>

// TODO: required only for IndexType and RealType
#include "Matrix.h"

class Array
{
protected:
    IndexType size = 0;
    RealType* data = nullptr;

    bool allocateMemory( RealType* & data, const IndexType & size );
    bool freeMemory( RealType* & data );
    bool copyMemory( const RealType* source, RealType* destination, const IndexType size );

public:
    Array( void );
    Array( const IndexType size );
    // copy constructor
    Array( const Array & array );
    // move constructor
    Array( Array && array ) noexcept;

    // destructor
    virtual ~Array( void );

    // copy assignment
    Array & operator=( const Array & array );
    // move assignment
    Array & operator=( Array && array ) noexcept;

    // set/get vector size
    virtual bool setSize( const IndexType size );
    IndexType getSize( void ) const;

    // accessors to data storage
    const RealType* getData( void ) const;
    RealType* getData( void );

    // element setters/getters
    void setElement( const IndexType index, const RealType & value );
    RealType getElement( const IndexType index ) const;

    // subscript operators pair
    RealType & operator[] ( const IndexType row );
    RealType operator[] ( const IndexType row ) const;

    // set all elements to the same value
    void setAllElements( const RealType & value );

    // file saving/loading
    bool save( const std::string & file_name ) const;
    bool load( const std::string & file_name );

    // simple output
    void print( std::ostream & os = std::cout ) const;
};

