#include "Background.h"

Background::Background(const std::string& filename):
	Object(filename, 0.0f, 0.0f, 0.0f)
{
	skybox = new GLuint[6];

	setSkyBox();
	setType(G_BACKGROUND);
}

Background::~Background()
{
	delete skybox;
}

// TBD set six unique pictures
void Background::setSkyBox()
{
	SDL_Surface* image;
	GLuint bottom, side, top;
	GLfloat texcoord[4];

	top = getTexture();

	image = SDL_LoadImage("media/ground.bmp");
	bottom = SDL_GL_LoadTexture(image,texcoord);
	SDL_FreeSurface(image);

	image = SDL_LoadImage("media/side.bmp");
	side = SDL_GL_LoadTexture(image,texcoord);
	SDL_FreeSurface(image);

	skybox[0] = side;
	skybox[1] = side;
	skybox[2] = side;
	skybox[3] = side;
	skybox[4] = top;
	skybox[5] = bottom;
}

void Background::Show()
{
	// Store the current matrix
	glPushMatrix();

	// Reset and transform the matrix.
	glLoadIdentity();
	
	// gluLookAt
	// eye position
	// point we look at                   
	// up vector
	gluLookAt(0.0f,0.0f,0.0f,
		mPosition[0],mPosition[1],mPosition[2],
		0.0f,1.0f,0.0f);
	
	// Enable/Disable features
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	// Just in case we set all vertices to white.
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	// Render the front quad
	glBindTexture(GL_TEXTURE_2D, skybox[0]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(  1.5f, -1.5f, -1.5f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f( -1.5f, -1.5f, -1.5f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f( -1.5f,  1.5f, -1.5f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f(  1.5f,  1.5f, -1.5f );
	glEnd();

	// Render the left quad
	glBindTexture(GL_TEXTURE_2D, skybox[1]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(  1.5f, -1.5f,  1.5f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f(  1.5f, -1.5f, -1.5f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f(  1.5f,  1.5f, -1.5f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f(  1.5f,  1.5f,  1.5f );
	glEnd();

	// Render the back quad
	glBindTexture(GL_TEXTURE_2D, skybox[2]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.5f, -1.5f,  1.5f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f(  1.5f, -1.5f,  1.5f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f(  1.5f,  1.5f,  1.5f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f( -1.5f,  1.5f,  1.5f );
	glEnd();

	// Render the right quad
	glBindTexture(GL_TEXTURE_2D, skybox[3]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.5f, -1.5f, -1.5f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f( -1.5f, -1.5f,  1.5f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f( -1.5f,  1.5f,  1.5f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f( -1.5f,  1.5f, -1.5f );
	glEnd();

	// Render the top quad
	glBindTexture(GL_TEXTURE_2D, skybox[4]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( -1.5f,  1.5f, -1.5f );
		glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.5f,  1.5f,  1.5f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f(  1.5f,  1.5f,  1.5f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f(  1.5f,  1.5f, -1.5f );
	glEnd();

	// Render the bottom quad
	glBindTexture(GL_TEXTURE_2D, skybox[5]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( -1.5f, -1.5f, -1.5f );
		glTexCoord2f(0.0f, 1.0f); glVertex3f( -1.5f, -1.5f,  1.5f );
		glTexCoord2f(1.0f, 1.0f); glVertex3f(  1.5f, -1.5f,  1.5f );
		glTexCoord2f(1.0f, 0.0f); glVertex3f(  1.5f, -1.5f, -1.5f );
	glEnd();

	// Restore enable bits and matrix
	glPopAttrib();
	glPopMatrix();
}

void Background::updateState(const Quaternion& q, const Vector& vec, const float& time)
{
	mPosition = vec;
}
