#include "HUD.h"

HUD::HUD(const std::string& filename) :
	Object(filename, 0.0f, 0.0f, 0.0f)
{
	hudColor[0] = 1.0f;
	hudColor[1] = 0.0f;
	hudColor[2] = 0.0f;
	hudColor[3] = 1.0f;

	textColor[0] = 0.0f;
	textColor[1] = 0.0f;
	textColor[2] = 0.0f;
	textColor[3] = 1.0f;

	mPosition[0] = 10.0f;
	mPosition[1] = (float)SCREEN_HEIGHT - 15.0f;
	mPosition[2] = 0.0f;

	buildFont();
}

HUD::~HUD()
{
	clearFont();
}

void HUD::clearFont()
{
	glDeleteLists(base, 96);
}

void HUD::buildFont()
{
	Display *dpy;          /* Our display */
	XFontStruct *fontInfo; /* Our font info */

	base = glGenLists( 96 );	

	dpy = XOpenDisplay( NULL );

	/* Get the font information */
	fontInfo = XLoadQueryFont( dpy, "-urw-nimbus sans l-medium-r-normal--18-*-*-*-p-*-iso8859-1" );
	if (fontInfo == NULL)
	{
		fontInfo = XLoadQueryFont( dpy, "fixed" );
		if (fontInfo == NULL)
		{
			std::cout << "no X font available" << std::endl;
			exit(1);
		}
	}

	glXUseXFont( fontInfo->fid, 32, 96, base );
	XFreeFont( dpy, fontInfo );
	XCloseDisplay( dpy );
}

void HUD::glPrint(const char *str,...)
{
	char text[255];
	va_list ap;

	if (str == NULL) return;

	va_start(ap, str);
		vsprintf(text, str, ap);
	va_end(ap);

	glPushMatrix();

	/* position text on screen */
	glRasterPos2f(mPosition[0],mPosition[1]);

	/* Pushes the Display List Bits */
	glPushAttrib( GL_LIST_BIT );

	/* Sets base character to 32 */
	glListBase( base - 32 );

	/* Draws the text */
	glCallLists( strlen( text ), GL_UNSIGNED_BYTE, text );

	/* Pops the Display List Bits */
	glPopAttrib( );

	glPopMatrix();

}

void HUD::setHUDcolor(const float &r, const float &g, const float &b)
{
	hudColor[0] = r;
	hudColor[1] = g;
	hudColor[2] = b;
	hudColor[3] = 1.0f;
}

void HUD::setPos2D(const float &x, const float &y)
{
	Vector temp(x, y, 0.0f);
	mPosition = temp;
}

void HUD::setTEXTcolor(const float &r, const float &g, const float &b)
{
	textColor[0] = r;
	textColor[1] = g;
	textColor[2] = b;
	textColor[3] = 1.0f;
}


void HUD::updateState(const char *str)
{
	if (str == NULL) return;

	strcpy(message, str);
}

void HUD::Show()
{
	float cross_w = 32.0f;
	float cross_h = 32.0f;
	float cross_x = (SCREEN_WIDTH  - cross_w)/2.0f;
	float cross_y = (SCREEN_HEIGHT - cross_h)/2.0f;

	glDisable(GL_FOG);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//display hud message
	glColor4f(textColor[0],textColor[1],textColor[2],textColor[3]);
	glPrint(message);

	//display crosshair
	glColor4f(hudColor[0],hudColor[1],hudColor[2],hudColor[3]);
	glTranslatef(cross_x,0.0f,0.0f);
	glTranslatef(0.0f,cross_y,0.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, getTexture());
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f,0.0f);glVertex2f(0.0f, 0.0f);
	glTexCoord2f(1.0f,0.0f);glVertex2f(cross_w, 0.0f);
	glTexCoord2f(1.0f,1.0f);glVertex2f(cross_w, cross_h);
	glTexCoord2f(0.0f,1.0f);glVertex2f(0.0f, cross_h);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_FOG);
}
