#ifndef EXPLOSION2_H
#define EXPLOSION2_H

#include "Object.h"
#include "Particle.h"

#ifndef FRAND
#define FRAND (((float)rand()-(float)rand())/RAND_MAX)
#endif

const Vector PARTICLE_VELOCITY2(2.0f,4.0f,2.0f);
const Vector VELOCITY_VARIATION2(9.0f,9.0f,9.0f);
const Vector PARTICLE_ACCELERATION2(0.0f,-10.0f,0.0f);
const float PARTICLE_SIZE2 = 1.0f;
const float SIZE_VARIATION2 = 5.0f;

class Explosion2 : public Object, public ParticleSystem
{
public:
	Explosion2(const std::string& filename, int numParticles, const Vector& vec, float spread);

	void Show();

	void Update(float elapsedTime);
protected:
	void InitializeParticle(int index);
	float m_spread;
};
#endif
