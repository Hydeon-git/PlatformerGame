#include "App.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Scene.h"
#include "Map.h"
#include "Checkpoint.h"
#include "Pathfinding.h"
#include "Collision.h"
#include "Player.h"
#include "GroundEnemy.h"
#include "AirEnemy.h"
#include "Objects.h"
#include "FadeToBlack.h"
#include "ModuleGUI.h"
#include "Fonts.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <sstream>

// Constructor
App::App(int argc, char* args[]) : argc(argc), args(args)
{
	frameCount = 0;

	win = new Window();
	input = new Input();
	render = new Render();
	tex = new Textures();
	audio = new Audio();
	gui = new ModuleGUI();
	fonts = new Fonts();
	scene = new Scene();
	map = new Map();
	checkpoint = new Checkpoint();
	pathfinding = new PathFinding();
	player = new Player();
	groundEnemy = new GroundEnemy();
	airEnemy = new AirEnemy();
	obj = new Objects();
	collision = new Collision();
	fade = new FadeToBlack();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(win);
	AddModule(input);
	AddModule(tex);	
	AddModule(audio);
	AddModule(scene);
	AddModule(map);
	AddModule(checkpoint);
	AddModule(pathfinding);
	AddModule(player);
	AddModule(groundEnemy);
	AddModule(airEnemy);
	AddModule(obj);
	AddModule(collision);
	AddModule(gui);
	AddModule(fonts);
	AddModule(fade);

	// Render last to swap buffer
	AddModule(render);
}

// Destructor
App::~App()
{
	// Release modules
	ListItem<Module*>* item = modules.end;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->prev;
	}

	modules.Clear();
}

void App::AddModule(Module* module)
{
	module->Init();
	modules.Add(module);
}

// Called before render is available
bool App::Awake()
{
	pugi::xml_document configFile;
	pugi::xml_node config;
	pugi::xml_node configApp;

	bool ret = false;

	// L01: DONE 3: Load config from XML
	config = LoadConfig(configFile);

	if (config.empty() == false)
	{
		ret = true;
		configApp = config.child("app");

		// L01: DONE 4: Read the title from the config file
		title.Create(configApp.child("title").child_value());
		organization.Create(configApp.child("organization").child_value());

		int highCap = configApp.child("fps").attribute("highCap").as_int();
		int lowCap = configApp.child("fps").attribute("lowCap").as_int();

		if (highCap > 0)
		{
			hCappedMs = 1000 / highCap;
			lCappedMs = 1000 / lowCap;
			capState = true;
		}
	}

	if (ret == true)
	{
		ListItem<Module*>* item;
		item = modules.start;

		while ((item != NULL) && (ret == true))
		{
			// L01: DONE 5: Add a new argument to the Awake method to receive a pointer to an xml node.
			// If the section with the module name exists in config.xml, fill the pointer with the valid xml_node
			// that can be used to read all variables for that module.
			// Send nullptr if the node does not exist in config.xml
			ret = item->data->Awake(config.child(item->data->name.GetString()));
			item = item->next;
		}
	}
	return ret;
}

// Called before the first frame
bool App::Start()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		if (item->data->active) ret = item->data->Start();
		else ret = true;
		item = item->next;
	}

	return ret;
}

// Called each loop iteration
bool App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT) == true)
		ret = false;

	if(ret == true)
		ret = PreUpdate();

	if(ret == true)
		ret = DoUpdate();

	if(ret == true)
		ret = PostUpdate();

	FinishUpdate();
	return ret;
}

// Load config from XML file
// NOTE: Function has been redesigned to avoid storing additional variables on the class
pugi::xml_node App::LoadConfig(pugi::xml_document& configFile) const
{
	pugi::xml_node ret;

	pugi::xml_parse_result result = configFile.load_file(CONFIG_FILENAME);

	if (result == NULL) LOG("Could not load xml file: %s. pugi error: %s", CONFIG_FILENAME, result.description());
	else ret = configFile.child("config");

	return ret;
}

// ---------------------------------------------
void App::PrepareUpdate()
{
	frameCount++;
	lastSecFrameCount++;

	dt = frameTime.ReadSec();
	frameTime.Start();
}

// ---------------------------------------------
void App::FinishUpdate()
{
	// L02: DONE 1: This is a good place to call Load / Save methods
	if (loadGameRequested == true) LoadGame();
	if (saveGameRequested == true) SaveGame();

	// Framerate calculations --

	if (lastSecFrameTime.Read() > 1000)
	{
		lastSecFrameTime.Start();
		prevLastSecFrameCount = lastSecFrameCount;
		lastSecFrameCount = 0;
	}

	avgFPS = float(frameCount) / startupTime.ReadSec();
	float secondsSinceStartup = startupTime.ReadSec();
	uint32 lastFramems = frameTime.Read();
	uint32 framesOnLastUpdate = prevLastSecFrameCount;

	// Change Window Title
	char title[256];
	
	sprintf_s(title, 256, "Space Thief | FPS: %i | Av.FPS: %.2f | Last Frame Ms: %02u | VSync: %s",
		framesOnLastUpdate, avgFPS, lastFramems, app->render->vsyncState ? "ON" : "OFF");
	
	app->win->SetTitle(title);

	if (isCapped && hCappedMs > 0 && lastFramems < hCappedMs)
	{
		PerfTimer t;
		if (capState)
		{
			SDL_Delay(hCappedMs - lastFramems);
		}
		else if (!capState)
		{
			SDL_Delay(lCappedMs - lastFramems);
		}
	}
}

// Call modules before each loop iteration
bool App::PreUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) 
		{
			continue;
		}

		ret = item->data->PreUpdate();
	}

	return ret;
}

// Call modules on each loop iteration
bool App::DoUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) 
		{
			continue;
		}

		ret = item->data->Update(dt);
	}

	return ret;
}

// Call modules after each loop iteration
bool App::PostUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) 
		{
			continue;
		}

		ret = item->data->PostUpdate();
	}

	return ret;
}

// Called before quitting
bool App::CleanUp()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.end;

	while(item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}

	return ret;
}

// ---------------------------------------
int App::GetArgc() const
{
	return argc;
}

// ---------------------------------------
const char* App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}

// ---------------------------------------
const char* App::GetTitle() const
{
	return title.GetString();
}

// ---------------------------------------
const char* App::GetOrganization() const
{
	return organization.GetString();
}

float App::GetFPS()
{
	return avgFPS;
}

void App::ChangeCapState()
{
	capState = !capState;
}

void App::ChangeCap()
{
	isCapped = !isCapped;
}

// Load / Save
void App::LoadGameRequest()
{
	// NOTE: We should check if SAVE_STATE_FILENAME actually exist
	pugi::xml_document savestateFile;
	pugi::xml_parse_result result = savestateFile.load_file(SAVE_STATE_FILENAME);
	if (result != NULL) 
	{
		loadGameRequested = true;
	}
}

// ---------------------------------------
void App::SaveGameRequest() const
{
	// NOTE: We should check if SAVE_STATE_FILENAME actually exist and... should we overwriten
	pugi::xml_document savestateFile;
	pugi::xml_parse_result result = savestateFile.load_file(SAVE_STATE_FILENAME);
	if (result == NULL) 
	{
		SDL_RWFromFile(SAVE_STATE_FILENAME, "w");
	}

	saveGameRequested = true;
}

// ---------------------------------------
// L02: TODO 5: Create a method to actually load an xml file
// then call all the modules to load themselves
bool App::LoadGame()
{
	bool ret = false;

	pugi::xml_document data;
	pugi::xml_node root;

	//Load savegame.xml file using load_file() method from the xml_document class
	pugi::xml_parse_result result = data.load_file(SAVE_STATE_FILENAME);

	//Check result for loading errors
	if (result != NULL)
	{
		LOG("Loading game state from %s", SAVE_STATE_FILENAME);

		root = data.child("save_state");
		ListItem<Module*>* item = modules.start;
		ret = true;
		while (item != NULL && ret == true)
		{
			ret = item->data->LoadState(root.child(item->data->name.GetString()));
			item = item->next;
		}
		player->dead = false;
	}
	else
	{
		LOG("Could not load game state from %s. Pugi error: %s", SAVE_STATE_FILENAME, result.description());
	}

	loadGameRequested = false;

	return ret;
}

// L02: TODO 7: Implement the xml save method for current state
bool App::SaveGame() const
{
	bool ret = true;

	LOG("Saving game state to %s", SAVE_STATE_FILENAME);

	pugi::xml_document data;
	pugi::xml_node root;

	root = data.append_child("save_state");

	ListItem<Module*>* item = modules.start;

	while (item != NULL && ret == true)
	{
		ret = item->data->SaveState(root.append_child(item->data->name.GetString()));
		item = item->next;
	}

	if (ret == true) 
	{
		data.save_file(SAVE_STATE_FILENAME);
	}

	saveGameRequested = false;

	return ret;
}



