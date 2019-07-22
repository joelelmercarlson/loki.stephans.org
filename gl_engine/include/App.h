#ifndef APP_H
#define APP_H

#include "Background.h"
#include "Boss.h"
#include "Camera.h"
#include "Global.h"
#include "HUD.h"
#include "Rocket.h"
#include "Rocket2.h"
#include "Terrain.h"
#include "Scenery.h"

#define APP AppManager::Instance()

class AppManager
{
private:
	int boss_cnt;
	int counter;
	Camera* player;
	HUD* hud;
	Background* skybox;
	Terrain* terrain;
	std::vector <Object*> Scene;
	int zoom; // zoom mode
	int zoomLast; // previous zoom mode
public:
	~AppManager();

	static AppManager& Instance()
	{
		static AppManager instance;
		return instance;
	}

	void Attach(Object* obj);
	void Animate(Input &input);
	void Crash();
	void LifeCycle();
	void Show();
	void Shutdown();
	void Startup();

	bool GameOver;
protected:
	AppManager();
};
#endif
