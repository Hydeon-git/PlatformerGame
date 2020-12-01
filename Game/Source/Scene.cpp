#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Map.h"
#include "Player.h"
#include "FadeToBlack.h"

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

	introTexturePath = config.child("textures").attribute("introTexture").as_string();
	endTexturePath = config.child("textures").attribute("endTexture").as_string();
	mapLevel1 = config.child("maps").attribute("level1").as_string();
	

	return ret;
}

// Called before the first frame
bool Scene::Start()
{	
	app->player->DisablePlayer();	
	introScreen = app->tex->Load(introTexturePath.GetString());

	app->audio->PlayMusic(menuAudioPath.GetString());
	app->audio->SetVolume(audioVol);

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
	switch (currentScene)
	{
		case SCENE_INTRO:
		{
			app->render->DrawTexture(introScreen, 0, 51, fullscreenRect, 3);

			if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				app->fade->FadeToBlk(SCENE_1);
			}

		} break;
		case SCENE_1:
		{
			// Falta F1 Load first level
			if (app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
				app->fade->FadeToBlk(SCENE_1);

			// Falta F3 Start from the beginning of the level
			if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
				app->fade->FadeToBlk(currentScene);

			if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
				app->SaveGameRequest();

			if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
			{
				app->fade->FadeToBlkLoad();			
			}

			if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
				app->debug = !app->debug;		
			
			// Draw map
			app->map->Draw();
		} break;
		case SCENE_END:
		{
			app->render->DrawTexture(endScreen, 0, 51, fullscreenRect, 3);
			if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				app->fade->FadeToBlk(SCENE_1);
			}
		} break;
	}

	return true;
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
	app->tex->UnLoad(endScreen);
	return true;
}

void Scene::ChangeScene(GameScene nextScene)
{
	ListItem<Collider*>* item;
	for (item = app->map->groundCol.start; item != NULL; item = item->next) //deleting all colliders
		item->data->toDelete = true;
	if (endCol != nullptr)
	{
		endCol->toDelete = true;
		endCol = nullptr;
	}
	app->map->groundCol.Clear();
	app->collision->CleanUp();
	app->map->CleanUp();
	if (introScreen) app->tex->UnLoad(introScreen);
	app->player->DisablePlayer();

	switch (nextScene)
	{
		case SCENE_1:
		{
			app->audio->PlayMusic(gameAudioPath.GetString());
			app->map->Load(mapLevel1.GetString());
			app->player->EnablePlayer();
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

bool Scene::OnCollision(Collider* c1, Collider* c2)
{
	if (c1 == endCol && c2->type == COLLIDER_PLAYER)
	{
		if (!ended) 
		{
			app->fade->FadeToBlk(SCENE_END);
			ended = true;
		}
	}
	return true;
}