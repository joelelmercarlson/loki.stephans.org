#ifndef INPUT_H
#define INPUT_H

class Input
{
public:
	Input() :
		vel(0.0f),
		strafe(0.0f),
		pitch(0.0f),
		heading(0.0f),
		rotate(5.0f),
		mouse_x(0),mouse_y(0),mouse_b(1),
		dt_mouse(0.15f),
		dt_mouseMax(1.0f),
		left(1.0f),
		right(1000.0f),
		near(1.0f),
		far(1000.0f),
		floor(1.0f),
		ceiling(1000.0f),
		fps(0),
		click(false),
		first(false),
		crash(false),
		debug(false),
		zoom(0),
		zoomMax(3) { };
	~Input() {};

	float vel, strafe;                 // velocity on z and x 
	float pitch, heading;              // rotations on y, z axis
	float rotate;                      // rate of heading change
	int mouse_x, mouse_y, mouse_b;     // mouse inputs
	float dt_mouse, dt_mouseMax;       // mouse sensitivity
	float left, right, near, far, floor, ceiling; // input contstraints
	int fps;                           // FramesPerSecond
	bool click;                        // mouse click
	bool first;                        // fixed-y?
	bool crash;                        // crash?
	bool debug;                        // camera debug
	int  zoom, zoomMax;                // zoom mode
};
#endif
