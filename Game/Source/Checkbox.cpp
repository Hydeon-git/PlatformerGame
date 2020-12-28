#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Checkbox.h"

Checkbox::Checkbox()
{}

// Destructor
Checkbox::~Checkbox()
{}

bool Checkbox::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading checkbox from config_file");
			
	return ret;
}

bool Checkbox::Start()
{
	bool ret = true;
	LOG("Creating Checkbox");
		
	return ret;
}

bool Checkbox::Update(float dt) 
{
	return true;
}

bool Checkbox::Draw(float dt)
{
	bool ret = false;
	
	return ret;
}

bool Checkbox::OnClick()
{
	return true;
}

// Unload assets
bool Checkbox::CleanUp()
{
	bool ret = true;
	LOG("Unloading checkbox");

	return ret;
}

bool Checkbox::EnableCbox() //Enable function for changing scene
{
	LOG("Enabling checkbox");
	bool ret = false;
	return ret;
}

bool Checkbox::DisableCbox() //Disable function for changing scene
{
	LOG("Disabling checkbox");	
	return true;
}

bool Checkbox::SaveState(pugi::xml_node& data) const 
{
	LOG("Loading checkbox form savefile");
	pugi::xml_node cbox = data.append_child("checkbox");

	return true;
}

bool Checkbox::LoadState(pugi::xml_node& data)
{
	LOG("Loading checkbox form savefile");
	pugi::xml_node cbox = data.child("checkbox");

	return true;
}