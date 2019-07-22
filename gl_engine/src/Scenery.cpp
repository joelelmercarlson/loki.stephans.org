#include "Scenery.h"

Scenery::Scenery(const std::string& filename, const Quaternion& q, const Vector& vec):
	Object(filename, vec[0], vec[1], vec[2])
{
	mPosition = vec;
	mOrientation = q;

	setRadius(2.0f);
	setType(G_DECORATION);
} 

void Scenery::Show()
{
	float size = getRadius() * 4.0f;

	if (!getVisible()) return;

	glEnable(GL_FOG);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, getTexture());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// translate and scale
	glTranslatef(mPosition[0],mPosition[1],mPosition[2]);
	glScalef(size,size,size);

	// 3D pyramid
	glBegin(GL_TRIANGLES);
	//top
	glTexCoord2f(0.0f,0.0f);glVertex3f(0.0f,1.0f,0.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f(-1.0f,-1.0f,1.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f(1.0f,-1.0f,1.0f);
	//right
	glTexCoord2f(1.0f,1.0f);glVertex3f(0.0f,1.0f,0.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f(1.0f,-1.0f,1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f(1.0f,-1.0f,-1.0f);
	//back
	glTexCoord2f(1.0f,0.0f);glVertex3f(0.0f,1.0f,0.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f(1.0f,-1.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
	//left
	glTexCoord2f(0.0f,1.0f);glVertex3f(0.0f,1.0f,0.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
	glTexCoord2f(1.0f,1.0f);glVertex3f(-1.0f,-1.0f,1.0f);
	//bottom
	glTexCoord2f(1.0f,1.0f);glVertex3f(-1.0f,-1.0f,-1.0f);
	glTexCoord2f(0.0f,1.0f);glVertex3f( 1.0f,-1.0f,-1.0f);
	glTexCoord2f(0.0f,0.0f);glVertex3f( 1.0f,-1.0f, 1.0f);
	glTexCoord2f(1.0f,0.0f);glVertex3f(-1.0f,-1.0f, 1.0f);
	glEnd();

	glDisable(GL_TEXTURE_2D); // switch back to colors
	glDisable(GL_FOG);
}

void Scenery::updateState(const Quaternion& q, const Vector& vec, const float& time)
{
}
