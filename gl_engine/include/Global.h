#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <cmath>
#include <string>
#include "Input.h"
#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"

#define TEST_ELSE_ZERO(a,b) (a<b) ? a : 0
#define DEG2RAD(n) ((float)n/180.0f) * PI
#define FRAND (((float)rand()-(float)rand())/RAND_MAX)

#define EPSILON 0.0001f 
#define PI 3.141592653589792f
#define DT 0.5f

// world
const float WORLD_VIEW    = 200.0f; // perspective
const float WORLD_EDGE    = 2048.0f;
const float WORLD_FLOOR   = 1.0f;
const float WORLD_CEILING = 250.0f;
const float WORLD_HELL    = -1.0f;

// grid (1,1) is near left corner
#define WORLD_U(n) (int(n + WORLD_EDGE) / int(WORLD_VIEW)) - 10
#define WORLD_V(n) (int(n + WORLD_EDGE) / int(WORLD_VIEW)) - 10

// display
const int    SCREEN_WIDTH       = 640;
const int    SCREEN_HEIGHT      = 480;
const int    SCREEN_BPP         = 32;
const float  SCREEN_PERSPECTIVE = 45.0f;
const int    FRAMES_PER_SECOND  = 60;

// types
enum GAME_TYPE {
	G_PLAYER,
	G_BOSS,
	G_DEAD_BOSS,
	G_ROCKET,
	G_HIT,
	G_HIT_BOSS,
	G_HIT_EXPLOSION,
	G_MAP,
	G_BACKGROUND,
	G_DECORATION,
	G_SPECIAL,
	G_GARBAGE
};
#endif
