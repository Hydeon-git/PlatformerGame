#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Text.h"

Text::Text()
{}

// Destructor
Text::~Text()
{}

bool Text::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading GUI Text from config_file");
			
	return ret;
}

bool Text::Start()
{
	bool ret = true;
	LOG("Creating GUI Text");
		
	return ret;
}

bool Text::Update(float dt) 
{
	return true;
}

bool Text::Draw(float dt)
{
	bool ret = false;
	
	return ret;
}

bool Text::OnClick()
{
	return true;
}

// Unload assets
bool Text::CleanUp()
{
	bool ret = true;
	LOG("Unloading GUI Text");

	return ret;
}

bool Text::EnableGuiText() //Enable function for changing scene
{
	LOG("Enabling GUI Text");
	bool ret = false;
	return ret;
}

bool Text::DisableGuiText() //Disable function for changing scene
{
	LOG("Disabling GUI Text");	
	return true;
}

bool Text::SaveState(pugi::xml_node& data) const 
{
	LOG("Loading GUI Text form savefile");
	pugi::xml_node guiText = data.append_child("guiText");

	return true;
}

bool Text::LoadState(pugi::xml_node& data)
{
	LOG("Loading GUI Text form savefile");
	pugi::xml_node guiText = data.child("guiText");

	return true;
}