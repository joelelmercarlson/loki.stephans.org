#include "Terrain.h"

Terrain::Terrain(const std::string& filename, const int& w, const float& rFactor):
	Object(filename, 0.0f, 0.0f, 0.0f)
{
	width      = w;
	scanDepth  = WORLD_VIEW;
	terrainMul = 50.0f;
	textureMul = 0.25f;
	heightMul  = WORLD_CEILING - 25.0f;
	size  = std::sqrt(width * terrainMul * width * terrainMul + width * terrainMul * width * terrainMul);

	fogColor[0] = 0.82f;
	fogColor[1] = 0.75f;
	fogColor[2] = 0.9f;
	fogColor[3] = 1.0f;

	heightMap = NULL;

	setRadius(1.0f);
	setType(G_MAP);

	BuildTerrain(width, rFactor);
}

void Terrain::BuildTerrain(int w, float rFactor)
{
	width = w;
	heightMap = new float[width * width]; 
	MakeTerrainPlasma(heightMap, width, rFactor);
}

void Terrain::Show()
{
	int x, z;

	// OpenGL Fog
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogfv(GL_FOG_COLOR,fogColor);
	glFogf(GL_FOG_DENSITY,0.543f);
	glFogf(GL_FOG_START,scanDepth * 0.02f);
	glFogf(GL_FOG_END,scanDepth * 2.5f);
	glHint(GL_FOG_HINT,GL_FASTEST);
	glEnable(GL_FOG);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER,0.0f);
	glDisable(GL_ALPHA_TEST);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,getTexture());
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glColor3f(1.0f,1.0f,1.0f);
	for (z = int(mPosition[2] / terrainMul - scanDepth), z = z<0?0:z; (z < mPosition[2] / terrainMul + scanDepth) && z < width-1; z++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (x = int(mPosition[0] / terrainMul - scanDepth), x = x<0?0:x; (x < mPosition[0] / terrainMul + scanDepth) && x < width-1; x++)
		{
			glTexCoord2f(textureMul * x, textureMul * z);
			glVertex3f((float)x*terrainMul, heightMap[x + z*width]*heightMul, (float)z*terrainMul);

			glTexCoord2f(textureMul * x, textureMul * (z+1));
			glVertex3f((float)x*terrainMul, heightMap[x + (z+1)*width]*heightMul, (float)(z+1)*terrainMul);

			glTexCoord2f(textureMul * (x+1), textureMul * z);
			glVertex3f((float)(x+1)*terrainMul, heightMap[(x+1) + z*width]*heightMul, (float)z*terrainMul);

			glTexCoord2f(textureMul * (x+1), textureMul * (z+1));
			glVertex3f((float)(x+1)*terrainMul, heightMap[(x+1) + (z+1)*width]*heightMul, (float)(z+1)*terrainMul);
		}
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
}

// updateState uses mPosition for frustum clipping in Show()
void Terrain::updateState(const Quaternion& q, const Vector& vec, const float& time)
{
	mOrientation = q;
	mPosition = vec;
} 

float Terrain::getHeight(const Vector& vec)
{
	float projCameraX, projCameraZ;
	int col0, col1, row0, row1;
	float h00, h01, h10, h11;

	// divide by the grid-spacing if it is not 1
	projCameraX = (float)(vec[0] / terrainMul);
	projCameraZ = (float)(vec[2] / terrainMul);

	// compute the height field coordinates (Col0, Row0)
	// and (Col1, Row1) that identify the height field cell 
	// directly below the camera.
	col0 = int(projCameraX);
	row0 = int(projCameraZ);
	col1 = col0 + 1;
	row1 = row0 + 1;

	// make sure that the cell coordinates don't fall
	// outside the height field.
	if (col1 > width)
		col1 = 0;
	if (row1 > width)
		row1 = 0;

	// get the four corner heights of the cell from the height field
	h00 = heightMul * (float)heightMap[col0 + row0*width];
	h01 = heightMul * (float)heightMap[col1 + row0*width];
	h11 = heightMul * (float)heightMap[col1 + row1*width];
	h10 = heightMul * (float)heightMap[col0 + row1*width];

	// calculate the position of the camera relative to the cell.
	// note, that 0 <= tx, ty <= 1.
	float tx = projCameraX - float(col0);
	float ty = projCameraZ - float(row0);

	// the next step is to perform a bilinear interpolation
	// to compute the height of the terrain directly below
	// the object.
	float txty = tx * ty;
	float final_height = h00 * (1.0f - ty - tx + txty)
					+ h01 * (tx - txty)
					+ h11 * txty
					+ h10 * (ty - txty);
	return final_height;
}

// RangedRandom()
// Returns a random number between v1 and v2
float Terrain::RangedRandom(float v1,float v2)
{
	return v1 + (v2-v1)*((float)rand())/((float)RAND_MAX);
}
//
// NormalizeTerrain()
// Given a height field, normalize it so that the minimum altitude
// is 0.0 and the maximum altitude is 1.0
void Terrain::NormalizeTerrain(float field[],int size)
{
	float maxVal,minVal,dh;
	/* Find the maximum and minimum values in the height field */
	maxVal = field[0];
	minVal = field[0];

	for (int i=1;i<size*size;i++)
	{
		if (field[i] > maxVal)
		{
			maxVal = field[i];
		}
		else if (field[i] < minVal)
		{
			minVal = field[i];
	}
	}
	/* Find the altitude range (dh) */
	if (maxVal <= minVal) return;
	dh = maxVal-minVal;
	/* Scale all the values so they are in the range 0-1 */
	for (int i=0;i<size*size;i++)
	{
		field[i] = (field[i]-minVal)/dh;
	}
}

// FilterHeightBand()
// Erosion filter -
// FilterHeightBand applies a FIR filter across a row or column of the
// height field
void Terrain::FilterHeightBand(float *band,int stride,int count,float filter)
{
	int i,j=stride;
	float v = band[0];
	for (i=0;i<count-1;i++)
	{
		band[j] = filter*v + (1-filter)*band[j];
		v = band[j];
		j+=stride;
	}
}

// FilterHeightField()
// Erosion filter -
// Erodes a terrain in all 4 directions
void Terrain::FilterHeightField(float field[],int size,float filter)
{
	// Erode rows left to right
	for (int i=0;i<size;i++)
	{
		FilterHeightBand(&field[size*i],1,size,filter);
	}

	// Erode rows right to left
	for (int i=0;i<size;i++)
	{
		FilterHeightBand(&field[size*i+size-1],-1,size,filter);
	}

	// Erode columns top to bottom
	for (int i=0;i<size;i++)
	{
		FilterHeightBand(&field[i],size,size,filter);
	}

	// Erode columns bottom to top
	for (int i=0;i<size;i++)
	{
		FilterHeightBand(&field[size*(size-1)+i],-size,size,filter);
	}
}

// MakeTerrainPlasma()
// desc: Genereate terrain using diamond-square (plasma) algorithm
void Terrain::MakeTerrainPlasma(float field[],int size,float rough)
{
	int i,j,ni,nj,mi,mj,pmi,pmj,rectSize = size;
	float dh = (float)rectSize/2,r = (float)std::pow(2,-1*rough);

// Since the terrain wraps, all 4 "corners" are represented by the 
// value at 0,0, so seeding the heightfield is very straightforward
// Note that it doesn't matter what we use for a seed value, since 
// we're going to renormalize the terrain after we're done
	field[0] = 0;

	while(rectSize > 0)
	{
/*
Diamond step - Find the values at the center of the retangles by
averaging the values at the corners and adding a random offset:

                a.....b
                .     .
                .  e  .
                .     .
                c.....d

                e  = (a+b+c+d)/4 + random

                In the code below:
                a = (i,j)
                b = (ni,j)
                c = (i,nj)
                d = (ni,nj)
                e = (mi,mj)
*/

		for (i=0;i<size;i+=rectSize)
		for (j=0;j<size;j+=rectSize)
		{
			ni = (i+rectSize)%size;
			nj = (j+rectSize)%size;
			mi = (i+rectSize/2);
			mj = (j+rectSize/2);
			field[mi+mj*size] = (field[i+j*size] + field[ni+j*size] + field[i+nj*size] + field[ni+nj*size])/4 + RangedRandom(-dh/2,dh/2);
		}

/*
Square step - Find the values on the left and top sides of each rectangle
The right and bottom sides are the left and top sides of the neighboring
rectangles, so we don't need to calculate them The height field wraps,
so we're never left hanging.  The right side of the last rectangle in a
row is the left side of the first rectangle in the row.  The bottom side
of the last rectangle in a column is the top side of the first rectangle
in the column

                      .......
                      .     .
                      .     .
                      .  d  .
                      .     .
                      .     .
                ......a..g..b
                .     .     .
                .     .     .
                .  e  h  f  .
                .     .     .
                .     .     .
                ......c......
                g = (d+f+a+b)/4 + random
                h = (a+c+e+f)/4 + random

                In the code below:
                        a = (i,j)
                        b = (ni,j)
                        c = (i,nj)
                        d = (mi,pmj)
                        e = (pmi,mj)
                        f = (mi,mj)
                        g = (mi,j)
                        h = (i,mj)
*/

		for (i=0;i<size;i+=rectSize)
		for (j=0;j<size;j+=rectSize)
		{
			ni = (i+rectSize)%size;
			nj = (j+rectSize)%size;
			mi = (i+rectSize/2);
			mj = (j+rectSize/2);
			pmi = (i-rectSize/2+size)%size;
			pmj = (j-rectSize/2+size)%size;

// Calculate the square value for the top side of the rectangle
			field[mi+j*size] = (field[i+j*size] + field[ni+j*size] + field[mi+pmj*size] + field[mi+mj*size])/4 + RangedRandom(-dh/2,dh/2);

// Calculate the square value for the left side of the rectangle
			field[i+mj*size] = (field[i+j*size] + field[i+nj*size] + field[pmi+mj*size] + field[mi+mj*size])/4 + RangedRandom(-dh/2,dh/2);
		}

// Setup values for next iteration
// At this point, the height field has valid values at each of 
// the coordinates that fall on a rectSize/2 boundary
		rectSize /= 2;
		dh *= r;
	}
//
// Normalize terrain so minimum value is 0 and maximum value is 1
	NormalizeTerrain(field,size);
}
