#ifndef TERRAIN_H
#define TERRAIN_H

#include "Object.h"

class Terrain : public Object
{
private:
	int width;
	float size;
	float terrainMul;
	float heightMul;
	float scanDepth;
	float textureMul;

	float RangedRandom(float v1,float v2);
	void NormalizeTerrain(float field[],int size);
	void FilterHeightBand(float *band,int stride,int count,float filter);
	void FilterHeightField(float field[],int size,float filter);
	void MakeTerrainPlasma(float field[],int size,float rough);
public:
	Terrain(const std::string& filename, const int& w, const float& rFactor);
	~Terrain() { delete [] heightMap; };

	float *heightMap;
	float fogColor[4];

	void BuildTerrain(int w, float r);
	float getHeight(const Vector& vec);
	float getSize() { return size; };
	void Show();
protected:
	Quaternion mOrientation;
	Vector mPosition;
	void updateState(const Quaternion& q, const Vector& vec, const float& time);
};
#endif
