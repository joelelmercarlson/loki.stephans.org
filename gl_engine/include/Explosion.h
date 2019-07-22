#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "Object.h"
#include "Particle.h"

#ifndef FRAND
#define FRAND (((float)rand()-(float)rand())/RAND_MAX)
#endif

const Vector PARTICLE_VELOCITY(0.0f,3.0f,0.0f);
const Vector VELOCITY_VARIATION(8.0f,8.0f,8.0f);
const Vector PARTICLE_ACCELERATION(0.0f,-5.0f,0.0f);
const float PARTICLE_SIZE = 5.0f;
const float SIZE_VARIATION = 2.0f;

class Explosion : public Object, public ParticleSystem
{
public:
	Explosion(const std::string& filename, int numParticles, const Vector& vec, float spread);

	void Show();

	void Update(float elapsedTime);
protected:
	void InitializeParticle(int index);
	float m_spread;
};
#endif
