#ifndef SCENERY_H
#define SCENERY_H

#include "Object.h"

class Scenery : public Object
{
public:
	Scenery(const std::string& filename, const Quaternion& q, const Vector& vec);
	~Scenery();

	void Animate() {};
	void Crash() {};
	void LifeCycle() {};
	void Show();
protected:
	Vector mPosition;
	Quaternion mOrientation;

	void updateState(const Quaternion& q, const Vector& vec, const float& time);
};
#endif
