#include "Camera.h"

Camera::Camera():
	Object(0.0f, 0.0f, 0.0f),
	FOVy(45.0f),
	aspect(SCREEN_WIDTH/SCREEN_HEIGHT),
	zNear(1.0f),
	zFar(1000.0f),
	height(SCREEN_HEIGHT),
	width(SCREEN_WIDTH),
	m_pitch(0.0f),
	m_heading(0.0f),
	m_ForwardVelocity(0.0f),
	m_MaxForwardVelocity(5.0f),
	m_StrafeVelocity(0.0f),
	m_MaxStrafeVelocity(1.0f),
	m_MaxHeading(360.0f),
	m_MaxHeadingRate(5.0f),
	m_MaxPitch(89.5f),
	m_MaxPitchRate(5.0f)
{
	mPosition = Vector::ZERO;
	mOrientation = Quaternion::IDENTITY;
	mDirectionVector = Vector::ZERO;
	mStrafeVector = Vector::ZERO;
	mYawFixedAxis = Vector::UNIT_Y;
	frustum = new Frustum;
	viewOutOfDate = true;
	lookAtMode = false;
	
	setRadius(8.0f);
	setType(G_PLAYER);
}

Camera::~Camera()
{
	delete frustum;
}

void Camera::Animate(Input &input)
{
	// save the input buffer
	i = input;

	if (input.crash || getCrash())
	{
		setCrash(false);
		input.vel = 0.0f;
		input.strafe = 0.0f;
		input.heading = 0.0f;
		input.crash = false;
		m_ForwardVelocity = 0.0f;
		m_StrafeVelocity = 0.0f;
	}

	ChangePitch(input.pitch);
	ChangeHeading(input.heading);
	ChangeVelocity(m_ForwardVelocity, input.vel, m_MaxForwardVelocity);
	ChangeVelocity(m_StrafeVelocity, input.strafe, m_MaxStrafeVelocity);

	// clear inputs
	input.vel = 0.0f;
	input.strafe = 0.0f;
	input.pitch = 0.0f;

	if (std::abs(input.heading) != std::abs(input.rotate)) input.heading = 0.0f;

	// dump camera
	if (input.debug && !viewOutOfDate)
	{
		std::cout << "==========" << std::endl;
		std::cout << "vector     = " << mPosition;
		std::cout << "quaternion = " << mOrientation;
		std::cout << "direction  = " << mOrientation * Vector::NEGATIVE_UNIT_Z;
		std::cout << "pitch      = " << m_pitch << std::endl;
		std::cout << "heading    = " << m_heading << std::endl;
		std::cout << "velocity   = " << m_ForwardVelocity << std::endl;
		std::cout << "strafe     = " << m_StrafeVelocity << std::endl;
		std::cout << "fps        = " << input.fps << std::endl;
	}

}

void Camera::ChangePitch(float degrees)
{
	if (std::abs(degrees) < std::abs(m_MaxPitchRate))
	{
		m_pitch += degrees;
	}
	else
	{
		if (degrees < 0)
		{
			// pitching down
			m_pitch -= m_MaxPitchRate;
		}
		else
		{
			// pitching up
			m_pitch += m_MaxPitchRate;
		}
	}

	if (m_pitch > m_MaxPitch)
		m_pitch = m_MaxPitch;
	
	if (m_pitch < -m_MaxPitch)
		m_pitch = -m_MaxPitch;
}

void Camera::ChangeHeading(float degrees)
{
	if (std::abs(degrees) < std::abs(m_MaxHeadingRate))
	{
		// check if inverted
		if ((m_pitch > 90 && m_pitch < 270)||(m_pitch < -90 && m_pitch > -270))
		{
			m_heading -= degrees;
		}
		else
		{
			m_heading += degrees;
		}
	}
	else
	{
		if (degrees < 0)
		{
			// check if inverted
			if ((m_pitch > 90 && m_pitch < 270)||(m_pitch < -90 && m_pitch > -270))
			{
				// upside down
				m_heading += m_MaxHeadingRate;
			}
			else
			{
				m_heading -= m_MaxHeadingRate;
			}
		}
		else
		{
			// check if inverted
			if ((m_pitch > 90 && m_pitch < 270)||(m_pitch < -90 && m_pitch > -270))
			{
				// upside down
				m_heading -= m_MaxHeadingRate;
			}
			else
			{
				m_heading += m_MaxHeadingRate;
			}
		}
	}

	if (m_heading > m_MaxHeading)
		m_heading -= m_MaxHeading;
	else if (m_heading < -m_MaxHeading)
		m_heading += m_MaxHeading;
}

void Camera::ChangeVelocity(float &currentSpeed, float vel, float maxSpeed)
{
	currentSpeed += vel;

	if (currentSpeed > maxSpeed)
		currentSpeed = maxSpeed;
	else if (currentSpeed < -maxSpeed)
		currentSpeed = -maxSpeed;
}

Vector Camera::getDirection() const
{
	return mOrientation * Vector::NEGATIVE_UNIT_Z;
}

Vector Camera::getUp() const
{
	return mOrientation * Vector::UNIT_Y;
}

Vector Camera::getRight() const
{
	return mOrientation * Vector::UNIT_X;
}

float Camera::getDistance(const Vector& vec, float radius)
{
	return frustum->f_SphereInFrustum(vec[0],vec[1],vec[2],radius);
}

bool Camera::getView(const Vector& vec, float radius)
{
	return frustum->SphereInFrustum(vec[0],vec[1],vec[2],radius);
}

void Camera::lookAt(const float& x, const float& y, const float& z)
{
	Vector temp(x,y,z);
	this->lookAt(temp);
}

void Camera::lookAt(const Vector& target)
{
	updateView();
	this->setDirection(target - mPosition);
}

void Camera::move(const Vector& vec)
{
	mPosition = getPosition();
	mPosition = mPosition + vec;

	updatePosition();
	invalidateView();
}

void Camera::roll(const float& angle)
{
	Vector zAxis = mOrientation * Vector::UNIT_Z;
	rotate(zAxis, angle);
}

void Camera::pitch(const float& angle)
{
	Vector xAxis = mOrientation * Vector::UNIT_X;
	rotate(xAxis, angle);
}

void Camera::yaw(const float& angle)
{
	Vector yAxis = mYawFixedAxis;
	rotate(yAxis, angle);
}

void Camera::rotate(const Vector& axis, const float& angle)
{
	Quaternion q;
	q.CreateFromAxisAngle(axis, angle);
	rotate(q);
}
 
void Camera::rotate(const Quaternion& q)
{
	mOrientation = q.normalize() * mOrientation;
	invalidateView();
}

void Camera::setCameraPerspective()
{
	// call gluPerspective
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0,0,width,height);
	gluPerspective(FOVy, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	invalidateView();
}

void Camera::setCameraPosition(const Vector& vec)
{
	mPosition = vec;

	updatePosition();
	invalidateView();
}

void Camera::setDirection(const Vector& vec)
{
	if (vec == Vector::ZERO) return;

	lookAtMode = true;

	Vector zAdjustVec = -vec;
	zAdjustVec = zAdjustVec.normalize();

	Quaternion targetOrientation;

	Vector xVec = mYawFixedAxis.cross( zAdjustVec );
	xVec = xVec.normalize();

	Vector yVec = zAdjustVec.cross( xVec );
	yVec = yVec.normalize();

	targetOrientation.CreateFromAxes( xVec, yVec, zAdjustVec);

	mOrientation = targetOrientation;

	invalidateView();
}

void Camera::setPerspective()
{
	MATRIX4 Matrix;

	// clean slate for every frame
	glLoadIdentity();

	if (!lookAtMode) mOrientation = Quaternion::IDENTITY;

	mDirectionVector = Vector::ZERO;
	mStrafeVector = Vector::ZERO;

	// camera rotations
	pitch(m_pitch);
	yaw(m_heading);

	// set our Direction (z-axis)
	mDirectionVector = mOrientation * Vector::NEGATIVE_UNIT_Z * m_ForwardVelocity;

	// set our Strafe (x-axis)
	mStrafeVector = mOrientation * Vector::UNIT_X * m_StrafeVelocity;
	mStrafeVector[1] = 0.0f;

	// move based on our movement vectors
	move(mDirectionVector + mStrafeVector);
	
	// set our view from mOrientation and mPosition
	mOrientation.CreateMatrix(Matrix);
	glMultMatrixf(Matrix);
	glTranslatef(-mPosition[0], -mPosition[1], -mPosition[2]);

	// update frustum
	updateView();
}

void Camera::updatePosition()
{
	// check collision with world max, min edges
	mPosition[0] = (mPosition[0] > i.right)   ? i.right   : mPosition[0];
	mPosition[1] = (mPosition[1] > i.ceiling) ? i.ceiling : mPosition[1];
	mPosition[2] = (mPosition[2] > i.far)     ? i.far     : mPosition[2];
	
	// stay in top-right quadrant with 1.0f the limit
 	mPosition[0] = (mPosition[0] < i.left)  ? i.left  : mPosition[0];
	mPosition[1] = (mPosition[1] < i.floor) ? i.floor : mPosition[1];
	mPosition[2] = (mPosition[2] < i.near)  ? i.near  : mPosition[2];

	// locked y as in first-person-shooter
	if (i.first) mPosition[1] = i.floor;

	// save position in Object
	setPosition(mPosition[0], mPosition[1], mPosition[2]);
}

void Camera::updateView()
{
	if (viewOutOfDate)
	{
		frustum->ExtractFrustum();
		viewOutOfDate = false;
		lookAtMode = false;
	}
}
