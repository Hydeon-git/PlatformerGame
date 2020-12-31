#ifndef __AIRENEMY_H__
#define __AIRENEMY_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"
#include "Module.h"
#include "Animation.h"
#include "Collision.h"

enum AirEnemyStatus 
{
	AIRENEMY_IDLE,
	AIRENEMY_MOVE,
	AIRENEMY_ATTACK,
	AIRENEMY_DEATH
};


class AirEnemy : public Entity
{
public:

	AirEnemy();

	// Destructor
	virtual ~AirEnemy();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called each loop iteration
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	// Called each loop iteration
	bool Draw(float dt);

	// Called before quitting
	bool CleanUp();
	bool DisableAirEnemy();
	bool EnableAirEnemy();

	// Load / Save
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

	// Collisions
	bool OnCollision(Collider* c1, Collider* c2);

	//Public variables
	Collider* colAirEnemy = nullptr;

	fPoint position;
	iPoint positionPixelPerfect;

private:

	bool ResetStates();

	int lifeConfig;
	float speed;
	int deathLimit;
	int damageFx;
	int damage;

	bool canAttack = false;
	float attackTimerConfig;
	float attackTimer;
	float animFrame;

	// Pathfinding variables
	int pathSteps = 0;
	iPoint nextPos;

	bool flip = true;
	bool dead = false;

	iPoint initialPos;

	Animation* currentAnimation = &idle;
	Animation idle;
	Animation move;
	Animation death;

	AirEnemyStatus status = AIRENEMY_IDLE;
};
#endif //__AIRENEMY_H__