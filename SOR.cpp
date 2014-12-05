#include <cmath>

#include "SOR.h"

bool SORMethod ( const DenseMatrix & A,
                 const Vector & b,
                 Vector & x,
                 RealType eps,
                 RealType omega )
{
    IndexType n = A.getCols();
    RealType normB = norm( b );
    RealType r = eps + 1.0;

    unsigned iteration = 0;
    unsigned maxIterations = 100000;

    while ( r > eps && iteration < maxIterations )
    {
        // SOR iteration
        for ( IndexType i = 0; i < n; i++ )
        {
            RealType s = 0.0;
            for ( IndexType j = 0; j < n; j++ )
                 s = s + A( i, j ) * x[ j ];
            x[ i ] += omega * ( b[ i ] - s ) / A( i, i );
        }

        // update residue
        r = 0.0;
        for ( IndexType i = 0; i < n; i++ )
        {
             RealType Ax_i = 0.0;
             for ( IndexType j = 0; j < n; j++ )
                 Ax_i = Ax_i + A( i, j ) * x[ j ];
             r = r + ( Ax_i - b[ i ] ) * ( Ax_i - b[ i ] );
        }
        r = sqrt( r ) / normB;

        // increment iteration counter
        iteration++;
    }
    return iteration < maxIterations;
}
