#include "Vector.h"

const Vector Vector::ZERO(0,0,0);
const Vector Vector::UNIT_X(1,0,0);
const Vector Vector::UNIT_Y(0,1,0);
const Vector Vector::UNIT_Z(0,0,1);
const Vector Vector::NEGATIVE_UNIT_X(-1,0,0);
const Vector Vector::NEGATIVE_UNIT_Y(0,-1,0);
const Vector Vector::NEGATIVE_UNIT_Z(0,0,-1);

Vector::Vector()
{
	v[0] = 0.0f;
	v[1] = 0.0f;
	v[2] = 0.0f;
}

Vector::Vector(double x, double y, double z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

Vector::~Vector()
{
}

double &Vector::operator[](int index)
{
	return v[index];
}

double Vector::operator[](int index) const
{
	return v[index];
}

bool Vector::operator==(const Vector& A) const
{
	if (v[0] == A[0] && v[1] == A[1] && v[2] == A[2])
		return true;
	else
		return false;
}

Vector Vector::operator-() const
{
	return Vector(-v[0], -v[1], -v[2]);
}

Vector Vector::cross(const Vector& A) const
{
	return Vector(
		v[1] * A.v[2] - v[2] * A.v[1],
		v[2] * A.v[0] - v[0] * A.v[2],
		v[0] * A.v[1] - v[1] * A.v[0]);
}

Vector Vector::normalize() const
{
	double m = magnitude();
	m = (!m) ? 1.0f : m; // avoid divide by zero

	return Vector(v[0]/m, v[1]/m, v[2]/m);
}

double Vector::dot(const Vector& A) const
{
        return (v[0] * A.v[0] + v[1] * A.v[1] + v[2] * A.v[2]);
}

double Vector::magnitude() const
{
	return std::sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

double Vector::magnitude_squared() const
{
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

Vector Vector::operator+(const Vector& A) const
{
	return Vector(v[0] + A.v[0], v[1] + A.v[1], v[2] + A.v[2]);
}

Vector Vector::operator-(const Vector& A) const
{
	return Vector(v[0] - A.v[0], v[1] - A.v[1], v[2] - A.v[2]);
}

const Vector &Vector::operator*(float scale)
{
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;

        return *this;
}

const Vector &Vector::operator/(float scale)
{
	float recip = 1/scale;
        v[0] *= recip;
        v[1] *= recip;
        v[2] *= recip;

        return *this;
}

const Vector &Vector::operator+=(const Vector& A)
{
        v[0] += A.v[0];
        v[1] += A.v[1];
        v[2] += A.v[2];

        return *this;
}

const Vector &Vector::operator-=(const Vector& A)
{
        v[0] -= A.v[0];
        v[1] -= A.v[1];
        v[2] -= A.v[2];

        return *this;
}

const Vector &Vector::operator*=(float scale)
{
        v[0] *= scale;
        v[1] *= scale;
        v[2] *= scale;

        return *this;
}
