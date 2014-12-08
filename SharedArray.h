#pragma once

#include "Array.h"

class SharedArray
    : virtual public Array
{
public:
    SharedArray( void )
        : Array() {};
    SharedArray( RealType * data, const IndexType size )
        : Array() {
            bind( data, size );
        };
    SharedArray( RealType & data, const IndexType size )
        : Array() {
            bind( data, size );
        };

    ~SharedArray( void ) {
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
    // disable Array's setSize() method which deletes old and allocates new data
    bool setSize( const IndexType size ) { return false; };
};
