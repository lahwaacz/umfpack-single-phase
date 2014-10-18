#pragma once

#include "DenseMatrix.h"
#include "Vector.h"

bool SORMethod ( const DenseMatrix & A,
                 const Vector & b,
                 Vector & x,
                 RealType eps,
                 RealType omega );
