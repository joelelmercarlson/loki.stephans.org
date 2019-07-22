#include "Object.h"

Object::Object():
	crash(false),
	radius(0.0f),
	visible(false),
	r(0.0f),
	g(0.0f),
	b(0.0f)
{
}

Object::Object(float x, float y, float z):
	crash(false),
	radius(0.0f),
	visible(true),
	r(0.0f),
	g(0.0f),
	b(0.0f)
{
	setPosition(x,y,z);
}

Object::Object(const std::string& filename, float x, float y, float z):
	crash(false),
	radius(0.0f),
	visible(true),
	r(0.0f),
	g(0.0f),
	b(0.0f)
{
	GLfloat texcoord[4];
	image = new SDL_Surface;

	setPosition(x,y,z);

	image = SDL_LoadImage(filename);
	texture = SDL_GL_LoadTexture(image, texcoord);

	SDL_FreeSurface(image);
}

Object::~Object()
{
}

void Object::Shutdown()
{
	if (texture > 0) glDeleteTextures(1, &texture);
	crash = false;
	radius = 0.0f;
	visible = false;
	r = 0.0f;
	g = 0.0f;
	b = 0.0f;
	type = G_GARBAGE;
}

// collision detection
bool Object::checkCollision(Object* A)
{
	// if we are the same or visible?
	if (A == this) return false;
	if (!A->getVisible()) return false;

	float r = getRadius() + A->getRadius();
	float d = distance(A->getPosition());

	switch (getType())
	{
		case G_PLAYER:
			if (d > EPSILON && d <= r)
			{
				if (A->getType() == G_BOSS)
				{
					setCrash(true);
				}
				else if (A->getType() == G_HIT_BOSS)
				{
					setCrash(true);
				}
				else if (A->getType() == G_DECORATION)
				{
					m_position += Vector::UNIT_X;
					setCrash(true);
				}
			}
		break;
		case G_BOSS:
			if (d > EPSILON && d <= r)
			{
				if (A->getType() == G_PLAYER)
				{
					setCrash(true);
				}
				else if (A->getType() == G_BOSS)
				{
					setCrash(true);
				}
				else if (A->getType() == G_HIT_BOSS)
				{
					setCrash(true);
				}
				else if (A->getType() == G_ROCKET)
				{
					setType(G_HIT_BOSS);
				}
				else if (A->getType() == G_DECORATION)
				{
					setCrash(true);
				}
			}
		break;
		case G_DEAD_BOSS:
		break;
		case G_ROCKET:
			if (d > EPSILON && d <= r)
			{
				if (A->getType() == G_BOSS)
				{
					setType(G_HIT);
				}
				else if (A->getType() == G_HIT_BOSS)
				{
					setType(G_HIT);
				}
				else if (A->getType() == G_DEAD_BOSS)
				{
					setType(G_HIT_EXPLOSION);
				}
				else if (A->getType() == G_DECORATION)
				{
					setType(G_HIT_EXPLOSION);
				}
			}
		break;
		case G_HIT:
		break;
		case G_HIT_BOSS:
		break;
		case G_HIT_EXPLOSION:
		break;
		case G_MAP:
		break;
		case G_BACKGROUND:
		break;
		case G_DECORATION:
		break;
		case G_SPECIAL:
		break;
		case G_GARBAGE:
		break;
	}
	return getCrash();
}

// pythagorean distance
float Object::distance(const Vector& vec)
{
	return (vec - m_position).magnitude();
}

void Object::setHeight(const float& h)
{
	m_position[1] = h + getRadius() + 2.5f;
}

void Object::setOBJECTcolor(const float& r, const float& g, const float& b)
{
	this->r = r;
	this->g = g;
	this->b = b;
}

void Object::setPosition(const Vector& vec)
{
	m_position = vec;
}

void Object::setPosition(const float& x, const float& y, const float& z)
{
	setPosition(Vector(x, y, z));
}

