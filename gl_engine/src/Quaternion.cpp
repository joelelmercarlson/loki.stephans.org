#include "Quaternion.h"

const float Quaternion::m_fEpsilon = 1e-03;
const Quaternion Quaternion::IDENTITY(0,0,0,1);
const Quaternion Quaternion::ZERO(0,0,0,0);

Quaternion::Quaternion():
	x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Quaternion::Quaternion(double x, double y, double z, float w):
	x(x), y(y), z(z), w(w)
{
}

Quaternion::Quaternion(const Vector& v, float w):
	x(v[0]), y(v[1]), z(v[2]), w(w)
{
}

Quaternion::~Quaternion()
{
}

Quaternion Quaternion::operator*(const Quaternion q) const
{
	return Quaternion(
		w*q.x + x*q.w + y*q.z - z*q.y, // x
		w*q.y + y*q.w + z*q.x - x*q.z, // y
		w*q.z + z*q.w + x*q.y - y*q.x, // z
		w*q.w - x*q.x - y*q.y - z*q.z  // w
	);
}

Vector Quaternion::operator*(const Vector& v) const
{
	// nVidia SDK implementation
	Vector uv, uuv;
	Vector qvec(x, y, z);
	uv = qvec.cross(v);
	uuv = qvec.cross(uv);
	uv *= (2.0f * w);
	uuv *= 2.0f;
	return v + uv + uuv;
}

Quaternion Quaternion::operator*(const float& scale) const
{
	return Quaternion(x*scale, y*scale, z*scale, w*scale);
}

Quaternion Quaternion::operator-() const
{
	return Quaternion(-x, -y, -z, -w);
}

Quaternion Quaternion::operator+(const Quaternion& q) const
{
	return Quaternion(x+q.x, y+q.y, z+q.z, w+q.w);
}

Quaternion Quaternion::operator-(const Quaternion& q) const
{
	return Quaternion(x-q.x, y-q.y, z-q.z, w-q.w);
}

void Quaternion::CreateFromAxisAngle(const Vector& axis, float degrees)
{
	CreateFromAxisAngle(axis[0], axis[1], axis[2], degrees);
}

void Quaternion::CreateFromAxisAngle(const float& in_x, const float& in_y, const float& in_z, float degrees)
{
	// radians
	float angle = (degrees/180.0f) * 3.141592653589792;

	// Here we calculate the sin( theta / 2) once for optimization
	float result = std::sin(angle/2.0f);
                
	// Calcualte the w value by cos( theta / 2 )
	w = std::cos(angle/2.0f);

	// Calculate the x, y and z of the quaternion
	x = float(in_x * result);
	y = float(in_y * result);
	z = float(in_z * result);
}

void Quaternion::CreateFromAxes(const Vector& xaxis, const Vector& yaxis, const Vector& zaxis)
{
	MATRIX3 Matrix;

	Matrix[0] = xaxis[0];
	Matrix[1] = xaxis[1];
	Matrix[2] = xaxis[2];
	Matrix[3] = yaxis[0];
	Matrix[4] = yaxis[1];
	Matrix[5] = yaxis[2];
	Matrix[6] = zaxis[0];
	Matrix[7] = zaxis[1];
	Matrix[8] = zaxis[2];

	CreateFromRotationMatrix(Matrix);
}

void Quaternion::CreateFromRotationMatrix(const MATRIX3& kRot)
{
        // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
        // article "Quaternion Calculus and Fast Animation".
	float mRot[3][3];
	float fTrace;
	float fRoot;
	
	// convert from opengl style to c++
	mRot[0][0] = kRot[0];
	mRot[0][1] = kRot[1];
	mRot[0][2] = kRot[2];
	mRot[1][0] = kRot[3];
	mRot[1][1] = kRot[4];
	mRot[1][2] = kRot[5];
	mRot[2][0] = kRot[6];
	mRot[2][1] = kRot[7];
	mRot[2][2] = kRot[8];

	fTrace = mRot[0][0] + mRot[1][1] + mRot[2][2];

	if ( fTrace > 0.0f ) 
	{
		// |w| > 1/2
		fRoot = std::sqrt(fTrace + 1.0f);
		w = 0.5f * fRoot;
		fRoot = 0.5f/fRoot; // 1/(4w)
		x = (mRot[2][1] - mRot[1][2]) * fRoot;
		y = (mRot[0][2] - mRot[2][0]) * fRoot;
		z = (mRot[1][0] - mRot[0][1]) * fRoot;
	}
	else
	{
		// |w| <= 1/2
		static size_t s_iNext[3] = { 1, 2, 0 };
		size_t i = 0;

		if (mRot[1][1] > mRot[0][0] )
			i = 1;
		if (mRot[2][2] > mRot[i][i] )
			i = 2;

		size_t j = s_iNext[i];
		size_t k = s_iNext[j];
		fRoot = std::sqrt(mRot[i][i]-mRot[j][j]-mRot[k][k] + 1.0f);
		double *apkQuat[3] = { &x, &y, &z };
		*apkQuat[i] = 0.5f * fRoot;
		fRoot = 0.5f / fRoot;
		w = (mRot[k][j]-mRot[j][k]) * fRoot;
		*apkQuat[j] = (mRot[j][i]+mRot[i][j]) * fRoot;
		*apkQuat[k] = (mRot[k][i]+mRot[i][k]) * fRoot;
	}
}

void Quaternion::CreateMatrix(float *pMatrix)
{
	// Make sure the matrix has allocated memory to store the rotation data
	if(!pMatrix) return;

	float xx = x * x;
	float xy = x * y;
	float xz = x * z;
	float xw = x * w;
	float yy = y * y;
	float yz = y * z;
	float yw = y * w;
	float zz = z * z;
	float zw = z * w;

	// First row
	pMatrix[ 0] = 1.0f - 2.0f * ( yy + zz ); 
	pMatrix[ 1] =        2.0f * ( xy - zw );
	pMatrix[ 2] =        2.0f * ( xz + yw );
	pMatrix[ 3] = 0.0f;
	// Second row
	pMatrix[ 4] =        2.0f * ( xy + zw );  
	pMatrix[ 5] = 1.0f - 2.0f * ( xx + zz ); 
	pMatrix[ 6] =        2.0f * ( yz - xw );  
	pMatrix[ 7] = 0.0f;
	// Third row
	pMatrix[ 8] =        2.0f * ( xz - yw );
	pMatrix[ 9] =        2.0f * ( yz + xw );
	pMatrix[10] = 1.0f - 2.0f * ( xx + yy );  
	pMatrix[11] = 0.0f;
	// Fourth row
	pMatrix[12] = 0.0f;
	pMatrix[13] = 0.0f;
	pMatrix[14] = 0.0f;
	pMatrix[15] = 1.0f;
	// Now pMatrix[] is a 4x4 homogeneous matrix that can be 
	// applied to an OpenGL Matrix
}


double Quaternion::dot(const Quaternion& q) const
{
	return w * q.w + x * q.x + y * q.y + z * q.z;
}

double Quaternion::magnitude() const
{
	return std::sqrt(x * x + y * y + z * z + w * w);
}

Quaternion Quaternion::normalize() const
{
	double m = magnitude();
	m = (!m) ? 1.0f : m; // avoid divide by zero

	return Quaternion(x/m, y/m, z/m, w/m);
}

Quaternion Quaternion::Slerp(const float& fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath)
{
	float fCos = rkP.dot(rkQ);
	Quaternion rkT;

	// invert rotation?
	if (fCos < 0.0f && shortestPath)
	{
		fCos = -fCos;
		rkT = -rkQ;
	}
	else
	{
		rkT = rkQ;
	}

	if (std::abs(fCos) < 1 - m_fEpsilon)
	{
// standard slerp
		float fSin = std::sqrt(1 - (fCos * fCos)); // Math::Sqr
		float fAngle = std::atan2(fSin, fCos); // Radian
		float fInvSin = 1.0f / fSin;
		float fCoeff0 = std::sin((1.0f - fT) * fAngle) * fInvSin;
		float fCoeff1 = std::sin(fT * fAngle) * fInvSin;
		return rkP * fCoeff0 + rkT * fCoeff1;
	}
	else
	{
// There are two situations:
// 1. "rkP" and "rkQ" are very close (fCos ~= +1), so we can do a 
// linear interpolation safely.
// 2. "rkP" and "rkQ" are almost inverse of each other (fCos ~= -1),
// there are an infinite number of possibilities interpolation. but we
// haven't have method to fix this case, so just use linear interpolation here.
		Quaternion t = rkP * (1.0f - fT) + rkT * fT;
		t = t.normalize();
		return t;
	}
}

Quaternion Quaternion::Squad(const float& fT, const Quaternion& rkP, const Quaternion& rkA, const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath)
{
	float fSlerpT = 2.0f * fT * (1.0f - fT);
	Quaternion kSlerpP = Slerp(fT, rkP, rkQ, shortestPath);
	Quaternion kSlerpQ = Slerp(fT, rkA, rkB, shortestPath);
	return Slerp(fSlerpT, kSlerpP, kSlerpQ, shortestPath);
}

Quaternion Quaternion::nlerp(const float& fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath)
{
	Quaternion result;
	float fCos = rkP.dot(rkQ);

	if (fCos < 0.0f && shortestPath)
	{
		result = rkP + ((-rkQ) - rkP) * fT;
	}
	else
	{
		result = rkP + (rkQ - rkP) * fT;
	}
	result = result.normalize();
	return result;
}
