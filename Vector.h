#pragma once

#include "Array.h"

class Vector
    : virtual public Array
{
public:
    RealType norm( void ) const;

    // TODO: operators
};

