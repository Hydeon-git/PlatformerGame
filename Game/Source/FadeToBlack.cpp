#include <math.h>
#include "App.h"
#include "Scene.h"
#include "FadeToBlack.h"
#include "Render.h"
#include "Window.h"
#include "Audio.h"
#include "Log.h"
#include "SDL/include/SDL_render.h"
#include "SDL/include/SDL_timer.h"

FadeToBlack::FadeToBlack()
{
	name.Create("fadetoblack");	
	screen = { 0, 0, 1280, 720 };
}

FadeToBlack::~FadeToBlack()
{}

// Load assets
bool FadeToBlack::Start()
{
	LOG("Preparing Fade Screen");
	loadState = false;
	SDL_SetRenderDrawBlendMode(app->render->renderer, SDL_BLENDMODE_BLEND);
	return true;
}

// Update: draw background
bool FadeToBlack::Update(float dt)
{
	if (current_step == fade_step::none)
		return true;

	Uint32 now = SDL_GetTicks() - start_time;
	float normalized = MIN(1.0f, (float)now / (float)total_time);

	switch (current_step)
	{
	case fade_step::fade_to_black:
	{
		if (now >= total_time)
		{
			if (loadState)app->LoadGameRequest();
			else app->scene->ChangeScene(level);
			app->render->SetBackgroundColor(app->render->background);
			total_time += total_time;
			start_time = SDL_GetTicks();
			current_step = fade_step::fade_from_black;
		}
	} break;

	case fade_step::fade_from_black:
	{
		normalized = 1.0f - normalized;

		if (now >= total_time)
			current_step = fade_step::none;
	} break;
	}

	// Finally render the black square with alpha on the screen
	SDL_SetRenderDrawColor(app->render->renderer, 0, 0, 0, (Uint8)(normalized * 255.0f));
	SDL_RenderFillRect(app->render->renderer, &screen);

	return true;
}

// Fade to black. At mid point deactivate one module, then activate the other
bool FadeToBlack::FadeToBlk(GameScene nextScene, float time)
{
	bool ret = false;
	if (current_step == fade_step::none)
	{
		loadState = false;
		current_step = fade_step::fade_to_black;
		start_time = SDL_GetTicks();
		total_time = (Uint32)(time * 0.5f * 1000.0f);
		level = nextScene;
		ret = true;
	}

	return ret;
}

bool FadeToBlack::FadeToBlkLoad(float time)
{
	bool ret = false;

	if (current_step == fade_step::none)
	{
		loadState = true;
		current_step = fade_step::fade_to_black;
		start_time = SDL_GetTicks();
		total_time = (Uint32)(time * 0.5f * 1000.0f);
		ret = true;
	}

	return ret;
}