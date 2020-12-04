#ifndef __FADETOBLACK_H__
#define __FADETOBLACK_H__

#include "Module.h"
#include "SDL\include\SDL_rect.h"

class FadeToBlack : public Module
{
public:
	FadeToBlack();
	virtual ~FadeToBlack();

	bool Start();
	bool Update(float dt);
	bool FadeToBlk(GameScene nextScene, float time = 2.0f);
	bool FadeToBlkLoad(float time = 2.0f);
	bool FadeToBlkCp(float time = 2.0f);

private:

	enum FadeStep
	{
		NONE,
		FADE_TO_BLACK,
		FADE_FROM_BLACK,
		FADE_TO_BLACK_CP
	} currentStep = FadeStep::NONE;

	Uint32 startTime = 0;
	Uint32 totalTime = 0;
	SDL_Rect screen;
	GameScene level = SCENE_INTRO;

	bool loadState;
};

#endif //__FADETOBLACK_H__