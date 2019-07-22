#include "Boss.h"

Boss::Boss(const std::string& filename, const Quaternion& q, const Vector& vec):
	Object(filename, vec[0], vec[1], vec[2])
{
	state = AI_WAIT;
	m_ForwardVelocity = 0.0f;
	slerpCounter = 0.0f;

	mPosition = vec;
	mOrientation = q;
	mDirectionVector = Vector::ZERO;

	bossColor[0] = 0.0f;
	bossColor[1] = 0.0f;
	bossColor[2] = 0.0f;
	bossColor[3] = 1.0f;

	if (rand() % 4 > 2)
	{
		health = 3;
		m_MaxForwardVelocity = 2.0f;
		bossColor[0] = 1.0f;
	}
	else
	{
		health = 9;
		m_MaxForwardVelocity = 1.0f;
		bossColor[2] = 1.0f;
	}

	setRadius(8.0f);
	setType(G_BOSS);
} 

void Boss::Animate()
{

	// set our Direction
	if (state != AI_DIE)
	{
		mDirectionVector = mOrientation * Vector::NEGATIVE_UNIT_Z * m_ForwardVelocity;
	}
	else // (state == AI_DIE)
	{
		mDirectionVector = mOrientation * Vector::NEGATIVE_UNIT_Y;
	}

        // update position
	mPosition = getPosition();
        mPosition += mDirectionVector;
	updatePosition();
}

void Boss::Crash()
{
	// set our mOrientation post crash
	int m_heading = rand() % 360;
	mOrientation.CreateFromAxisAngle(Vector::UNIT_Y, (float)m_heading);
	m_ForwardVelocity = m_MaxForwardVelocity;
	slerpCounter = 0.0f;
	setCrash(false);
}

void Boss::LifeCycle()
{
	if (getCrash()) Crash();

	if (getType() == G_HIT_BOSS)
	{
		health--;
		setType(G_BOSS);
	}

	if (health < 0)
	{
		m_ForwardVelocity = 0.0f;
		state = AI_DIE;
		setType(G_DEAD_BOSS);
	}

}

void Boss::updatePosition()
{
	// check world edges
	if (mPosition[0] > WORLD_EDGE)
	{
		mPosition[0] = WORLD_EDGE;
		setCrash(true);
	}
	if (mPosition[0] < WORLD_VIEW)
	{
		mPosition[0] = WORLD_VIEW;
		setCrash(true);

	}

	// don't fly away because of slerp
	if (mPosition[1] > WORLD_CEILING)
	{
		mPosition[1] = WORLD_CEILING;
	}

	if (mPosition[2] > WORLD_EDGE)
	{
		mPosition[2] = WORLD_EDGE;
		setCrash(true);
	}
	if (mPosition[2] < WORLD_VIEW)
	{
		mPosition[2] = WORLD_VIEW;
		setCrash(true);
	}

	setPosition(mPosition[0], mPosition[1], mPosition[2]);
}

void Boss::updateState(const Quaternion& q, const Vector& vec, const float& time)
{
	float dist = distance(vec);

	if (state != AI_DIE)
	{
		// Bosses run away!
		if (dist <= WORLD_EDGE)
		{
			state = AI_MOVE;
			m_ForwardVelocity = m_MaxForwardVelocity; 

			if (slerpCounter < 1.0f)
			{
				Quaternion qs;
				mOrientation = qs.Slerp(slerpCounter, mOrientation, q, false);
				slerpCounter += time;
			}
		}
		else
		{
			state = AI_WAIT;
			m_ForwardVelocity = 0.0f;
			slerpCounter = 0.0f;
		}
	}

	if (state == AI_DIE)
	{
		if (mPosition[1] < WORLD_HELL)
		{
			Shutdown();
		}
	}
			
}

void Boss::Show()
{
	float size = getRadius();

	Animate();
	LifeCycle();

	if (!getVisible()) return;

	glEnable(GL_FOG);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, getTexture());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(bossColor[0], bossColor[1], bossColor[2], bossColor[3]);

	// translate and scale
	glTranslatef(mPosition[0], mPosition[1], mPosition[2]);
	glScalef(size,size,size);

	//3D cube
	glBegin(GL_QUADS);
	// front
	glTexCoord2f(0.0f,0.0f);glVertex3f(-1.0f,-1.0f, 1.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f( 1.0f,-1.0f, 1.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f( 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f(-1.0f, 1.0f, 1.0f);
	// back
	glTexCoord2f(1.0f,0.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f(-1.0f, 1.0f,-1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f( 1.0f, 1.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f( 1.0f,-1.0f,-1.0f);
	// top
	glTexCoord2f(0.0f,1.0f);glVertex3f(-1.0f, 1.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f( 1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f( 1.0f, 1.0f,-1.0f);
	// bottom
	glTexCoord2f(1.0f,1.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f( 1.0f,-1.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f( 1.0f,-1.0f, 1.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f(-1.0f,-1.0f, 1.0f);
	// right
	glTexCoord2f(1.0f,0.0f);glVertex3f( 1.0f,-1.0f,-1.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f( 1.0f, 1.0f,-1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f( 1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f( 1.0f,-1.0f, 1.0f);
	// left
	glTexCoord2f(0.0f,0.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f(-1.0f,-1.0f, 1.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f(-1.0f, 1.0f,-1.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D); // switch back to colors
	glDisable(GL_FOG);
}
