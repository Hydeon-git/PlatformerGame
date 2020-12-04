#ifndef _OBJECTS_H_
#define _OBJECTS_H_
#include "Module.h"
#include "Point.h"

enum ObjectType
{
	NO_TYPE,
	DIAMOND,
	H_POTION
};

struct SDL_Texture;

class Objects : public Module
{
public:
	// Constructor
	Objects();
	Objects(fPoint objectPos, ObjectType type);
	
	// Destructor
	~Objects();

	bool Start();
	bool Update(float dt);
	bool CleanUp();

	// OnCollision with another object
	bool OnCollision(Collider* c1, Collider* c2);

	// Add an object into the queue to be spawned later
	List<Objects*> objects;
	

public:
	// Variables;
	fPoint position;
	ObjectType type;
	// Textures
	SDL_Texture* diamondTex = nullptr;
	SString diamondTexPath;
	SDL_Rect dRect = {};
	Collider* dCol = nullptr;
};

#endif // __OBJECTS_H__
