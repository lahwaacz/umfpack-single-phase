#pragma once

#include "Array.h"

class Vector
    : virtual public Array
{
public:
    using Array::Array;
    RealType norm( void ) const;

    // TODO: operators
};

