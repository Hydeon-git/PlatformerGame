#include "EntityManager.h"
#include "Log.h"

EntityManager::EntityManager()
{
	name.Create("entitymanager");
}

//Destructor
EntityManager::~EntityManager()
{}

// Called before render is available
bool EntityManager::Awake(pugi::xml_node& config)
{
	node = config;
	return true;
}

// Called before the first frame
bool EntityManager::Start()
{
	return true;
}

// Called each loop iteration
bool EntityManager::PreUpdate()
{
	for (unsigned int i = 0; i < entities.Count(); i++)
	{
		entities.At(i)->data->PreUpdate();
	}
	return true;
}

// Called each loop iteration
bool EntityManager::Update(float dt)
{
	for (unsigned int i = 0; i < entities.Count(); i++)
	{
		entities.At(i)->data->Update(dt);
	}
	return true;
}

// Called before quitting
bool EntityManager::CleanUp()
{
	for (int i = entities.Count() - 1; i >= 0; i--)
	{
		entities.At(i)->data->CleanUp();
		entities.Del(entities.At(i));
	}
	entities.Clear();
	return true;
}

//Called when loading the game
bool EntityManager::LoadState(pugi::xml_node& n)
{
	for (unsigned int i = 0; i < entities.Count(); i++)
	{
		entities.At(i)->data->LoadState(n.child(entities.At(i)->data->name.GetString()));
	}
	return true;
}

//Called when saving the game
bool EntityManager::SaveState(pugi::xml_node& s) const
{
	for (unsigned int i = 0; i < entities.Count(); i++)
	{
		entities.At(i)->data->SaveState(s.append_child(entities.At(i)->data->name.GetString()));
	}
	return true;
}

//Called when creating a new Entity
Entity* EntityManager::CreateEntity(EntityType type)
{
	static_assert(EntityType::UNKNOWN == (EntityType)6, "Types need update");
	Entity* ret = nullptr;
	switch (type)
	{
	case EntityType::PLAYER:
		ret = new Player();
		ret->type = EntityType::PLAYER;
		break;
	case EntityType::AIR_ENEMY:
		ret = new AirEnemy();
		ret->type = EntityType::AIR_ENEMY;
		break;
	case EntityType::GROUND_ENEMY:
		ret = new GroundEnemy();
		ret->type = EntityType::GROUND_ENEMY;
		break;
	case EntityType::CHECKPOINT:
		ret = new Checkpoint();
		ret->type = EntityType::CHECKPOINT;
		break;
	}

	if (ret != nullptr)
	{
		entities.Add(ret);
		ret->Awake(node.child(ret->name.GetString()));
		ret->Start();
	}
	return ret;
}

//Called when deleting a new Entity
bool EntityManager::DeleteEntity(Entity* e)
{
	int n = entities.Find(e);
	if (n == -1)return false;
	else
	{
		entities.At(n)->data->CleanUp();
		entities.Del(entities.At(n));
		return true;
	}
}