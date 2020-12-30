#pragma once

#include "Module.h"
#include "Point.h"
#include "External/SDL/include/SDL.h"

enum class EntityType
{
	PLAYER,
	GROUND_ENEMY,
	AIR_ENEMY,
	CHECKPOINT,
	ITEM_DIAMOND,
	ITEM_POTION,
	UNKNOWN
};

class Entity : public Module
{
public:
	Entity(EntityType type) {}
	virtual ~Entity() {}

	// Called before render is available
	virtual bool Awake(pugi::xml_node&)
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PreUpdate()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool Update(float dt)
	{
		return true;
	}

	// Called before quitting
	virtual bool CleanUp()
	{
		return true;
	}

	//Called when loading the game
	virtual bool LoadState(pugi::xml_node&)
	{
		return true;
	}

	//Called when saving the game
	virtual bool SaveState(pugi::xml_node&) const
	{
		return true;
	}

public:
	EntityType type;
	fPoint position;
	iPoint positionPixelPerfect;
	fPoint velocity;
	SDL_Rect r;
	SDL_Rect rCollider;

	SString texPath;
	SDL_Texture* graphics;

	int life;
};