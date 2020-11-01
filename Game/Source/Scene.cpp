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
			app->render->DrawTexture(introScreen, 0, 51, fullscreenRect, 3);

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
		} break;
		case SceneEnd:
		{
			app->render->DrawTexture(endScreen, 0, 51, fullscreenRect, 3);
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
	if (endCol != nullptr)
	{
		endCol->to_delete = true;
		endCol = nullptr;
	}
	app->map->groundCol.clear();
	app->collision->CleanUp();
	app->map->CleanUp();
	if (introScreen) app->tex->UnLoad(introScreen);
	app->player->DisablePlayer();

	switch (nextScene)
	{
		case Scene1:
		{
			app->audio->PlayMusic(gameAudioPath.GetString());
			app->map->Load("scifi_map.tmx");
			app->player->EnablePlayer();
			endCol = app->collision->AddCollider({ 720, 194, 15, 30 }, COLLIDER_END, this);
			ended = false;
			currentScene = Scene1;
		} break;
		case SceneEnd:
		{
			endScreen = app->tex->Load("Assets/textures/screens/ending_image.png");
			currentScene = SceneEnd;
		} break;
	}
}

bool Scene::OnCollision(Collider* c1, Collider* c2)
{
	if (c1 == endCol && c2->type == COLLIDER_PLAYER)
	{
		if (!ended) 
		{
			app->fade->FadeToBlk(SceneEnd);
			app->audio->StopMusic();
			ended = true;
		}
	}
	return true;
}