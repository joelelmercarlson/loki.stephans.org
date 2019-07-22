#ifndef SDL_LOADIMAGE_H
#define SDL_LOADIMAGE_H

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include <string>
#include <iostream>

// convert SDL surfaces to GL textures
int power_of_two(int input);
SDL_Surface* SDL_LoadImage(const std::string& filename);
GLuint SDL_GL_LoadTexture(SDL_Surface *surface, GLfloat *texcoord);

#endif // SDL_LOADIMAGE_H
