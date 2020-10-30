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
bool Scene::Awake(pugi::xml_node&)
{
	bool ret = true;
	LOG("Loading Scene");
	introRect = new SDL_Rect{ 0,0,1280,720 };
	return ret;
}

// Called before the first frame
bool Scene::Start()
{	
	app->player->DisablePlayer();	
	introScreen = app->tex->Load("Assets/textures/screens/intro1.png");	
	//app->audio->PlayMusic("Assets/audio/music/music_spy.ogg");
	app->audio->SetVolume(80);

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
			// Audio doesn't load
			//app->audio->PlayMusic("Assets/audio/music/music_spy.ogg");
			

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

	return true;
}

void Scene::ChangeScene(GameScene nextScene)
{
	switch (nextScene)
	{
		case Scene1:
		{
			app->audio->PlayMusic("Assets/audio/music/music_spy.ogg");
			app->map->Load("scifi_map.tmx");
			app->player->EnablePlayer();	
			currentScene = Scene1;
		} break;
	}
}