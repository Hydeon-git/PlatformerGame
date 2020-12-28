#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Slider.h"

Slider::Slider()
{}

// Destructor
Slider::~Slider()
{}

bool Slider::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading slider from config_file");
			
	return ret;
}

bool Slider::Start()
{
	bool ret = true;
	LOG("Creating slider");
		
	return ret;
}

bool Slider::Update(float dt) 
{
	return true;
}

bool Slider::Draw(float dt)
{
	bool ret = false;
	
	return ret;
}

bool Slider::OnClick()
{
	return true;
}

// Unload assets
bool Slider::CleanUp()
{
	bool ret = true;
	LOG("Unloading slider");

	return ret;
}

bool Slider::EnableSlider() //Enable function for changing scene
{
	LOG("Enabling slider");
	bool ret = false;
	return ret;
}

bool Slider::DisableSlider() //Disable function for changing scene
{
	LOG("Disabling slider");	
	return true;
}

bool Slider::SaveState(pugi::xml_node& data) const 
{
	LOG("Loading slider form savefile");
	pugi::xml_node slider = data.append_child("slider");

	return true;
}

bool Slider::LoadState(pugi::xml_node& data)
{
	LOG("Loading slider form savefile");
	pugi::xml_node slider = data.child("slider");

	return true;
}