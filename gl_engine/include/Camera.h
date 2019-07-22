#ifndef CAMERA_H
#define CAMERA_H

#include "Frustum.h"
#include "Object.h"

class Camera : public Object
{
private:
	// gluPerspective calls
	double FOVy; 
	double aspect;
	double zNear;
	double zFar;
	int height;
	int width;

	float m_pitch;
	float m_heading;
	float m_ForwardVelocity;
	float m_MaxForwardVelocity;
	float m_StrafeVelocity;
	float m_MaxStrafeVelocity;
	float m_MaxHeading;
	float m_MaxHeadingRate;
	float m_MaxPitch;
	float m_MaxPitchRate;
	Vector mYawFixedAxis;
	bool viewOutOfDate;
	bool lookAtMode;
	Input i;
public:
	Camera();
	~Camera();

	void Animate(Input &input);
	void ChangePitch(float degrees);
	void ChangeHeading(float degrees);
	void ChangeVelocity(float &currentSpeed, float vel, float maxSpeed);
	Vector getDirection() const;
	Vector getUp() const;
	Vector getRight() const;
	Quaternion getOrientation() const { return mOrientation; };
	float getPitch() const { return m_pitch; };
	float getHeading() const { return m_heading; };
	float getDistance(const Vector& vec, float radius); 
	bool getView(const Vector& vec, float radius);
	void invalidateView() { viewOutOfDate = true; };
	void lookAt(const float& x, const float& y, const float &z);
	void lookAt(const Vector& target);
	void move(const Vector& vec);
	void roll(const float& angle);
	void pitch(const float& angle);
	void yaw(const float& angle);
	void rotate(const Vector& axis, const float& angle);
	void rotate(const Quaternion& q);
	void setAspect(double const& a) { aspect = a; };
	void setFOVy(double const& a) { FOVy = a; };
	void setNearClip(double const& a) { zNear = a; };
	void setFarClip(double const& a) { zFar = a; };
	void setScreenHeight(int const& a) { height = a; };
	void setScreenWidth(int const& a) { width = a; };
	void setCameraPerspective();
	void setCameraPosition(const Vector& vec);
	void setDirection(const Vector& vec);
	void setPerspective();
	void updateView();
	void Show() {};
protected:
	Vector mPosition;        // default 0,0,0
	Quaternion mOrientation; // camera orientation
	Vector mDirectionVector; // direction of camera
	Vector mStrafeVector;    // strafe
	Frustum* frustum;        // viewing volume
	void updatePosition();
};
#endif
