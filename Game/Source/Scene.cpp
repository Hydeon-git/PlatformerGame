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

Scene::Scene() : Module()
{
	name.Create("scene");

	currentScene = GameScene::SceneIntro;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake()
{
	bool ret = true;
	LOG("Loading Scene");

	return ret;
}

// Called before the first frame
bool Scene::Start()
{
	bool ret = false;

	ret = app->map->Load("scifi_map.tmx");
	ret = app->audio->PlayMusic("Assets/audio/music/music_spy.ogg");
	app->audio->SetVolume(0);

	if (ret) loaded = true;

	return ret;
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
	case GameScene::SceneIntro:
	{
		app->audio->CleanUp();
		app->player->DisablePlayer();

		if (app->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN)
		{
			currentScene = Scene1;
		}

	}break;
	case GameScene::Scene1:
	{
		// Audio doesn't load
		//app->audio->PlayMusic("Assets/audio/music/music_spy.ogg");
		app->player->EnablePlayer();

		// Falta F1 Load first level

		// Falta F3 Start from the beginning of the level

		if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
			app->SaveGameRequest();

		if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
			app->LoadGameRequest();

		if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
			app->debug = !app->debug;

		if (app->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)	
			app->fade->FadeToBlkVisualEffect();
		
		// Draw map
		app->map->Draw();

		SString title("Map:%dx%d Tiles:%dx%d Tilesets:%d",
			app->map->data.width, app->map->data.height,
			app->map->data.tileWidth, app->map->data.tileHeight,
			app->map->data.tilesets.count());

		app->win->SetTitle(title.GetString());
	}break;
	case GameScene::GameOver:
	{		
	}break;
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

	return true;
}
