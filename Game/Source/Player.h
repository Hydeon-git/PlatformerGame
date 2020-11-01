#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "Module.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Point.h"
#include "Module.h"
#include "Animation.h"
#include "Collision.h"

enum player_status {
	PLAYER_IDLE,
	PLAYER_FORWARD,
	PLAYER_BACKWARD,
	PLAYER_JUMP,
	PLAYER_IN_AIR,
	PLAYER_DEATH
};


class Player : public Module
{
public:

	Player();

	// Destructor
	virtual ~Player();

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
	bool DisablePlayer();
	bool EnablePlayer();

	// Load / Save
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

	// Collisions
	bool OnCollision(Collider* c1, Collider* c2);

	//Public variables
	Collider* colPlayer;
	SDL_Rect rCollider;

	iPoint position;

private:

	bool ResetStates();

	int life = 100;
	float speed;
	float gravity;
	float deathTimer_config;
	float deathTimer;
	int deathLimit;

	bool input = true;
	bool godmode = false;

	bool jumpEnable = true;
	bool doubleJump = true;
	bool flip = true;
	bool dead = false;

	bool onGround;
	bool leftColliding;
	bool rightColliding;

	SString texPath;
	SDL_Texture* graphics;

	fPoint velocity;
	iPoint initialPos;

	Animation* current_animation = &idle;
	Animation idle;
	Animation walk;
	Animation jump;
	Animation death;

	player_status status = PLAYER_IDLE;

	Collider* colPlayerWalls;

	SDL_Rect r;	
};
#endif //__PLAYER_H__