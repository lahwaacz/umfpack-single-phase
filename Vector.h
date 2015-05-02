#pragma once

#include "Array.h"

class Vector
    : virtual public Array
{
public:
    RealType norm( void ) const;
    bool save_tnl( const std::string & filename ) const;

    // TODO: operators
};

