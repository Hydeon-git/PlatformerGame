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
	if (currentStep == FadeStep::NONE)
		return true;

	Uint32 now = SDL_GetTicks() - startTime;
	float normalized = MIN(1.0f, (float)now / (float)totalTime);

	switch (currentStep)
	{
	case FadeStep::FADE_TO_BLACK:
	{
		if (now >= totalTime)
		{
			if (loadState)app->LoadGameRequest();
			else app->scene->ChangeScene(level);
			app->render->SetBackgroundColor(app->render->background);
			totalTime += totalTime;
			startTime = SDL_GetTicks();
			currentStep = FadeStep::FADE_FROM_BLACK;
		}
	} break;

	case FadeStep::FADE_FROM_BLACK:
	{
		normalized = 1.0f - normalized;

		if (now >= totalTime)
			currentStep = FadeStep::NONE;
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
	if (currentStep == FadeStep::NONE)
	{
		loadState = false;
		currentStep = FadeStep::FADE_TO_BLACK;
		startTime = SDL_GetTicks();
		totalTime = (Uint32)(time * 0.5f * 1000.0f);
		level = nextScene;
		ret = true;
	}

	return ret;
}

bool FadeToBlack::FadeToBlkLoad(float time)
{
	bool ret = false;

	if (currentStep == FadeStep::NONE)
	{
		loadState = true;
		currentStep = FadeStep::FADE_TO_BLACK;
		startTime = SDL_GetTicks();
		totalTime = (Uint32)(time * 0.5f * 1000.0f);
		ret = true;
	}

	return ret;
}