#ifndef BOSS_H
#define BOSS_H

#include "Object.h"

enum MODEL_STATE {
	AI_WAIT,
	AI_MOVE,
	AI_DIE
};

class Boss : public Object
{
private:
	MODEL_STATE state;
	float m_ForwardVelocity;
	float m_MaxForwardVelocity;
	float slerpCounter;
	int health;
	float bossColor[4];
public:
	Boss(const std::string& filename, const Quaternion& q, const Vector& vec);
	~Boss();

	void Animate();
	void Crash();
	void LifeCycle();
	void Show();
protected:
	Vector mPosition;
	Quaternion mOrientation;
	Vector mDirectionVector;

	void updatePosition();
	void updateState(const Quaternion& q, const Vector& vec, const float& time);
};
#endif
