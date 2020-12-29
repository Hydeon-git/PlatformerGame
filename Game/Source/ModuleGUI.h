#ifndef __MODULEGUI_H__
#define __MODULEGUI_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"
#include "Module.h"
#include "Button.h"
#include "Checkbox.h"
#include "Slider.h"
#include "Text.h"

class ModuleGUI : public Module
{
public:

	ModuleGUI();

	// Destructor
	virtual ~ModuleGUI();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called each loop iteration
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called each loop iteration
	bool Draw(float dt);

	// Called before quitting
	bool CleanUp();

	// Load - Save Settings XML
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

	// Add Widgets
	bool AddButton();
	bool AddCbox();
	bool AddSlider();
	bool AddText();

private:	
	bool input = true;
	bool flip = true;
	int generalVolume;
};
#endif //__MODULEGUI_H__