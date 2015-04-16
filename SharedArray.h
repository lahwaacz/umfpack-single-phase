#pragma once

#include "Array.h"

// FIXME: the state of SharedArray is undefined after any operation involving move or swap
// FIXME: [-Wvirtual-move-assign]  http://stackoverflow.com/questions/17252869/danger-with-virtual-base-move-assignment-operators-when-they-are-now-allowed-to

class SharedArray
    : virtual public Array
{
public:
    SharedArray( void )
        : Array() {};
    SharedArray( Array & array )
        : Array() {
            bind( array.getData(), array.getSize() );
        };
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

    // Disable default copy constructor to prevent sharing from const object
    // and potentially modifying its data.
    SharedArray( const SharedArray & array ) = delete;

    // Disable Array's setSize() method which deletes old and allocates new data.
    bool setSize( const IndexType size ) { return false; };
};

