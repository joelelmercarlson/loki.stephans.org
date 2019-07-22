#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Object.h"

class Background : public Object
{
private:
	GLuint* skybox;
	void setSkyBox();
public:
	Background(const std::string& filename);
	~Background();

	void Show();
protected:
	Vector mPosition;

	void updateState(const Quaternion& q, const Vector& vec, const float& time);
};
#endif
