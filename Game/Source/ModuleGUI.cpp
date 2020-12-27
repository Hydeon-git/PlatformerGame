#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "ModuleGUI.h"

ModuleGUI::ModuleGUI() : Module()
{
	
}

// Destructor
ModuleGUI::~ModuleGUI()
{}

bool ModuleGUI::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading settings from config file");

	return ret;
}

bool ModuleGUI::Start()
{
	bool ret = true;
	LOG("Creating UI");

	return ret;
}

bool ModuleGUI::PreUpdate() 
{
	return true;
}

bool ModuleGUI::Update(float dt) 
{
	bool ret = false;
	return true;
}

bool ModuleGUI::Draw(float dt)
{
	bool ret = false;
	return ret;
}

// Unload assets
bool ModuleGUI::CleanUp()
{
	bool ret = true;
	LOG("Unloading UI");

	return ret;
}

bool ModuleGUI::SaveState(pugi::xml_node& data) const 
{
	LOG("Saving settings form savefile");
	pugi::xml_node st = data.append_child("settings");

	st.append_attribute("volume") = generalVolume;

	return true;
}

bool ModuleGUI::LoadState(pugi::xml_node& data)
{
	LOG("Loading settings form savefile");
	pugi::xml_node st = data.child("settings");

	generalVolume = st.attribute("volume").as_int();

	return true;
}