#ifndef QUATERNION_H
#define QUATERNION_H

#include <cmath>
#include <iostream>
#include "Vector.h"
#include "Matrix.h"

class Quaternion
{
private:
	double x;
	double y;
	double z;
	double w;
public:
	Quaternion();
	Quaternion(double x, double y, double z, float w);
	Quaternion(const Vector& v, float w);
	~Quaternion();

	Quaternion operator*(const Quaternion q) const;
	Vector operator*(const Vector& v) const;
	Quaternion operator*(const float& scale) const;
	Quaternion operator-() const;
	Quaternion operator+(const Quaternion& q) const;
	Quaternion operator-(const Quaternion& q) const;

	void CreateMatrix(float *pMatrix);
	void CreateFromAxisAngle(const Vector& axis, float degrees);
	void CreateFromAxisAngle(const float& in_x, const float& in_y, const float& in_z, float degrees);
	void CreateFromAxes(const Vector& xaxis, const Vector& yaxis, const Vector& zaxis);
	void CreateFromRotationMatrix(const MATRIX3& kRot);

	double dot(const Quaternion& q) const;
	double magnitude() const;
	Quaternion normalize() const;

	// Quaternion interploation
	Quaternion Slerp(const float& fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath);
	Quaternion Squad(const float& fT, const Quaternion& rkP, const Quaternion& rkA, const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath);
	Quaternion nlerp(const float& fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath);

	friend std::ostream &operator<<(std::ostream& os, const Quaternion& q)
	{
		return std::cout << "{" << q.x << ", " <<  q.y << ", " << q.z << ", " << q.w << "}" << std::endl; 
        }

	static const float m_fEpsilon;
	static const Quaternion IDENTITY;
	static const Quaternion ZERO;
};
#endif
