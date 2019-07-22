#ifndef PARTICLE_H
#define PARTICLE_H

#include "Vector.h"

struct particle_t
{
        Vector m_pos;
        Vector m_prevPos;
        Vector m_velocity;
        Vector m_acceleration;
        float m_energy;
        float m_size;
        float m_sizeDelta;
        float m_weight;
        float m_weightDelta;
        float m_color[4];
        float m_colorDelta[4];
};

class ParticleSystem
{
public:
	ParticleSystem(int maxParticles, const Vector& vec);

	virtual int Emit(int numParticles);
	virtual void InitializeSystem();
	virtual void Update(float elapsedTime) = 0;
	virtual void KillSystem();
protected:
	virtual void InitializeParticle(int index) = 0;
	particle_t* m_particleList;
	int m_maxParticles;
	int m_numParticles;
	Vector m_origin;
	float m_accumulatedTime;
};

#endif
