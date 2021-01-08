#include <math.h>
#include "App.h"
#include "Scene.h"
#include "FadeToBlack.h"
#include "Render.h"
#include "Window.h"
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
		app->scene->canPause = false;
		if (now >= totalTime)
		{					
			switch (fadeType)
			{
			case CHANGE_SCENE:
				app->scene->ChangeScene(level);
				break;
			case LOAD_SAVE:
				app->LoadGameRequest();
				break;
			case MOVE_CHECKPOINT:
				app->scene->MovePlayer(position);
				break;
			default:
				break;
			}
			app->render->SetBackgroundColor(app->render->background);
			totalTime += totalTime;
			startTime = SDL_GetTicks();
			currentStep = FadeStep::FADE_FROM_BLACK;

		}
	} break;
	case FadeStep::FADE_FROM_BLACK:
	{		
		app->scene->canPause = false;
		normalized = 1.0f - normalized;

		if (now >= totalTime)
		{
			currentStep = FadeStep::NONE;
			app->scene->canPause = true;
		}
			
			
		
	} break;
	}

	// Finally render the black square with alpha on the screen
	SDL_SetRenderDrawColor(app->render->renderer, 0, 0, 0, (Uint8)(normalized * 255.0f));
	SDL_RenderFillRect(app->render->renderer, &screen);

	return true;
}

// Fade to black. At mid point deactivate one module, then activate the other
bool FadeToBlack::FadeToBlk(FadeType ft, GameScene nextScene, iPoint newPos, float time)
{
	bool ret = false;

	fadeType = ft;
	position = newPos;
	level = nextScene;
	
	if (currentStep == FadeStep::NONE)
	{
		currentStep = FadeStep::FADE_TO_BLACK;		
		startTime = SDL_GetTicks();
		totalTime = (Uint32)(time * 0.5f * 1000.0f);
		ret = true;
	}

	return ret;
}