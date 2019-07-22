#include "Particle.h"

ParticleSystem::ParticleSystem(int maxParticles, const Vector& vec)
{
	m_maxParticles = maxParticles;
	m_origin = vec;
	m_particleList = NULL;
}

int ParticleSystem::Emit(int numParticles)
{
	while (numParticles && (m_numParticles < m_maxParticles))
	{
		InitializeParticle(m_numParticles++);
		--numParticles;
	}
	return numParticles;
}


void ParticleSystem::InitializeSystem()
{
	if (m_particleList)
	{
		delete[] m_particleList;
		m_particleList = NULL;
	}
	m_particleList = new particle_t[m_maxParticles];
	m_numParticles = 0;
	m_accumulatedTime = 0.0f;
}

void ParticleSystem::KillSystem()
{
	if (m_particleList)
	{
		delete[] m_particleList;
		m_particleList = NULL;
	}
	m_numParticles = 0;
}
