#pragma once

#include "Vector.h"
#include "SharedArray.h"

// SharedVector combines Vector and SharedArray classes.
//
// - Vector and SharedArray must use virtual inheritance
//   on Array to avoid the Diamond of Death.
// - SharedArray is inherited first, so that its overridden
//   methods (destructor and setSize) are used. This is
//   necessary to avoid shared data deallocation.

class SharedVector
    : public SharedArray, public Vector
{
    using SharedArray::SharedArray;
};
