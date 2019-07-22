#ifndef ROCKET_H
#define ROCKET_H

#include "Explosion.h"
#include "Object.h"

class Rocket : public Object
{
private:
	float m_ForwardVelocity;
	float distance;
	float distanceMax;
	bool isExploding;
	Explosion *explosion;
public:
	Rocket(const std::string& filename, const Quaternion& q, const Vector& vec);
	~Rocket();

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
