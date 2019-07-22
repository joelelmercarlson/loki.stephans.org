#include "Explosion.h"

Explosion::Explosion(const std::string& filename, int numParticles, const Vector& vec, float spread) : 
	Object(filename, vec[0], vec[1], vec[2]),
	ParticleSystem(numParticles,vec),
	m_spread(spread)
{
	ParticleSystem::InitializeSystem();
	Emit(numParticles);
	setType(G_DECORATION);
}

void Explosion::InitializeParticle(int index)
{
	// position and size
	m_particleList[index].m_pos[0] = m_origin[0] + FRAND * m_spread;
	m_particleList[index].m_pos[1] = m_origin[1] + FRAND * m_spread;
	m_particleList[index].m_pos[2] = m_origin[2] + FRAND * m_spread;
	m_particleList[index].m_size = PARTICLE_SIZE + FRAND * SIZE_VARIATION;

	// give the particle a random velocity
	m_particleList[index].m_velocity[0] = PARTICLE_VELOCITY[0] + FRAND * VELOCITY_VARIATION[0];
	m_particleList[index].m_velocity[1] = PARTICLE_VELOCITY[1] + FRAND * VELOCITY_VARIATION[1];
	m_particleList[index].m_velocity[2] = PARTICLE_VELOCITY[2] + FRAND * VELOCITY_VARIATION[2];
	m_particleList[index].m_acceleration = PARTICLE_ACCELERATION;

	// colors in yellow and red
	m_particleList[index].m_color[0] = 1.0f;
	m_particleList[index].m_color[1] = 1.0f*FRAND;
	m_particleList[index].m_color[2] = 0.4f*FRAND;
	m_particleList[index].m_color[3] = 1.0f;

	// how much energy?
	m_particleList[index].m_energy = 1.5f + FRAND/2.0f;

	// how fast to alpha?
	m_particleList[index].m_colorDelta[0] = 0.0f;
	m_particleList[index].m_colorDelta[1] = 0.0f;
	m_particleList[index].m_colorDelta[2] = 0.0f;
	m_particleList[index].m_colorDelta[3] = -1.0f/m_particleList[index].m_energy;
	m_particleList[index].m_sizeDelta = 8.0f;

}

void Explosion::Show()
{
	Update(0.1f);

        // billboarding technique
	MATRIX4 Matrix;
        glGetFloatv(GL_MODELVIEW_MATRIX,Matrix);
        Vector right(Matrix[0],Matrix[4],Matrix[8]);
        Vector up(Matrix[1],Matrix[5],Matrix[9]);

        glDisable(GL_FOG);
	glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, getTexture());
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);

        glBegin(GL_QUADS);
	for (int i=0; i < m_numParticles; ++i)
	{
		float size = m_particleList[i].m_size/2;
		Vector pos = m_particleList[i].m_pos;

		Vector bl = pos + (right + up) * -size; // bottom-left
		Vector br = pos + (right - up) * size;  // bottom-right
		Vector tr = pos + (right + up) * size;  // top-right
		Vector tl = pos + (up - right) * size;  // top-left

		glColor4fv(m_particleList[i].m_color);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(bl[0],bl[1],bl[2]);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(br[0],br[1],br[2]);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(tr[0],tr[1],tr[2]);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(tl[0],tl[1],tl[2]);
	}
        glEnd();

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
        glEnable(GL_FOG);
}

void Explosion::Update(float elapsedTime)
{
	for (int i = 0; i < m_numParticles; )
	{
		m_particleList[i].m_pos = m_particleList[i].m_pos + m_particleList[i].m_velocity * elapsedTime;
		m_particleList[i].m_velocity = m_particleList[i].m_velocity + m_particleList[i].m_acceleration * elapsedTime;

		m_particleList[i].m_energy -= elapsedTime;
		m_particleList[i].m_size += m_particleList[i].m_sizeDelta * elapsedTime;

		m_particleList[i].m_color[3] += m_particleList[i].m_colorDelta[3] * elapsedTime;
		m_particleList[i].m_color[1] += m_particleList[i].m_colorDelta[1] * elapsedTime;

		if (m_particleList[i].m_energy <= 0.0f)
		{
			m_particleList[i] = m_particleList[--m_numParticles];
		}
		else
		{
			++i;
		}
	}

	if (m_numParticles < 1) Shutdown();
}
