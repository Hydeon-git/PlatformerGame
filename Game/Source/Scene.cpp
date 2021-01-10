#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Scene.h"
#include "Map.h"
#include "Pathfinding.h"
#include "EntityManager.h"
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

	player = nullptr;
	airEnemy = nullptr;
	groundEnemy = nullptr;
	checkpoint = nullptr;
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
	ChangeScene(SCENE_INTRO);

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

	// Activate and deactivate gui debug mode
	if (app->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN)
		app->guiDebug = !app->guiDebug;

	// Activate and deactivate debug mode
	if (app->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN)
		app->debug = !app->debug;
	

	switch (currentScene)
	{
	case SCENE_INTRO:
	{
		app->render->DrawTexture(introScreen, 0, 51, fullscreenRect, 3);

	} break;
	case SCENE_1:
	{
		// Load first level
		if (app->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
			app->fade->FadeToBlk(CHANGE_SCENE, SCENE_1);

		// Start from the beginning of the level
		if (app->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
			app->fade->FadeToBlk(CHANGE_SCENE, currentScene);

		//Save Game
		if ((app->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) && (player->dead == false))
			app->SaveGameRequest();

		//Load Game
		if ((app->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) && (player->dead == false))
			LoadLastSave();

		if (app->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		{
			app->fade->FadeToBlk(MOVE_CHECKPOINT, SCENE_NONE, iPoint(checkpoint->position.x, checkpoint->position.y));
		}

		if ((app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) && (!player->dead) && (canPause))
		{
			if (pauseMenu)
			{
				GameUI();
				pauseMenu = false;
			}
			else PauseMenu();
		}
		
		// Draw map
		app->map->Draw();
		checkpoint->Draw(dt);
	} break;
	case SCENE_END:
	{
		pauseMenu = false;
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
	app->gui->ClearUI();
	GameUI();
	pauseMenu = false;
	app->fade->FadeToBlk(LOAD_SAVE);
}

// Called each loop iteration
bool Scene::PostUpdate()
{
	bool ret = true;

	if(exitGame)
		ret = false;

	return ret;
}

// Called before quitting
bool Scene::CleanUp()
{
	LOG("Freeing scene");
	app->gui->ClearUI();
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
	app->obj->DeleteObjects();
	// Delete player and enemies
	if (currentScene == SCENE_1)
	{
		app->entityManager->DeleteEntity(player);
		app->entityManager->DeleteEntity(groundEnemy);
		app->entityManager->DeleteEntity(airEnemy);
		app->entityManager->DeleteEntity(checkpoint);
		player = nullptr;
		groundEnemy = nullptr;
		airEnemy = nullptr;
		checkpoint = nullptr;
	}
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

	app->gui->ClearUI();

	switch (nextScene)
	{
	case SCENE_NONE:
		LOG("ERROR: Scene loaded was none so intro scene loaded instead.");
		ChangeScene(SCENE_INTRO);
		break;
	case SCENE_INTRO:
	{
		app->audio->PlayMusic(menuAudioPath.GetString());
		introScreen = app->tex->Load(introTexturePath.GetString());

		MenuUI();

		currentScene = SCENE_INTRO;
	}
		break;
	case SCENE_1:
	{
 		app->audio->PlayMusic(gameAudioPath.GetString());
		player = (Player*)app->entityManager->CreateEntity(EntityType::PLAYER);
		airEnemy = (AirEnemy*)app->entityManager->CreateEntity(EntityType::AIR_ENEMY);
		groundEnemy = (GroundEnemy*)app->entityManager->CreateEntity(EntityType::GROUND_ENEMY);
		checkpoint = (Checkpoint*)app->entityManager->CreateEntity(EntityType::CHECKPOINT);

		player->EnablePlayer();
		groundEnemy->EnableGroundEnemy();
		airEnemy->EnableAirEnemy();

		if (app->map->Load(mapLevel1.GetString()) == true)
		{
			checkpoint->Load();

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

		GameUI();

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
	player->position = newPos;
	return true;
}

bool Scene::MenuUI()
{
	app->gui->ClearUI();

	int x_offset = 33;
	int y_offset = 182;

	button1 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset, y_offset, 92, 28 }, { 1, 1, 46, 14 }, "Start", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
	text1 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 16, y_offset + 6, 44, 44 }, { 10, 0, 0, 0 }, "Start");
	button2 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset + 90, y_offset, 92, 28 }, { 1, 1, 46, 14 }, "Options", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
	text2 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 98, y_offset + 6, 44, 44 }, { 10, 0, 0, 0 }, "Options");
	button3 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset + 180, y_offset, 92, 28 }, { 1, 1, 46, 14 }, "Credits", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
	text3 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 187, y_offset + 6, 44, 44 }, { 10, 0, 0, 0 }, "Credits");
	button4 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset + 270, y_offset, 92, 28 }, { 1, 1, 46, 14 }, "Exit", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
	text4 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 294, y_offset + 6, 44, 44 }, { 10, 0, 0, 0 }, "Exit");

	return true;
}

bool Scene::GameUI()
{
	app->gui->ClearUI();
	image1 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { 2, 2, 72, 24 }, { 67, 1, 72, 24 });

	text1 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { 37, 4, 1, 1 }, { 7, 0, 0, 0 }, std::to_string(player->life).c_str());
	text2 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { 37, 17, 1, 1 }, { 5, 0, 0, 0 }, std::to_string(player->diamonds).c_str());

	button1 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { 405, 5, 18, 18 }, { 48, 1, 18, 18 }, "Pause", { 48, 1, 18, 18 }, { 48, 1, 18, 18 }, false, { 0,0,0,0 }, this);
	image2 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { 411, 10, 6, 8 }, { 54, 22, 6, 8 });
	return true;
}

bool Scene::CreditsUI()
{
	app->gui->ClearUI();

	int x_offset = 140;
	int y_offset = 40;

	image1 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { x_offset, y_offset, 146, 168 }, { 2, 34, 73, 83 });

	text1 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 37, y_offset + 15, 44, 44 }, { 8, 0, 0, 0 }, "Made by");

	text2 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 15, y_offset + 45, 44, 44 }, { 10, 0, 0, 0 }, "Oscar Royo");

	text3 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 15, y_offset + 75, 44, 44 }, { 10, 0, 0, 0 }, "Albert Pou");

	//Back
	button5 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset + 27, y_offset + 130, 92, 28 }, { 1, 1, 46, 14 }, "Back", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
	text5 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 48, y_offset + 135, 44, 44 }, { 10, 0, 0, 0 }, "Back");

	optionsMenu = true;
	return true;
}

bool Scene::PauseMenu() 
{
	app->gui->ClearUI();
	if (currentScene == SCENE_1)
	{
		image1 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { 2, 2, 72, 24 }, { 67, 1, 72, 24 });
		text1 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { 37, 4, 1, 1 }, { 7, 0, 0, 0 }, std::to_string(player->life).c_str());
		text2 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { 37, 17, 1, 1 }, { 5, 0, 0, 0 }, std::to_string(player->diamonds).c_str());
	}

	int x_offset = 140;
	int y_offset = 50;
	
	image2 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { x_offset, y_offset, 146, 168 }, { 2, 34, 73, 83 });

	if (!optionsMenu)
	{ 
		image1 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { x_offset + 32, y_offset - 38, 80,38 }, { 82,50,40,19 });
		//Resume
		button1 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset+27, y_offset + 10, 92, 28 }, { 1, 1, 46, 14 }, "Resume", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
		text1 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset+34, y_offset+15, 44, 44 }, { 10, 0, 0, 0 }, "Resume");
		//Load
		button2 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset+27, y_offset+40, 92, 28 }, { 1, 1, 46, 14 }, "Load", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
		text2 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset+49, y_offset+45, 44, 44 }, { 10, 0, 0, 0 }, "Load");
		//Options
		button3 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset+27, y_offset+70, 92, 28 }, { 1, 1, 46, 14 }, "Options", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
		text3 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset+34, y_offset+75, 44, 44 }, { 10, 0, 0, 0 }, "Options");
		//Menu
		button4 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset+27, y_offset+100, 92, 28 }, { 1, 1, 46, 14 }, "Menu", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
		text4 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset+46, y_offset+105, 44, 44 }, { 10, 0, 0, 0 }, "Menu");
		//Exit
		button5 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset + 27, y_offset+130, 92, 28 }, { 1, 1, 46, 14 }, "Exit", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
		text5 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 52, y_offset + 135, 44, 44 }, { 10, 0, 0, 0 }, "Exit");
	}
	else
	{
		image1 = app->gui->CreateUIElement(UiType::IMAGE, nullptr, { x_offset + 32, y_offset - 38, 80,38 }, { 82,70,40,19 });
		//Checkbox
		text1 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 16, y_offset + 21, 44, 44 }, { 7, 0, 0, 0 }, "Fullscreen");
		button1 = app->gui->CreateUIElement(UiType::CHECKBOX, nullptr, { x_offset + 108, y_offset + 16, 19, 19 }, { 48, 1, 18, 18 }, "Fullscreen", { 90, 38, 6, 5 }, { 48, 1, 18, 18 }, fullscreen, { 0,0,0,0 }, this);
		text6 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 16, y_offset + 49, 44, 44 }, { 7, 0, 0, 0 }, "VSync");
		button6 = app->gui->CreateUIElement(UiType::CHECKBOX, nullptr, { x_offset + 108, y_offset + 44, 19, 19 }, { 48, 1, 18, 18 }, "VSync", { 90, 38, 6, 5 }, { 48, 1, 18, 18 }, vsync, { 0,0,0,0 }, this);
		//Slider
		text2 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset+45, y_offset + 75, 44, 44 }, { 10, 0, 0, 0 }, "Sound");
		button2 = app->gui->CreateUIElement(UiType::SLIDER, nullptr, { x_offset + 14, y_offset + 98, 118, 6 }, { 79, 30, 59, 3 }, "Sound", { 79, 36, 5, 9 });
		//Back
		button7 = app->gui->CreateUIElement(UiType::BUTTON, nullptr, { x_offset + 27, y_offset + 130, 92, 28 }, { 1, 1, 46, 14 }, "Back", { 1, 17, 46, 14 }, { 1, 1, 46, 14 }, false, { 0,0,0,0 }, this);
		text7 = app->gui->CreateUIElement(UiType::TEXT, nullptr, { x_offset + 48, y_offset + 135, 44, 44 }, { 10, 0, 0, 0 }, "Back");
	}
	
	pauseMenu = true;
	return true;
}

void Scene::OnClick(UI* interaction)
{
	if (currentScene == SCENE_INTRO)
	{
		if (interaction == button1)
		{
			if (optionsMenu)
			{
				fullscreen = !fullscreen;				
				if (fullscreen) SDL_SetWindowFullscreen(app->win->window, SDL_WINDOW_FULLSCREEN);
				else SDL_SetWindowFullscreen(app->win->window, SDL_WINDOW_RESIZABLE);							
			}
			else app->fade->FadeToBlk(CHANGE_SCENE, SCENE_1);
		}
		else if (interaction == button2)
		{
			optionsMenu = true;
			PauseMenu();
		}
		else if (interaction == button3)
		{
			CreditsUI();
		}
		else if (interaction == button4)
		{
			exitGame = true;
		}
		else if (interaction == button5)
		{
			if (optionsMenu)
			{
				optionsMenu = false;
				pauseMenu = false;
				MenuUI();
			}
		}
		else if (interaction == button6)
		{
			if (optionsMenu)
			{
				vsync = !vsync;
				if (vsync) app->render->vsyncState = true;
				else app->render->vsyncState = false;
			}
		}
		else if (interaction == button7)
		{
			if (optionsMenu)
			{
				optionsMenu = false;
				pauseMenu = false;
				MenuUI();
			}
		}
	}
	else if (currentScene == SCENE_1)
	{
		if (interaction == button1)
		{
			if (pauseMenu)
			{
				if (optionsMenu)
				{
					fullscreen = !fullscreen;
					if (fullscreen) SDL_SetWindowFullscreen(app->win->window, SDL_WINDOW_FULLSCREEN);
					else SDL_SetWindowFullscreen(app->win->window, SDL_WINDOW_RESIZABLE);
				}
				else
				{
					GameUI();
					pauseMenu = false;
				}
			}
			else PauseMenu();
		}
		else if (interaction == button2)
		{
			LoadLastSave();
		}
		else if (interaction == button3)
		{
			optionsMenu = true;
			PauseMenu();
		}
		else if (interaction == button4)
		{
			if (optionsMenu)
			{
				optionsMenu = false;
				PauseMenu();
			}
			else app->fade->FadeToBlk(CHANGE_SCENE, SCENE_INTRO);
		}
		else if (interaction == button5)
		{
			exitGame = true;
		}
		else if (interaction == button6)
		{
			if (optionsMenu)
			{
				vsync = !vsync;
				if (vsync) app->render->vsyncState = true;
				else app->render->vsyncState = false;
			}
		}
		else if (interaction == button7)
		{
			if (optionsMenu)
			{
				optionsMenu = false;
				PauseMenu();
			}
		}
	}
}