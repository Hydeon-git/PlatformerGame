#ifndef __TEXT_H__
#define __TEXT_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"

class Text
{
public:

	Text();

	// Destructor
	virtual ~Text();

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
	bool DisableGuiText();
	bool EnableGuiText();

	// Load / Save
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

private:
	fPoint position;
	iPoint positionPixelPerfect;
	SDL_Rect guiTextRect;
};
#endif //__TEXT_H__