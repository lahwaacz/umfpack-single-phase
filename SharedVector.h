#pragma once

#include "Vector.h"

class SharedVector
    : public Vector
{
public:
    SharedVector( void )
        : Vector() {};
    SharedVector( RealType * data, const IndexType size )
        : Vector() {
            bind( data, size );
        };
    SharedVector( RealType & data, const IndexType size )
        : Vector() {
            bind( data, size );
        };

    ~SharedVector( void ) {
        // Make sure that the parent destructor does not delete data. The
        // data is shared, and destroyed from the object which allocated it.
        data = nullptr;
    };

    // methods to bind to new data
    void bind( RealType * data, const IndexType size ) {
        this->data = data;
        this->size = size;
    };
    void bind( RealType & data, const IndexType size ) {
        this->data = &data;
        this->size = size;
    };

    // TODO: is it possible to disable allocateMemory() and freeMemory() instead?
    // disable Vector's setSize() method which deletes old and allocates new data
    bool setSize( const IndexType size ) { return false; };
};
