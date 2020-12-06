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

enum PlayerStatus 
{
	PLAYER_IDLE,
	PLAYER_FORWARD,
	PLAYER_BACKWARD,
	PLAYER_JUMP,
	PLAYER_IN_AIR,
	PLAYER_HIT,
	PLAYER_DEATH
};

class Bullet : public Module
{
public:
	Bullet(SDL_Texture* grph, float sp, fPoint pos, bool fp, int sound);
	~Bullet();

	bool Update(float dt);
	bool OnCollision(Collider* c1, Collider* c2);

	//Public variables
	bool toDelete = false;

private:

	bool flip;
	fPoint position;
	SDL_Rect rect;
	SDL_Texture* graphics = nullptr;
	float speed = 1.0f;
	Collider* bulletCollider;
	int wallHitFx;
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

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called each loop iteration
	bool Draw(float dt);

	// Called before quitting
	bool CleanUp();
	bool DisablePlayer();
	bool EnablePlayer();

	// Player recibe hit
	void Hit(int damage);
	void Heal(int lifeHealed);

	// Load / Save
	bool SaveState(pugi::xml_node&) const;
	bool LoadState(pugi::xml_node&);

	// Collisions
	bool OnCollision(Collider* c1, Collider* c2);

	//Public variables
	Collider* colPlayer;

	fPoint position;
	iPoint positionPixelPerfect;

	int checkpoint = 0;
	bool godmode = false;
	bool dead = false;

private:

	bool ResetStates();
	
	int life = 100;
	int lifeConfig = 100;
	float speed;
	float jumpForce;
	int gravity;
	float deathTimerConfig;
	float deathTimer;
	int deathLimit;

	bool input = true;

	bool jumpEnable = true;
	bool doubleJump = true;
	bool flip = true;

	bool onGround;
	bool leftColliding;
	bool rightColliding;

	int jumpFx;
	int shotFx;
	int wallHitFx;
	int damageFx;
	int deathFx;

	SString texPath;
	SDL_Texture* graphics;

	fPoint velocity;
	iPoint initialPos;

	Animation* currentAnimation = &idle;
	Animation idle;
	Animation walk;
	Animation jump;
	Animation hit;
	Animation death;

	PlayerStatus status = PLAYER_IDLE;

	Collider* colPlayerWalls;

	SDL_Rect rCollider;
	SDL_Rect r;	

	iPoint gunOffset;

	//Bullet variables
	List<Bullet*> bullets;
	SString bulletTexPath;
	SDL_Texture* bulletGraphics;
	float bulletSpeed;
public:
	int bulletDamage;
};
#endif //__PLAYER_H__