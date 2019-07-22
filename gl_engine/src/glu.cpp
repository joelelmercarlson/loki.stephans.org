#include "SDL/SDL_opengl.h"
#include "Vector.h"

using namespace std;

/* from opengl libglu */

void gluPerspective(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
	GLdouble fH = tan( fovY / 360.0f * 3.141592653589792 ) * zNear;
	GLdouble fW = fH * aspect;
	glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

void gluLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, 
        GLdouble centerx, GLdouble centery, GLdouble centerz,
        GLdouble upx, GLdouble upy, GLdouble upz)
{
        Vector forward(centerx - eyex, centery - eyey, centerz - eyez);
	Vector side;
	Vector up(upx,upy,upz);
        GLfloat m[16] = {
		1.0, 0.0, 0.0, 0.0, 
		0.0, 1.0, 0.0, 0.0, 
		0.0, 0.0, 1.0, 0.0, 
		0.0, 0.0, 0.0, 1.0 };

	// normalize vector
	forward = forward.normalize();

	// side = forward x up
        side = forward.cross(up).normalize();

	// up = side x forward
        up = side.cross(forward);

/*
OpenGL translation 4x4 matrix
	x, y, z,  translation
        0, 4, 8,  12
        1, 5, 9,  13
        2, 6, 10, 14
        3, 7, 11, 15
*/
        m[0] = side[0];
        m[1] = side[1];
        m[2] = side[2];

        m[4] = up[0];
        m[5] = up[1];
        m[6] = up[2];

        m[8] = -forward[0];
        m[9] = -forward[1];
        m[10] = -forward[2];

	glMultMatrixf(m);
        glTranslated(-eyex, -eyey, -eyez);
}

void gluOrtho2D(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
{
    glOrtho(left, right, bottom, top, -1, 1);
}
