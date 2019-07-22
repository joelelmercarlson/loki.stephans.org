#include "Rocket2.h"

Rocket2::Rocket2(const std::string& filename, const Quaternion& q, const Vector& vec):
	Object(filename, vec[0], vec[1], vec[2])
{
	m_ForwardVelocity = 10.5f;
	distance = 0.0f;
	distanceMax = vec.magnitude() * m_ForwardVelocity * 4.75f;
	isExploding = false;
	explosion = NULL;

	rocketColor[0] = 0.0f;
	rocketColor[1] = 0.0f;
	rocketColor[2] = 1.0f;
	rocketColor[3] = 1.0f;

	mPosition = vec;
	mOrientation = q;
	mDirectionVector = Vector::ZERO;

	setRadius(4.5f);
	setType(G_ROCKET);
} 

Rocket2::~Rocket2()
{
	if (explosion != NULL)
	{
		explosion->KillSystem();
		delete [] explosion;
		explosion = NULL;
	}
}

void Rocket2::Animate()
{
	Vector mGravity(0.0f,-0.123f,0.0f);

	// set our Direction
	mDirectionVector = mOrientation * Vector::NEGATIVE_UNIT_Z;

	// scale our vector
	mDirectionVector *= m_ForwardVelocity;

	// add gravity
	mDirectionVector += mGravity;

	// update position
	mPosition += mDirectionVector;
	updatePosition();
}

void Rocket2::LifeCycle()
{

	if ((!isExploding) && getType() == G_HIT_EXPLOSION)
	{
		isExploding = true;
		m_ForwardVelocity = 0.0f;
		explosion = new Explosion2("media/explosion2.bmp",1500,mPosition,8.5f);
	}

	if ((!isExploding) && getType() == G_HIT)
	{
		isExploding = true;
		m_ForwardVelocity = 0.0f;
		explosion = new Explosion2("media/explosion2.bmp",10,mPosition,0.1f);
	}

	if (distance >= distanceMax)
	{
		setType(G_HIT);
	}

	if (isExploding)
	{
		if (explosion->getType() == G_GARBAGE)
		{
			Shutdown();
		}
	}
}

void Rocket2::updatePosition()
{
	// distance traveled
	distance += mPosition.magnitude();

	// save object position
	setPosition(mPosition[0],mPosition[1],mPosition[2]);
}

void Rocket2::Show()
{
	Animate();
	LifeCycle();

	if (isExploding)
	{
		explosion->Show();
	}
	else  // Rocket2 in flight
	{
		float size = 0.15f;

		glDisable(GL_FOG);
		glDepthMask(GL_FALSE);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, getTexture());
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor4f(rocketColor[0],rocketColor[1],rocketColor[2],rocketColor[3]);

                glTranslatef(mPosition[0],mPosition[1],mPosition[2]);
                glScalef(size,size,size);

                glBegin(GL_TRIANGLES);
                //top
                glTexCoord2f(0.0f,0.0f);glVertex3f(0.0f,1.0f,0.0f);
                glTexCoord2f(1.0f,0.0f);glVertex3f(-1.0f,-1.0f,1.0f);
                glTexCoord2f(1.0f,1.0f);glVertex3f(1.0f,-1.0f,1.0f);
                //right
                glTexCoord2f(0.0f,1.0f);glVertex3f(0.0f,1.0f,0.0f);
                glTexCoord2f(0.0f,0.0f);glVertex3f(1.0f,-1.0f,1.0f);
                glTexCoord2f(1.0f,0.0f);glVertex3f(1.0f,-1.0f,-1.0f);
                //back
                glTexCoord2f(1.0f,1.0f);glVertex3f(0.0f,1.0f,0.0f);
                glTexCoord2f(0.0f,1.0f);glVertex3f(1.0f,-1.0f,-1.0f);
                glTexCoord2f(0.0f,0.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
                //left
                glTexCoord2f(1.0f,0.0f);glVertex3f(0.0f,1.0f,0.0f);
                glTexCoord2f(1.0f,1.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
                glTexCoord2f(0.0f,1.0f);glVertex3f(-1.0f,-1.0f,1.0f);
                glEnd();

		glDisable(GL_TEXTURE_2D);
		glDepthMask(GL_TRUE);
		glEnable(GL_FOG);
	}
}
