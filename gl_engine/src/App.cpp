#include "App.h"

AppManager::AppManager() :
	boss_cnt(0),
	counter(0),
	player(NULL),
	hud(NULL),
	skybox(NULL),
	terrain(NULL),
	zoom(-1),
	zoomLast(-1),
	GameOver(false)
{
}

AppManager::~AppManager()
{
	delete player;
	delete hud;
	delete skybox;
	delete terrain;
}

void AppManager::Animate(Input &input)
{
	// scene
	counter++;

	// zoom
	zoomLast = zoom;
	zoom = input.zoom;

	// camera constraints
	input.left    = WORLD_VIEW;
	input.right   = WORLD_EDGE - WORLD_VIEW;
	input.near    = WORLD_VIEW;
	input.far     = WORLD_EDGE - WORLD_VIEW;
	input.floor   = terrain->getHeight(player->getPosition()) + player->getRadius() + 1.5f;
	input.ceiling = WORLD_CEILING;
	
	// move player
	player->Animate(input);

	// fire weapon
	if (input.click)
	{
		if (input.mouse_b == 1)
		{
			Rocket *rocket = new Rocket("media/explosion.bmp", player->getOrientation(), player->getPosition());
			Attach(rocket);
		}
		else if (input.mouse_b == 3)
		{
			Rocket2 *rocket = new Rocket2("media/explosion.bmp", player->getOrientation(), player->getPosition());
			Attach(rocket);
		}
	}	

	LifeCycle();
	Show();
}

void AppManager::Attach(Object *obj)
{
	if (obj != NULL) Scene.push_back(obj);
}

void AppManager::Crash()
{
	// collision detection
	for (int i = 0; i < (int)Scene.size(); i++)
	{
		Vector v = Scene[i]->getPosition();
		GAME_TYPE t = Scene[i]->getType();
		float height = terrain->getHeight(v);

		// terrain collision first
		if (t == G_BOSS)
		{
			Scene[i]->setHeight(height);
		}
		else if ((t == G_ROCKET) && (v[1] < height))
		{
			Scene[i]->setType(G_HIT); 
		}

		// the moveable world objects
		for (int j = 0; j < (int)Scene.size(); j++)
		{
			if (t == G_BOSS || t == G_PLAYER || t == G_ROCKET)
			{
				Scene[i]->checkCollision(Scene[j]);
			}
		}
	}

	// check for the player
	for (int i = 0; i < (int)Scene.size(); i++)
	{
		player->checkCollision(Scene[i]);
	}
}

void AppManager::LifeCycle()
{
	int bosses = 0;
	if (counter > 2 * FRAMES_PER_SECOND)
	{
		counter = 0;
		for(int i = 0; i < (int)Scene.size(); i++)
		{
			if (Scene[i]->getType() == G_BOSS) bosses++;
			if (Scene[i]->getType() == G_GARBAGE)
				Scene.erase(Scene.begin() + i);
		}
		if (bosses < 1) GameOver = true;
		boss_cnt = bosses;
	}
}

void AppManager::Show()
{
	Vector v;
	Quaternion q;
	char status[255];
	
	// clear screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (zoom > 0)
		player->setFOVy(45.0f/zoom); // 2x - max_zoom
	else
		player->setFOVy(45.0f);

	// gluPerspective
	if (zoomLast != zoom)
		player->setCameraPerspective();
	
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();

	// move our player
	player->setPerspective();

	// where is our player?
	v = player->getPosition();
	q = player->getOrientation();

	// optimize the scene by frustum testing
	for (int i = 0; i < (int)Scene.size(); i++)
	{
		if(player->getView(Scene[i]->getPosition(), WORLD_VIEW))
		{
			Scene[i]->setVisible(true);
		}
		else
		{
			Scene[i]->setVisible(false);
		}
		Scene[i]->updateState(q, v, 0.1f);
	}

	// collision detection on all objects in scene
	Crash();

	// call Show method in all stored objects
	for (int i = 0; i < (int)Scene.size(); i++)
	{
		glPushMatrix();
		Scene[i]->Show();	
		glPopMatrix();
	}

	// set game status
	if (GameOver)
	{
		sprintf(status, "Game Over! press ESC to quit!");
		hud->setTEXTcolor(1.0f,0.0f,0.0f);
	}
	else if (zoom)
	{
		sprintf(status, "zoom %dx. Press h for help.", zoom);
		hud->setTEXTcolor(0.0f,0.0f,1.0f);
	}
	else
	{
		sprintf(status, "(%d,%d) boss = %d", WORLD_U(v[0]), WORLD_V(v[2]), boss_cnt);
		hud->setTEXTcolor(0.0f,0.0f,0.0f);
	}
	hud->updateState(status);

	// hud
	hud->Show();
	glPopMatrix();
}


void AppManager::Startup()
{
	// game data
	srand(time(NULL));

	// player camera
	player = new Camera();

	// HUD display last
	hud = new HUD("media/cross.bmp");

	// Skybox
	skybox = new Background("media/sky.bmp");
	Attach(skybox);

	// Terrain
	terrain = new Terrain("media/texture.bmp", 64, 0.5f);
	Attach(terrain);

	// Map random-world with bosses and bumps
	int bosses = 0;
	int maxBosses = 8;
	for (int i=WORLD_VIEW; i < terrain->getSize(); i += WORLD_VIEW)
	{
		for (int j=WORLD_VIEW; j < terrain->getSize(); j += WORLD_VIEW)
		{
			if ((rand() % 10 > 8) && (bosses < maxBosses))
			{
				Boss* b = new Boss("media/rock.bmp", Quaternion::IDENTITY, Vector(i, 0.0f, j));
				Attach(b);
				bosses++;
			}
		}
	}

	int bumps = 0;
	int maxBumps = 200;
	for (int i=2 * WORLD_VIEW; i < terrain->getSize(); i += WORLD_VIEW/2)
	{
		for (int j=2 * WORLD_VIEW; j < terrain->getSize(); j += WORLD_VIEW/2)
		{
			if ((rand() % 10 > 4) && (bumps < maxBumps))
			{
				float height = terrain->getHeight(Vector(i,0.0f,j));
				Scenery* s = new Scenery("media/sm_texture.bmp", Quaternion::IDENTITY, Vector(i, height+1.5f, j));
				Attach(s);
				bumps++;
			}
		}
	}
}

void AppManager::Shutdown()
{
	// clean up objects
	for (int i = 0; i < (int)Scene.size(); i++)
	{
		Scene[i]->Shutdown();
	}
	counter = 666;
	LifeCycle();
}
