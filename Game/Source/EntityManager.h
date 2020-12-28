#ifndef _ENTITYMANAGER_H
#define _ENTITYMANAGER_H

#include "Module.h"
#include "Entity.h"
#include "Player.h"
#include "AirEnemy.h"
#include "GroundEnemy.h"
#include "Checkpoint.h"
#include "List.h"
#include "External/PugiXml/src/pugixml.hpp"

class Entity;

class EntityManager : public Module
{
public:

	EntityManager();

	// Destructor
	~EntityManager();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	//Called when loading the game
	bool LoadState(pugi::xml_node&);

	//Called when saving the game
	bool SaveState(pugi::xml_node&) const;

	//Called when creating a new Entity
	Entity* CreateEntity(EntityType type);

	//Called when deleting a new Entity
	bool DeleteEntity(Entity*);

public:
	pugi::xml_node node;
	List <Entity*> entities;
};
#endif // !_ENTITYMANAGER_H