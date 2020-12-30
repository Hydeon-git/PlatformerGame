#ifndef _OBJECTS_H_
#define _OBJECTS_H_
#include "Module.h"
#include "Point.h"
#include "Animation.h"

struct SDL_Texture;

enum ObjectType
{
	NO_TYPE,
	DIAMOND,
	HEALTH_POTION
};

class Object
{
public:
	Object(iPoint objectPos, ObjectType type, SDL_Texture* tex);
	~Object();

	bool Draw(float dt);

	ObjectType type;
	Collider* collider = nullptr;
private:
	SDL_Rect rect = {};
	iPoint position;
	SDL_Texture* texture = nullptr;
	Animation anim;
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

	int healthPotionFx;
	SString diamondTexPath;
	SString healthPotionTexPath;

	int diamondFx;
	SDL_Texture* diamondTex = nullptr;
	SDL_Texture* healthPotionTex = nullptr;

	
};

#endif // __OBJECTS_H__
