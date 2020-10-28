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
	bool FadeToBlk(int module, float time = 2.0f);
	bool FadeToBlkVisualEffect(float time = 2.0f);

private:

	enum fade_step
	{
		none,
		fade_to_black,
		fade_from_black
	} current_step = fade_step::none;

	Uint32 start_time = 0;
	Uint32 total_time = 0;
	SDL_Rect screen;
	int level = 0;
};

#endif //__FADETOBLACK_H__