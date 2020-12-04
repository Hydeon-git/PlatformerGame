#ifndef __FADETOBLACK_H__
#define __FADETOBLACK_H__

#include "Module.h"
#include "SDL\include\SDL_rect.h"
#include "Point.h"

enum FadeType
{
	CHANGE_SCENE,
	LOAD_SAVE,
	MOVE_CHECKPOINT
};

class FadeToBlack : public Module
{
public:
	FadeToBlack();
	virtual ~FadeToBlack();

	bool Start();
	bool Update(float dt);
	bool FadeToBlk(FadeType ft, GameScene nextScene = SCENE_NONE, iPoint newPos = iPoint(0,0), float time = 2.0f);

private:
	enum FadeStep
	{
		NONE,
		FADE_TO_BLACK,
		FADE_FROM_BLACK,
	} currentStep = FadeStep::NONE;

	Uint32 startTime = 0;
	Uint32 totalTime = 0;
	SDL_Rect screen;

	GameScene level = SCENE_NONE;
	iPoint position;
	FadeType fadeType;
};

#endif //__FADETOBLACK_H__