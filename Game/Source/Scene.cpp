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
	fullscreenRect = new SDL_Rect{ 0,0,1280,720 };

	menuAudioPath = config.child("music").attribute("menuMusic").as_string();
	gameAudioPath = config.child("music").attribute("gameMusic").as_string();
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
			app->render->DrawTexture(introScreen, 0, 81, fullscreenRect, 3);

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
				app->fade->FadeToBlkVisualEffect();
				app->LoadGameRequest();				
			}

			if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
				app->debug = !app->debug;		
			
			// Draw map
			app->map->Draw();

			SString title("Map:%dx%d Tiles:%dx%d Tilesets:%d",
				app->map->data.width, app->map->data.height,
				app->map->data.tileWidth, app->map->data.tileHeight,
				app->map->data.tilesets.count());

			app->win->SetTitle(title.GetString());
			if (endCol->CheckCollision(app->player->rCollider))
			{
				LOG("colision detectada");
			}
		} break;
		case SceneEnd:
		{
			app->render->DrawTexture(endScreen, 0, 81, fullscreenRect, 3);
			if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
			{
				app->fade->FadeToBlk(Scene1);
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
		item->data->to_delete = true;
	app->map->groundCol.clear();
	app->collision->CleanUp();
	app->map->CleanUp();
	if (introScreen) app->tex->UnLoad(introScreen);

	switch (nextScene)
	{
		case Scene1:
		{
			app->audio->PlayMusic(gameAudioPath.GetString());
			app->map->Load("scifi_map.tmx");
			app->player->EnablePlayer();
			endCol = app->collision->AddCollider({ 720, 193, 15, 30 }, COLLIDER_END, this);
			currentScene = Scene1;
		} break;
		case SceneEnd:
		{
			app->render->camera.x = 0;
			app->render->camera.y = 0;

			endScreen = app->tex->Load("Assets/textures/screens/gameover1.png");
		} break;
	}
}

bool Scene::OnCollision(Collider* c1, Collider* c2)
{
	LOG("hola de nuevo");
	if (c1 == endCol && c2->type == COLLIDER_PLAYER)
	{
		
		//app->fade->FadeToBlk(SceneEnd);
	}
	return true;
}