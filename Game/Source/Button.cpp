#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Button.h"

Button::Button()
{}

// Destructor
Button::~Button()
{}

bool Button::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading button from config_file");
			
	return ret;
}

bool Button::Start()
{
	bool ret = true;
	LOG("Creating Button");
		
	return ret;
}

bool Button::Update(float dt) 
{
	return true;
}

bool Button::Draw(float dt)
{
	bool ret = false;
	
	return ret;
}

bool Button::OnClick()
{
	return true;
}

// Unload assets
bool Button::CleanUp()
{
	bool ret = true;
	LOG("Unloading button");

	return ret;
}

bool Button::EnableButton() //Enable function for changing scene
{
	LOG("Enabling button");
	bool ret = false;
	return ret;
}

bool Button::DisableButton() //Disable function for changing scene
{
	LOG("Disabling button");	
	return true;
}

bool Button::SaveState(pugi::xml_node& data) const 
{
	LOG("Loading button form savefile");
	pugi::xml_node button = data.append_child("button");

	return true;
}

bool Button::LoadState(pugi::xml_node& data)
{
	LOG("Loading button form savefile");
	pugi::xml_node button = data.child("button");

	return true;
}