#include "App.h"
#include "Engine.h"

Input input;

// Engine code handles input and passes to App
class CMyEngine: public CEngine
{
public:
	void AdditionalInit ();
	void Think	    ( const int& iElapsedTime );
	void Render	    ();
 
	void KeyUp  	    (const int& iKeyEnum);
	void KeyDown	    (const int& iKeyEnum);
 
	void MouseMoved     (const int& iButton, 
			     const int& iX, 
			     const int& iY, 
			     const int& iRelX, 
		             const int& iRelY);
 
	void MouseButtonUp  (const int& iButton, 
			     const int& iX, 
			     const int& iY, 
			     const int& iRelX, 
		             const int& iRelY);
 
	void MouseButtonDown(const int& iButton, 
			     const int& iX, 
			     const int& iY, 
			     const int& iRelX, 
		             const int& iRelY);
 
	void WindowInactive();
	void WindowActive();

	void End();
};
 
 
int main(int argc, char* argv[]) 
{
        CMyEngine Engine;
 
	Engine.SetTitle( "gl_game: Init()" );
	Engine.Init();
 
	Engine.SetTitle( "gl_game: Running" );
	Engine.Start();
 
	Engine.SetTitle( "gl_game: End()" );
	Engine.End();

	return 0;
}
 
void CMyEngine::AdditionalInit()
{
	// SDL Setup
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_GrabInput(SDL_GRAB_ON);
	SDL_WarpMouse(0,0);

	// Game Setup
	APP.Startup();
}
 
void CMyEngine::Think( const int& iElapsedTime )
{
	// store current fps
	input.fps = GetFPS();

	// Do time-based calculations
	if (iElapsedTime < 1000 / FRAMES_PER_SECOND)
		SDL_Delay((1000/FRAMES_PER_SECOND) - iElapsedTime); 
}
 
void CMyEngine::Render()
{
	// Display slick graphics on screen
	APP.Animate(input);
}
 
void CMyEngine::KeyDown(const int& iKeyEnum)
{        
	switch (iKeyEnum)
	{
	case SDLK_LEFT:
		input.heading = input.rotate;
		break;
	case SDLK_RIGHT:
		input.heading = -input.rotate;
		break;
	case SDLK_UP:
		input.vel = DT;
		break;
	case SDLK_DOWN:
		input.vel = -DT;
		break;
	case SDLK_SPACE:
		input.crash = true;
		break;
	case SDLK_PLUS:
	case SDLK_EQUALS:
		input.dt_mouse += 0.1f;
		if (input.dt_mouse > input.dt_mouseMax) input.dt_mouse = input.dt_mouseMax;
		break;
	case SDLK_MINUS:
		input.dt_mouse -= 0.1f;
		if (input.dt_mouse < 0.05f) input.dt_mouse = 0.05f;
		break;
	case SDLK_a:
		input.strafe = -DT;
		break;
	case SDLK_d:
		input.debug = (!input.debug);
		break;
	case SDLK_f:
		input.first = (!input.first);
		break;
	case SDLK_m:
		if (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON)
			SDL_WM_GrabInput(SDL_GRAB_OFF);
		else
			SDL_WM_GrabInput(SDL_GRAB_ON);
		break;
	case SDLK_q:
		End();
		break;
	case SDLK_s:
		input.strafe = DT;
		break;
	}
}
 
 
void CMyEngine::KeyUp(const int& iKeyEnum)
{
	switch (iKeyEnum)
	{
	case SDLK_LEFT:
		break;
	case SDLK_RIGHT:
		break;
	case SDLK_UP:
		break;
	case SDLK_DOWN:
		break;
	}
}
 
void CMyEngine::MouseMoved(const int& iButton, 
			   const int& iX, 
			   const int& iY, 
			   const int& iRelX, 
			   const int& iRelY)
{
	// Handle mouse movement
	input.heading = -(iRelX * input.dt_mouse); 
	input.pitch   = -(iRelY * input.dt_mouse);
	input.mouse_x = iX;
	input.mouse_y = iY;
}
 
void CMyEngine::MouseButtonUp(const int& iButton, 
			      const int& iX, 
			      const int& iY, 
			      const int& iRelX, 
			      const int& iRelY)
{
	// Handle mouse button released
	input.click = false; 

	// wheel mouse zooms
	if (iButton == 4) input.zoom++;
	else if (iButton == 5) input.zoom--;
	else input.mouse_b = iButton;

	input.zoom = (input.zoom > input.zoomMax) ? input.zoomMax : input.zoom;
	input.zoom = (input.zoom < 0) ? 0 : input.zoom;
}
 
void CMyEngine::MouseButtonDown(const int& iButton, 
				const int& iX, 
				const int& iY, 
				const int& iRelX, 
				const int& iRelY)
{
	// Handle mouse button pressed
	input.click = true;
}
 
void CMyEngine::WindowInactive()
{
	// Pause game
}
 
void CMyEngine::WindowActive()
{
	// Un-pause game
}
 
void CMyEngine::End()
{
	// Clean up
	APP.Shutdown();
	exit(0);
}
