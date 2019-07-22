#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>
#include <iostream>

class Vector
{
private:
	double v[3];
public:
	Vector();
	Vector(double x, double y, double z);
	~Vector();

	Vector cross(const Vector& A) const;
	Vector normalize() const;
	double dot(const Vector& A) const;
	double magnitude() const;
	double magnitude_squared() const;

	double &operator[](int index);
	double operator[](int index) const;
	Vector operator+(const Vector& A) const;
	Vector operator-(const Vector& A) const;
	bool   operator==(const Vector& A) const;
	Vector operator-() const;

	const Vector &operator*(float scale);
	const Vector &operator/(float scale);
	const Vector &operator+=(const Vector& A);
	const Vector &operator-=(const Vector& A);
	const Vector &operator*=(float scale);

	friend std::ostream &operator<<(std::ostream& os, const Vector& A)
	{
		return std::cout << "{" << A[0] << ", " <<  A[1] << ", " << A[2] << "}" << std::endl; 
	}

	static const Vector ZERO;
	static const Vector UNIT_X;
	static const Vector UNIT_Y;
	static const Vector UNIT_Z;
	static const Vector NEGATIVE_UNIT_X;
	static const Vector NEGATIVE_UNIT_Y;
	static const Vector NEGATIVE_UNIT_Z;
};
#endif
