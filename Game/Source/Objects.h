#ifndef _OBJECTS_H_
#define _OBJECTS_H_
#include "Module.h"
#include "Point.h"

enum ObjectType
{
	NO_TYPE,
	DIAMOND,
	HEALTH_POTION
};

struct SDL_Texture;

class Object
{
public:
	Object(iPoint objectPos, ObjectType type, SDL_Texture* tex);
	~Object();

	bool Draw();

	ObjectType type;
	Collider* collider;
private:
	SDL_Rect rect = {};
	iPoint position;
	SDL_Texture* texture = nullptr;
};

class Objects : public Module
{
public:
	// Constructor
	Objects();
	
	// Destructor
	~Objects();

	// Called before render is available
	bool Awake(pugi::xml_node& config);
	bool Start();
	bool Update(float dt);
	bool CleanUp();

	bool DeleteObjects();
	bool DeleteObject(Object* obj);

	// OnCollision with another object
	bool OnCollision(Collider* c1, Collider* c2);

	Object* CreateObject(iPoint pos, ObjectType type);

private:
	// List of objects
	List<Object*> objects;

	SString diamondTexPath;
	SString healthPotionTexPath;

	SDL_Texture* diamondTex;
	SDL_Texture* healthPotionTex;

	int diamondFx;
};

#endif // __OBJECTS_H__
