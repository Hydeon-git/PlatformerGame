#ifndef __GROUNDENEMY_H__
#define __GROUNDENEMY_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"
#include "Module.h"
#include "Animation.h"
#include "Collision.h"

enum GroundEnemyStatus 
{
	GROUNDENEMY_IDLE,
	GROUNDENEMY_MOVE,
	GROUNDENEMY_ATTACK,
	GROUNDENEMY_DEATH
};


class GroundEnemy : public Entity
{
public:

	GroundEnemy();

	// Destructor
	virtual ~GroundEnemy();

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
	bool DisableGroundEnemy();
	bool EnableGroundEnemy();

	// Load / Save
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

	// Collisions
	bool OnCollision(Collider* c1, Collider* c2);

	//Public variables
	Collider* colGroundEnemy = nullptr;

private:

	bool ResetStates();

	int lifeConfig;
	float speed;
	int gravity;
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

	bool onGround;

	iPoint initialPos;

	Animation* currentAnimation = &idle;
	Animation idle;
	Animation move;
	Animation death;

	GroundEnemyStatus status = GROUNDENEMY_IDLE;
};
#endif //__GROUNDENEMY_H__