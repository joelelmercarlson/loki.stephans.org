#ifndef HUD_H
#define HUD_H

#include "Object.h"
#include "GL/glx.h"

class HUD : public Object
{
private:
	GLuint base;
	char message[255];
	void clearFont();
	void buildFont();
public:
	HUD(const std::string& filename);
	~HUD();
	
	void glPrint(const char *str,...);

	void setHUDcolor(const float& r, const float& g, const float& b);
	void setPos2D(const float& x, const float& y);
	void setTEXTcolor(const float& r, const float& g, const float& b);

	void Show();

	float hudColor[4];
	float textColor[4];

	void updateState(const char *str);	

protected:
	Vector mPosition;
};
#endif
