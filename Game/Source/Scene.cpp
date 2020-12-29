#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Map.h"
#include "Checkpoint.h"
#include "Pathfinding.h"
#include "Player.h"
#include "GroundEnemy.h"
#include "AirEnemy.h"
#include "FadeToBlack.h"
#include "Objects.h"
#include "ModuleGUI.h"

#include "Defs.h"
#include "Log.h"

#include "SDL_image/include/SDL_image.h"

Scene::Scene() : Module()
{
	name.Create("scene");
	fullscreenRect = nullptr;
	ended = false;
	currentScene = GameScene::SCENE_INTRO;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake(pugi::xml_node& config)
{
	bool ret = true;
	LOG("Loading Scene");
	fullscreenRect = new SDL_Rect{ 0,0,1280,720 };

	menuAudioPath = config.child("music").attribute("menuMusic").as_string();
	gameAudioPath = config.child("music").attribute("gameMusic").as_string();
	winAudioPath = config.child("music").attribute("winMusic").as_string();
	audioVol = config.child("properties").attribute("volume").as_int();
	deathLimit = config.child("properties").attribute("deathLimit").as_int();
	gravity = config.child("properties").attribute("gravity").as_int();

	introTexturePath = config.child("textures").attribute("introTexture").as_string();
	endTexturePath = config.child("textures").attribute("endTexture").as_string();
	mapLevel1 = config.child("maps").attribute("level1").as_string();

	for (pugi::xml_node& node = config.child("objects").child("level1").child("obj"); node && ret; node = node.next_sibling("obj")) 
	{
		newobj* obj = new newobj;
		obj->pos = iPoint(node.attribute("x").as_int(), node.attribute("y").as_int());
		obj->type = node.attribute("type").as_int();
		objects.Add(obj);
	}

	return ret;
}

// Called before the first frame
bool Scene::Start()
{	
	introScreen = app->tex->Load(introTexturePath.GetString());

	app->audio->PlayMusic(menuAudioPath.GetString());
	app->audio->SetVolume(audioVol);

	CreateUI();

	return true;
}

// Called each loop iteration
bool Scene::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool Scene::Update(float dt)
{
	// Cap or Uncapp framerate
	if (app->input->GetKey(SDL_SCANCODE_F11) == KEY_DOWN)
		if (app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) app->ChangeCap();
		else app->ChangeCapState();

	switch (currentScene)
	{
		case SCENE_INTRO:
		{
			app->render->DrawTexture(introScreen, 0, 51, fullscreenRect, 3);

			if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				app->fade->FadeToBlk(CHANGE_SCENE, SCENE_1);
			}

		} break;
		case SCENE_1:
		{
			// Load first level
			if (app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
				app->fade->FadeToBlk(CHANGE_SCENE, SCENE_1);

			// Start from the beginning of the level
			if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
				app->fade->FadeToBlk(CHANGE_SCENE ,currentScene);

			//Save Game
			if ((app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) && (app->player->dead == false))
				app->SaveGameRequest();

			//Load Game
			if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
				LoadLastSave();

			// Activate and deactivate debug mode
			if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
				app->debug = !app->debug;

			if (app->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
			{
				app->fade->FadeToBlk(MOVE_CHECKPOINT, SCENE_NONE, app->checkpoint->position);
			}
			
			// Draw map
			app->map->Draw();
			app->checkpoint->Draw(dt);
		} break;
		case SCENE_END:
		{
			app->render->DrawTexture(endScreen, 0, 51, fullscreenRect, 3);
			if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				app->fade->FadeToBlk(CHANGE_SCENE, SCENE_1);
			}
		} break;
	}

	return true;
}

void Scene::LoadLastSave()
{
	app->fade->FadeToBlk(LOAD_SAVE);
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");
	delete fullscreenRect;
	fullscreenRect = nullptr;
	if (introScreen) app->tex->UnLoad(introScreen);
	if (endScreen) app->tex->UnLoad(endScreen);

	if (objects.Count() > 0) objects.Clear();

	return true;
}

void Scene::ChangeScene(GameScene nextScene)
{
	LOG("Changing scene");
	app->checkpoint->CleanUp();
	app->obj->DeleteObjects();
	// Disabling player and enemies
	app->player->DisablePlayer();
	app->groundEnemy->DisableGroundEnemy();
	app->airEnemy->DisableAirEnemy();
	// Unloading menu and ending screens
	if (introScreen) app->tex->UnLoad(introScreen);
	if (endScreen) app->tex->UnLoad(endScreen);

	// Deleting all ground and end colliders
	ListItem<Collider*>* item;
	for (item = app->map->groundCol.start; item != NULL; item = item->next) 
		item->data->toDelete = true;
	if (endCol != nullptr)
	{
		endCol->toDelete = true;
		endCol = nullptr;
	}
	app->map->groundCol.Clear();
	app->collision->CleanUp();
	// Deleting map
	app->map->CleanUp();

	switch (nextScene)
	{
	case SCENE_NONE:
		LOG("ERROR: Scene loaded was none so intro scene loaded instead.");
		ChangeScene(SCENE_INTRO);
		break;
	case SCENE_INTRO:
		app->audio->PlayMusic(menuAudioPath.GetString());
		introScreen = app->tex->Load(introTexturePath.GetString());
		CreateUI();
		currentScene = SCENE_INTRO;
		break;
	case SCENE_1:
	{
		app->gui->ClearUI();
 		app->audio->PlayMusic(gameAudioPath.GetString());

		app->player->EnablePlayer();
		app->groundEnemy->EnableGroundEnemy();
		app->airEnemy->EnableAirEnemy();

		if (app->map->Load(mapLevel1.GetString()) == true)
		{
			app->checkpoint->Load();

			int w, h;
			uchar* data = NULL;
			if (app->map->CreateWalkabilityMap(w, h, &data))
				app->pathfinding->SetMap(w, h, data);

			RELEASE_ARRAY(data);
		}

		// Objects
		for (int i = 0; i < objects.Count(); i++)
		{
			app->obj->CreateObject(objects[i]->pos, (ObjectType)objects[i]->type);
		}

		endCol = app->collision->AddCollider({ 960, 194, 15, 30 }, COLLIDER_END, this);
		ended = false;
		currentScene = SCENE_1;
	} break;
	case SCENE_END:
	{
		app->audio->PlayMusic(winAudioPath.GetString());
		endScreen = app->tex->Load(endTexturePath.GetString());
		currentScene = SCENE_END;
	} break;
	}
}

bool Scene::CreateUI() {

	app->gui->ClearUI();

	menu = true;

	int window_pos_x = 0;
	int window_pos_y = 0;

	//image = app->gui->CreateUIElement(Type::IMAGE, nullptr, { window_pos_x, window_pos_y, 46*2, 14*2 }, { 145, 81, 46, 14 });
	//image2 = app->gui->CreateUIElement(Type::TEXT, nullptr, { window_pos_x, window_pos_y+50, 44, 44 }, { 2, 34, 44, 44 }, "Options");
	//window = app->gui->CreateUIElement(Type::WINDOW, nullptr, { window_pos_x, window_pos_y, 48, 42 }, { 34, 0, 48, 42 });
	startButton = app->gui->CreateUIElement(Type::BUTTON, nullptr, { window_pos_x, window_pos_y, 92, 28}, { 145, 81, 46, 14 }, "Options", { 145, 97, 46, 14 }, { 145, 81, 46, 14 }, false, { 0,0,0,0 }, this);
	image2 = app->gui->CreateUIElement(Type::TEXT, startButton, { window_pos_x+20, window_pos_y+70, 44, 44 }, { 2, 34, 44, 44 }, "Options");
	//optionsButton = app->gui->CreateUIElement(Type::BUTTON, window, { window_pos_x + 8, window_pos_y + 5, 32, 9 }, { 0, 9, 32, 9 }, "OPTIONS", { 0, 9, 32, 9 }, { 0, 9, 32, 9 }, false, { 0,0,0,0 }, this);
	//creditsButton = app->gui->CreateUIElement(Type::BUTTON, window, { window_pos_x + 8, window_pos_y + 17, 32, 9 }, { 0, 18, 32, 9 }, "CREDITS", { 0, 18, 32, 9 }, { 0, 18, 32, 9 }, false, { 0,0,0,0 }, this);
	//quitButton = app->gui->CreateUIElement(Type::BUTTON, window, { window_pos_x + 8, window_pos_y + 29, 32, 9 }, { 0, 27, 32, 9 }, "QUIT", { 0, 27, 32, 9 }, { 0, 27, 32, 9 }, false, { 0,0,0,0 }, this);

	return true;
}

bool Scene::OnCollision(Collider* c1, Collider* c2)
{
	if (c1 == endCol && c2->type == COLLIDER_PLAYER)
	{
		if (!ended) 
		{
			app->fade->FadeToBlk(CHANGE_SCENE, SCENE_END);
			ended = true;
		}
	}
	return true;
}

bool Scene::MovePlayer(iPoint pos) 
{
	fPoint newPos;
	newPos.x = pos.x;
	newPos.y = pos.y;
	app->player->position = newPos;
	return true;
}