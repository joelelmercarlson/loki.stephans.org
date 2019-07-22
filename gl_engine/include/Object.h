#ifndef OBJECT_H
#define OBJECT_H

#include "Engine.h"
#include "SDL_LoadImage.h"

class Object
{
private:
	SDL_Surface *image;
	GLuint texture;    // opengl texture
	Vector m_position; // now position
	bool crash;        // did we crash?
	float radius;      // circle = (w+h)/4
	bool visible;      // are we visible?
	GAME_TYPE type;    // what are we?
public:
	Object();
	Object(float x, float y, float z);
	Object(const std::string& filename, float x, float y, float z);
	~Object();

	bool checkCollision(Object* A);
	float distance(const Vector& vec);
	bool getCrash() const { return crash; };
	Vector getPosition() const { return m_position; };
	float getRadius() const { return radius; };
	GLuint getTexture() const { return texture; };
	GAME_TYPE getType() const { return type; };
	bool getVisible() const { return visible; };
	void setCrash(bool a) { crash = a; };
	void setHeight(const float& h);
	void setOBJECTcolor(const float& r, const float& g, const float& b);
	void setPosition(const Vector& vec);
	void setPosition(const float& x, const float& y, const float& z);
	void setRadius(const float& a) { radius = a; };
	void setType(const GAME_TYPE& a) { type = a; };
	void setVisible(bool a) { visible = a; };
	void Shutdown();

	// virtual functions
	virtual void Show() {};
	virtual void updatePosition() {};
	virtual void updateState(const float& time) {};
	virtual void updateState(const char *str) {};
	virtual void updateState(const Quaternion& q, const Vector& vec, const float& time) {};

protected:
	float r, g, b; // color values
};
#endif
