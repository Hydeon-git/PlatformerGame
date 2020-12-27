#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"

class Button
{
public:

	Button();

	// Destructor
	virtual ~Button();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called each loop iteration
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	// Called each loop iteration
	bool Draw(float dt);

	// Called before quitting
	bool CleanUp();

	// Managment Functions
	bool OnClick();
	bool DisableButton();
	bool EnableButton();

	// Load / Save
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

private:
	fPoint position;
	iPoint positionPixelPerfect;
	SDL_Rect buttonRect;
};
#endif //__BUTTON_H__