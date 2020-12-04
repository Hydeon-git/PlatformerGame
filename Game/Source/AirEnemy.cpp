#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Scene.h"
#include "Map.h"
#include "Pathfinding.h"
#include "DynArray.h"
#include "Player.h"
#include "AirEnemy.h"

AirEnemy::AirEnemy() : Module()
{
	name.Create("airEnemy");

	//animations
	idle.PushBack({ 1, 1, 18, 10 });
	idle.PushBack({ 1, 1, 18, 10 });
	idle.PushBack({ 1, 1, 18, 10 });
	idle.PushBack({ 1, 1, 18, 10 });
	idle.PushBack({ 21, 1, 18, 10 });
	idle.PushBack({ 41, 1, 18, 10 });
	idle.PushBack({ 61, 1, 18, 10 });
	idle.PushBack({ 41, 1, 18, 10 });
	idle.PushBack({ 21, 1, 18, 10 });
	idle.speed = 0.3f;

	move.PushBack({ 1, 12, 18, 10 });
	move.PushBack({ 21, 12, 18, 10 });
	move.speed = 0.2f;

	death.PushBack({ 41, 12, 18, 10 });
	death.PushBack({ 41, 12, 18, 10 });
	death.PushBack({ 41, 12, 18, 10 });
	death.PushBack({ 21, 1, 18, 10 });
	death.PushBack({ 41, 1, 18, 10 });
	death.PushBack({ 61, 1, 18, 10 });
	death.PushBack({ 61, 12, 18, 10 });
	death.speed = 0.2f;
	death.loop = false;
}

// Destructor
AirEnemy::~AirEnemy()
{}

bool AirEnemy::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading enemy from config_file");

	texPath = config.child("texPath").attribute("tex").as_string();
	life = config.child("properties").attribute("life").as_int();
	speed = config.child("properties").attribute("speed").as_float();
	attackTimerConfig = config.child("properties").attribute("attackSpeed").as_float();
	damage = config.child("properties").attribute("damage").as_int();
	initialPos.x = config.child("initialPos1").attribute("x").as_int();
	initialPos.y = config.child("initialPos1").attribute("y").as_int();
	damageFx = app->audio->LoadFx(config.child("sounds").attribute("damageFx").as_string());

	deathLimit = app->scene->deathLimit;

	active = false;
	
	return ret;
}

bool AirEnemy::Start()
{
	bool ret = true;
	LOG("Creating AirEnemy");

	//Loading assets and properties from config file
	position.x = initialPos.x;
	position.y = initialPos.y;

	positionPixelPerfect.x = position.x;
	positionPixelPerfect.y = position.y;

	velocity.SetToZero();

	if(graphics == nullptr) graphics = app->tex->Load(texPath.GetString());
	flip = false;

	r = { positionPixelPerfect.x, positionPixelPerfect.y, 16, 10 };
	colAirEnemy = app->collision->AddCollider(r, COLLIDER_ENEMY, this);

	currentAnimation = &idle;

	return ret;
}

// Unload assets
bool AirEnemy::CleanUp()
{
	bool ret = false;
	LOG("Unloading enemy");
	if (colAirEnemy != nullptr)
	{
		colAirEnemy->toDelete = true;
		colAirEnemy = nullptr;
	}
	ret = app->tex->UnLoad(graphics);
	graphics = nullptr;
	return ret;
}

bool AirEnemy::Update(float dt) 
{
	bool ret = false;

	if (!dead) 
	{
		// Input
		if ((position.DistanceTo(app->player->position) < 128) && (app->player->godmode == false) && (app->player->life > 0) && (status != AIRENEMY_ATTACK))
		{
			if (canAttack && attackTimer <= 0)
			{
				status = AIRENEMY_ATTACK;
			}
			else
			{
				status = AIRENEMY_MOVE;
				attackTimer -= dt;
			}
		}
		else
		{
			status = AIRENEMY_IDLE;
			attackTimer = 0;
		}

		if (app->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN)
		{
			life = 0;
		}

		if (positionPixelPerfect.y > deathLimit || life <= 0)
		{
			velocity.x = 0;
			status = AIRENEMY_DEATH;
		}
		canAttack = false;
	}
	else status = AIRENEMY_IDLE;
	
	//Status
	switch (status)
	{
	case AIRENEMY_IDLE:
		velocity.SetToZero();
		currentAnimation = &idle;
		break;
	case AIRENEMY_MOVE:
	{
		currentAnimation = &move;
		static iPoint origin;
		// Target is player position
		iPoint playerPos = app->player->positionPixelPerfect;

		// Convert World position to map position
		origin = app->map->WorldToMap(positionPixelPerfect.x + 9, positionPixelPerfect.y);
		playerPos = app->map->WorldToMap(playerPos.x + 16, playerPos.y + 16);

		// Create new path
		app->pathfinding->CreatePath(origin, playerPos);
		const DynArray<iPoint> * path = app->pathfinding->GetLastPath();

		if (path->At(1) != NULL)
		{
			// Move Enemy to Player
			if (path->At(1)->x < origin.x)
			{
				velocity.y = 0;
				velocity.x = -speed;
				flip = false;
			}
			else if (path->At(1)->x > origin.x)
			{
				velocity.y = 0;
				velocity.x = speed;
				flip = true;
			}
			else if (path->At(1)->y < origin.y)
			{
				velocity.x = 0;
				velocity.y = -speed;
			}
			else if (path->At(1)->y > origin.y)
			{
				velocity.x = 0;
				velocity.y = speed;
			}
		}

		// Draw path
		if (app->debug)
		{
			for (uint i = 0; i < path->Count(); ++i)
			{
				iPoint nextPoint = app->map->MapToWorld(path->At(i)->x, path->At(i)->y);
				SDL_Rect pathRect = { nextPoint.x, nextPoint.y, 16, 16 };
				app->render->DrawRectangle(pathRect, 255, 0, 0, 100);
			}
		}
		break;
	}
	case AIRENEMY_ATTACK:
		currentAnimation = &move;
		app->player->Hit(damage);
		attackTimer = attackTimerConfig;
		status = AIRENEMY_IDLE;
		break;
	case AIRENEMY_DEATH:
		currentAnimation = &death;
		if (death.Finished()) 
		{
			dead = true;
			DisableAirEnemy();
		}
		break;
	default:
		break;
	}

	if (!dead)
	{
		if (velocity.x != 0) idle.Reset();

		//Change position from velocity
		position.x += (velocity.x * dt);
		position.y += (velocity.y * dt);

		positionPixelPerfect.x = round(position.x);
		positionPixelPerfect.y = round(position.y);

		//Collider position
		colAirEnemy->SetPos(positionPixelPerfect.x, positionPixelPerfect.y);
		r.x = positionPixelPerfect.x; r.y = positionPixelPerfect.y;

		//Function to draw the player
		ret = Draw(dt);
	}
	else ret = true;
	
	return ret;
}

bool AirEnemy::Draw(float dt)
{
	bool ret = false;
	r = currentAnimation->GetCurrentFrame(dt);
	if (graphics != nullptr) 
	{
		ret = app->render->DrawTexture(graphics, positionPixelPerfect.x, positionPixelPerfect.y, &r, 1, 1.0f, 0.0f, INT_MAX, INT_MAX, flip);
	}
	else LOG("No available graphics to draw.");

	r.x = positionPixelPerfect.x;
	r.y = positionPixelPerfect.y;
	return ret;
}

bool AirEnemy::OnCollision(Collider* c1, Collider* c2) 
{
	bool ret = false;

	if (c1 == colAirEnemy && c2->type == COLLIDER_BULLET)
	{
		//Take damage
		life -= app->player->bulletDamage;
		//Sound
		app->audio->PlayFx(damageFx);

		ret = true;
	}

	if (!app->player->godmode && (c1 == colAirEnemy && c2->type == COLLIDER_PLAYER))
	{
		velocity.SetToZero();
		canAttack = true;
	}
	return ret;
}

bool AirEnemy::EnableAirEnemy() //Enable function for changing scene
{
	LOG("Enabling air enemy");
	bool ret = false;
	active = true;
	ret = Start();
	return ret;
}

bool AirEnemy::DisableAirEnemy() //Disable function for changing scene
{
	LOG("Disabling air enemy");
	active = false;
	if (colAirEnemy != nullptr)
	{
		colAirEnemy->toDelete = true;
		colAirEnemy = nullptr;
	}
	return true;
}

bool AirEnemy::ResetStates() //Reset all states before checking input
{
	velocity.x = 0;
	velocity.y = 0;

	app->scene->loaded = false;

	return true;
}

bool AirEnemy::SaveState(pugi::xml_node& data) const 
{
	pugi::xml_node gEnemy = data.append_child("airEnemy");

	gEnemy.append_attribute("x") = positionPixelPerfect.x;
	gEnemy.append_attribute("y") = positionPixelPerfect.y;

	gEnemy.append_attribute("life") = life;
	gEnemy.append_attribute("dead") = dead;

	return true;
}

bool AirEnemy::LoadState(pugi::xml_node& data)
{
	LOG("Loading air enemy form savefile");
	pugi::xml_node gEnemy = data.child("airEnemy");

	position.x = gEnemy.attribute("x").as_int();
	position.y = gEnemy.attribute("y").as_int();

	dead = gEnemy.attribute("dead").as_bool();

	if (dead)
	{
		life = 0;
	}
	else 
	{
		life = gEnemy.attribute("life").as_int();
		positionPixelPerfect.x = position.x;
		positionPixelPerfect.y = position.y;

		colAirEnemy->SetPos(positionPixelPerfect.x + 13, positionPixelPerfect.y + 17);

		r.x = positionPixelPerfect.x + 13; r.y = positionPixelPerfect.y + 17;

		r.x = positionPixelPerfect.x;
		r.y = positionPixelPerfect.y;

		status = AIRENEMY_IDLE;
	}

	

	return true;
}