#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "SDL/SDL_opengl.h"
#include <cmath>

class Frustum
{
private:
	float frustum[6][4];
public:
	Frustum();
	~Frustum();

	void ExtractFrustum();
	bool PointInFrustum(float x, float y, float z);
	bool SphereInFrustum(float x, float y, float z, float radius);
	float f_SphereInFrustum(float x, float y, float z, float radius);
};
#endif
