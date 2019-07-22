#include "Matrix.h"

// matrix helpers
void m3_identity( MATRIX3 mat )
{
	for (int i=0; i<9; i++)
		mat[i] = M3_I[i];
}

float m3_det( MATRIX3 mat )
{
	float det;
	det = mat[0] * ( mat[4]*mat[8] - mat[7]*mat[5] )
	  - mat[1] * ( mat[3]*mat[8] - mat[6]*mat[5] )
	  + mat[2] * ( mat[3]*mat[7] - mat[6]*mat[4] );
	return( det );
}

int m3_inverse( MATRIX3 mr, MATRIX3 ma )
{
	float det = m3_det( ma );
	if ( std::abs( det ) < 0.0005 )
	{
		m3_identity( mr );
		return(0);
	}
	mr[0] =    ma[4]*ma[8] - ma[5]*ma[7]   / det;
	mr[1] = -( ma[1]*ma[8] - ma[7]*ma[2] ) / det;
	mr[2] =    ma[1]*ma[5] - ma[4]*ma[2]   / det;
	mr[3] = -( ma[3]*ma[8] - ma[5]*ma[6] ) / det;
	mr[4] =    ma[0]*ma[8] - ma[6]*ma[2]   / det;
	mr[5] = -( ma[0]*ma[5] - ma[3]*ma[2] ) / det;
	mr[6] =    ma[3]*ma[7] - ma[6]*ma[4]   / det;
	mr[7] = -( ma[0]*ma[7] - ma[6]*ma[1] ) / det;
	mr[8] =    ma[0]*ma[4] - ma[1]*ma[3]   / det;
	return(1);
}

void m4_zero( MATRIX4 mat )
{
	for (int i=0; i<16; i++)
		mat[i] = M4_Z[i];
}

void m4_copy( MATRIX4 mr, const MATRIX4 mb )
{
	for (int i=0; i<16; i++)
		mr[i] = mb[i];
}

void m4_identity( MATRIX4 mat )
{
	for (int i=0; i<16; i++)
		mat[i] = M4_I[i];
}

void m4_submat( MATRIX4 mr, MATRIX3 mb, int i, int j )
{
	int di, dj, si, sj;
	// loop through 3x3 submatrix
	for( di = 0; di < 3; di ++ )
	{
		for( dj = 0; dj < 3; dj ++ )
		{
		// map 3x3 element (destination) to 4x4 element (source)
		si = di + ( ( di >= i ) ? 1 : 0 );
		sj = dj + ( ( dj >= j ) ? 1 : 0 );
		// copy element
		mb[di * 3 + dj] = mr[si * 4 + sj];
		}
	}
}

float m4_det( MATRIX4 mr )
{
	float  det, result = 0, i = 1;
	MATRIX3 msub3;
	int     n;
	for ( n = 0; n < 4; n++, i *= -1 )
	{
		m4_submat( mr, msub3, 0, n );
		det     = m3_det( msub3 );
		result += mr[n] * det * i;
	}
	return( result );
}

int m4_inverse( MATRIX4 mr, MATRIX4 ma )
{
	float  mdet = m4_det( ma );
	MATRIX3 mtemp;
	int     i, j, sign;

	if ( std::abs( mdet ) < 0.0005 )
	{
		m4_identity( mr );
		return( 0 );
	}
	for ( i = 0; i < 4; i++ )
	{
		for ( j = 0; j < 4; j++ )
		{
		sign = 1 - ( (i +j) % 2 ) * 2;
		m4_submat( ma, mtemp, i, j );
		mr[i+j*4] = ( m3_det( mtemp ) * sign ) / mdet;
		}
	}
	return( 1 );
}

void m4_print( const MATRIX4 mat )
{
	for (int i=0; i<4; i++)
		std::cout << "\t" << mat[i] << ", " << mat[4+i] << ", " << mat[8+i] << ", " << mat[12+i] << std::endl;
}
