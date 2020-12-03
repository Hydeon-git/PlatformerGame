#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Scene.h"
#include "Map.h"
#include "Pathfinding.h"
#include "DynArray.h"
#include "Player.h"
#include "GroundEnemy.h"

GroundEnemy::GroundEnemy() : Module()
{
	name.Create("groundEnemy");

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
GroundEnemy::~GroundEnemy()
{}

bool GroundEnemy::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading enemy from config_file");

	texPath = config.child("texPath").attribute("tex").as_string();
	life = config.child("propierties").attribute("life").as_int();
	speed = config.child("propierties").attribute("speed").as_float();
	gravity = config.child("propierties").attribute("gravity").as_float();
	initialPos.x = config.child("initialPos1").attribute("x").as_int();
	initialPos.y = config.child("initialPos1").attribute("y").as_int();
	damageFx = app->audio->LoadFx(config.child("sounds").attribute("damageFx").as_string());

	deathLimit = app->scene->deathLimit;
	
	return ret;
}

bool GroundEnemy::Start()
{
	bool ret = true;

	//Loading assets and propierties from config file
	position.x = initialPos.x;
	position.y = initialPos.y;
	velocity.SetToZero();
	onGround = true;

	if(graphics == nullptr) graphics = app->tex->Load(texPath.GetString());
	flip = false;

	LOG("Creating ground enemy colliders");
	r = { positionPixelPerfect.x, positionPixelPerfect.y, 16, 10 };
	colGroundEnemy = app->collision->AddCollider(r, COLLIDER_ENEMY, this);

	currentAnimation = &idle;

	return ret;
}

// Unload assets
bool GroundEnemy::CleanUp()
{
	bool ret = false;
	LOG("Unloading enemy");
	colGroundEnemy->toDelete = true;
	ret = app->tex->UnLoad(graphics);
	graphics = nullptr;
	return ret;
}

bool GroundEnemy::Update(float dt) 
{
	bool ret = false;

	if (!dead) 
	{
		// Input
		if (onGround)
		{
			if ((position.DistanceTo(app->player->position) < 128) && (app->player->godmode == false) && (app->player->life > 0))
			{
				status = GROUNDENEMY_MOVE;
			}
			else status = GROUNDENEMY_IDLE;
		}
		else
		{
			velocity.y += gravity * dt;
			status = GROUNDENEMY_IDLE;
		}

		if (app->input->GetKey(SDL_SCANCODE_J) == KEY_DOWN)
		{
			life = 0;
		}

		if (positionPixelPerfect.y > deathLimit || life <= 0)
		{
			velocity.x = 0;
			status = GROUNDENEMY_DEATH;
		}
	}
	else status = GROUNDENEMY_IDLE;
	
	//Status
	switch (status)
	{
	case GROUNDENEMY_IDLE:
		velocity.x = 0;
		currentAnimation = &idle;
		break;
	case GROUNDENEMY_MOVE:
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
				velocity.x = -speed;
				flip = false;
			}
			else if (path->At(1)->x > origin.x)
			{
				velocity.x = speed;
				flip = true;
			}
			if (path->At(1)->y < origin.y)
			{
				velocity.x = 0;
				currentAnimation = &idle;
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
	case GROUNDENEMY_DEATH:
		currentAnimation = &death;
		if (death.Finished()) 
		{
			dead = true;
			DisableGroundEnemy();
		}
		break;
	default:
		break;
	}

	if(velocity.x != 0) idle.Reset();

	//Change position from velocity
	position.x += (velocity.x * dt);
	position.y += (velocity.y * dt);

	positionPixelPerfect.x = round(position.x);
	positionPixelPerfect.y = round(position.y);

	//Collider position
	colGroundEnemy->SetPos(positionPixelPerfect.x, positionPixelPerfect.y);
	r.x = positionPixelPerfect.x; r.y = positionPixelPerfect.y;

	//Function to draw the player
	if(!dead) ret = Draw(dt);
	onGround = false;
	return true;
}

bool GroundEnemy::Draw(float dt)
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

bool GroundEnemy::OnCollision(Collider* c1, Collider* c2) 
{
	bool ret = false;

	if (c1 == colGroundEnemy && c2->type == COLLIDER_GROUND)
	{
		if (velocity.y != 0) position.y = c2->rect.y - c1->rect.h;
		velocity.y = 0;
		onGround = true;
		ret = true;
	}

	if (c1 == colGroundEnemy && c2->type == COLLIDER_BULLET)
	{
		//Take damage
		life -= app->player->bulletDamage;
		//Sound
		app->audio->PlayFx(damageFx);

		ret = true;
	}

	if (!app->player->godmode && (c1 == colGroundEnemy && c2->type == COLLIDER_PLAYER))
	{
		velocity.x = 0;
	}
	return ret;
}

bool GroundEnemy::EnableGroundEnemy() //Enable function for changing scene
{
	LOG("Enabling enemy");
	bool ret = false;
	active = true;
	ret = Start();
	return ret;
}

bool GroundEnemy::DisableGroundEnemy() //Disable function for changing scene
{
	LOG("Disabling enemy");
	active = false;
	if(colGroundEnemy != nullptr) colGroundEnemy->toDelete = true;
	return true;
}

bool GroundEnemy::ResetStates() //Reset all states before checking input
{
	velocity.x = 0;
	velocity.y = 0;

	app->scene->loaded = false;

	return true;
}

bool GroundEnemy::SaveState(pugi::xml_node& data) const 
{
	pugi::xml_node gEnemy = data.append_child("groundEnemy");

	gEnemy.append_attribute("x") = positionPixelPerfect.x;
	gEnemy.append_attribute("y") = positionPixelPerfect.y;

	gEnemy.append_attribute("life") = life;
	gEnemy.append_attribute("dead") = dead;

	return true;
}

bool GroundEnemy::LoadState(pugi::xml_node& data)
{
	pugi::xml_node gEnemy = data.child("groundEnemy");

	position.x = gEnemy.attribute("x").as_int();
	position.y = gEnemy.attribute("y").as_int();

	dead = gEnemy.attribute("dead").as_bool();

	if (!dead) life = gEnemy.attribute("life").as_int();
	else life = 0;

	positionPixelPerfect.x = position.x;
	positionPixelPerfect.y = position.y;

	colGroundEnemy->SetPos(positionPixelPerfect.x + 13, positionPixelPerfect.y + 17);

	r.x = positionPixelPerfect.x + 13; r.y = positionPixelPerfect.y + 17;

	r.x = positionPixelPerfect.x;
	r.y = positionPixelPerfect.y;

	onGround = false;

	status = GROUNDENEMY_IDLE;

	return true;
}