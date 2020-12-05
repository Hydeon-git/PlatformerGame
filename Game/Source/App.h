#ifndef __APP_H__
#define __APP_H__

#include "Module.h"
#include "List.h"
#include "PerfTimer.h"
#include "Timer.h"

#include "PugiXml/src/pugixml.hpp"

#define CONFIG_FILENAME		"config.xml"
#define SAVE_STATE_FILENAME "save_game.xml"

// Modules
class Window;
class Input;
class Render;
class Textures;
class Audio;
class Scene;
class Map;
class Checkpoint;
class PathFinding;
class Collision;
class Player;
class GroundEnemy;
class AirEnemy;
class Objects;
class FadeToBlack;

class App
{
public:

	// Constructor
	App(int argc, char* args[]);

	// Destructor
	virtual ~App();

	// Called before render is available
	bool Awake();

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update();

	// Called before quitting
	bool CleanUp();

	// Add a new module to handle
	void AddModule(Module* module);

	// Exposing some properties for reading
	int GetArgc() const;
	const char* GetArgv(int index) const;
	const char* GetTitle() const;
	const char* GetOrganization() const;
	float GetFPS();

	// Change cap
	void ChangeCap();
	void Uncap();
	void ChangeCap(bool capped);

    // L02: DONE 1: Create methods to request Load / Save
	void LoadGameRequest();
	void SaveGameRequest() const;

	bool debug = false;

private:

	// Load config file
	// NOTE: It receives config document
	pugi::xml_node LoadConfig(pugi::xml_document&) const;

	// Call modules before each loop iteration
	void PrepareUpdate();

	// Call modules before each loop iteration
	void FinishUpdate();

	// Call modules before each loop iteration
	bool PreUpdate();

	// Call modules on each loop iteration
	bool DoUpdate();

	// Call modules after each loop iteration
	bool PostUpdate();

	// Load / Save
	bool LoadGame();
	bool SaveGame() const;

public:

	// Modules
	Window* win;
	Input* input;
	Render* render;
	Textures* tex;
	Audio* audio;
	Scene* scene;
	Map* map;
	Checkpoint* checkpoint;
	PathFinding* pathfinding;
	Collision* collision;
	Player* player;
	GroundEnemy* groundEnemy;
	AirEnemy* airEnemy;
	Objects* obj;
	FadeToBlack* fade;

private:

	int argc;
	char** args;
	SString title;
	SString organization;

	List<Module *> modules;

	// L01: DONE 2: Create new variables from pugui namespace
	// NOTE: Redesigned LoadConfig() to avoid storing this variables
	//pugi::xml_document configFile;
	//pugi::xml_node config;
	//pugi::xml_node configApp;

	PerfTimer			pTimer;
	uint64				frameCount = 0;
	Timer				startupTime;
	Timer				frameTime;
	Timer				lastSecFrameTime;
	uint32				lastSecFrameCount = 0;
	uint32				prevLastSecFrameCount = 0;
	float				dt = 0.0f;
	float				avgFPS = -1;
	int					hCappedMs = -1;
	int					lCappedMs = -1;
	bool				capState;					// If true cap is 60, if false cap is 30

	bool isCapped = true;

	mutable bool saveGameRequested;
	bool loadGameRequested;
};

extern App* app;

#endif	// __APP_H__