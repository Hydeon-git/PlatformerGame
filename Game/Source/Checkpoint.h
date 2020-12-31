#ifndef __CHECKPOINT_H__
#define __CHECKPOINT_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"
#include "Animation.h"
#include "Collision.h"

enum CheckpointStatus
{
	CHECKPOINT_UNCHECKED,
	CHECKPOINT_CHECKING,
	CHECKPOINT_CHECKED,
};


class Checkpoint : public Entity
{
public:

	Checkpoint();

	// Destructor
	virtual ~Checkpoint();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	bool Load();

	// Called each loop iteration
	bool Draw(float dt);

	// Called before quitting
	bool CleanUp();

	// Collisions
	bool OnCollision(Collider* c1, Collider* c2);

	//Public variables
	Collider* colCheckpoint = nullptr;

private:

	bool check = false;
	int checkpointFx;
	float checkTime;
	float checkTimer;
	float animFrame;

	Animation unchecked;
	Animation checking;
	Animation checked;

	Animation* currentAnimation = &unchecked;

	CheckpointStatus status = CHECKPOINT_UNCHECKED;
};
#endif //__CHECKPOINT_H__