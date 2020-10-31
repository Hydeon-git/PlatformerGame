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
	introRect = nullptr;
	currentScene = GameScene::SceneIntro;
}

// Destructor
Scene::~Scene()
{}

// Called before render is available
bool Scene::Awake(pugi::xml_node& config)
{
	bool ret = true;
	LOG("Loading Scene");
	introRect = new SDL_Rect{ 0,0,1280,720 };

	menuAudioPath = config.child("menu").attribute("menuMusic").as_string();
	gameAudioPath = config.child("scene1").attribute("gameMusic").as_string();
	audioVol = config.child("properties").attribute("volume").as_int();

	return ret;
}

// Called before the first frame
bool Scene::Start()
{	
	app->player->DisablePlayer();	
	introScreen = app->tex->Load("Assets/textures/screens/intro_image.png");
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
		case SceneIntro:
		{
			app->render->DrawTexture(introScreen, 0, 81, introRect, 3);

			if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				app->fade->FadeToBlk(Scene1);
			}

		} break;
		case Scene1:
		{
			// Falta F1 Load first level
			if (app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
				app->fade->FadeToBlk(Scene1);

			// Falta F3 Start from the beginning of the level
			if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
				app->fade->FadeToBlk(currentScene);

			if (app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
				app->SaveGameRequest();

			if (app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
			{
				app->LoadGameRequest();
				app->fade->FadeToBlkVisualEffect();
			}

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
		} break;
		case GameOver:
		{} break;
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
	delete introRect;
	introRect = nullptr;
	return true;
}

void Scene::ChangeScene(GameScene nextScene)
{
	ListItem<Collider*>* item;
	for (item = app->map->groundCol.start; item != NULL; item = item->next) //deleting all colliders
		item->data->to_delete = true;
	app->map->groundCol.clear();
	app->collision->CleanUp();
	app->map->CleanUp();
	switch (nextScene)
	{
		case Scene1:
		{
			app->audio->PlayMusic(gameAudioPath.GetString());
			app->map->Load("scifi_map.tmx");
			app->player->EnablePlayer();
			currentScene = Scene1;
		} break;
	}
}