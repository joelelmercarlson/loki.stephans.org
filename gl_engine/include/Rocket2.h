#ifndef ROCKET2_H
#define ROCKET2_H

#include "Explosion2.h"
#include "Object.h"

class Rocket2 : public Object
{
private:
	float m_ForwardVelocity;
	float distance;
	float distanceMax;
	bool isExploding;
	Explosion2 *explosion;
public:
	Rocket2(const std::string& filename, const Quaternion& q, const Vector& vec);
	~Rocket2();

	void Animate();
	void Crash() {};
	void LifeCycle();
	void Show();

	float rocketColor[4];
protected:
	Vector mPosition;
	Quaternion mOrientation;	
	Vector mDirectionVector;

	void updatePosition();
};
#endif
