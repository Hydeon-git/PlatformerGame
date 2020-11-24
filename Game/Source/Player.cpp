#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Scene.h"
#include "Map.h"
#include "Player.h"

Player::Player() : Module()
{
	name.Create("player");

	//animations
	idle.PushBack({ 0, 0, 32, 32 });
	idle.PushBack({ 32, 0, 32, 32 });
	idle.speed = 0.1f;

	walk.PushBack({ 0, 32, 32, 32 });
	walk.PushBack({ 32, 32, 32, 32 });
	walk.PushBack({ 64, 32, 32, 32 });
	walk.PushBack({ 96, 32, 32, 32 });
	walk.PushBack({ 128, 32, 32, 32 });
	walk.PushBack({ 160, 32, 32, 32 });
	walk.speed = 0.4f;

	death.PushBack({ 0, 96, 32, 32 });
	death.PushBack({ 32, 102, 32, 26 });
	death.PushBack({ 64, 102, 32, 26 });
	death.PushBack({ 96, 102, 32, 26 });
	death.speed = 0.1f;
	death.loop = false;

	jump.PushBack({ 64, 0, 32, 32 });
	jump.PushBack({ 96, 0, 32, 32 });
	jump.speed = 0.1f;
}

// Destructor
Player::~Player()
{}

bool Player::Awake(pugi::xml_node& config) 
{
	bool ret = true;
	LOG("Loading player from config_file");

	texPath = config.child("texPath").attribute("tex").as_string();
	life = config.child("propierties").attribute("life").as_int();
	speed = config.child("propierties").attribute("speed").as_float();
	gravity = config.child("propierties").attribute("gravity").as_float();
	deathTimerConfig = config.child("death").attribute("time").as_float();
	deathLimit = config.child("death").attribute("height").as_int();
	initialPos.x = config.child("initialPos1").attribute("x").as_int();
	initialPos.y = config.child("initialPos1").attribute("y").as_int();
	jumpFx = app->audio->LoadFx(config.child("sounds").attribute("jumpFx").as_string());
	
	return ret;
}

bool Player::Start()
{
	bool ret = true;

	//Loading assets and propierties from config file
	position.x = initialPos.x;
	position.y = initialPos.y;
	if(graphics == nullptr) graphics = app->tex->Load(texPath.GetString());
	flip = false;

	LOG("Creating player colliders");
	rCollider = { position.x+13, position.y+17, 6, 15 };
	colPlayer = app->collision->AddCollider(rCollider, COLLIDER_PLAYER, this);
	colPlayerWalls = app->collision->AddCollider({position.x+11, position.y+18, 10, 13 }, COLLIDER_PLAYER, this);

	return ret;
}

// Unload assets
bool Player::CleanUp()
{
	bool ret = false;
	LOG("Unloading player");
	ret = app->tex->UnLoad(graphics);
	return ret;
}

bool Player::Update(float dt) 
{
	bool ret = false;
	if (position.y > deathLimit || status == PLAYER_DEATH) 
	{
		if (!dead) 
		{
			ResetStates();
			velocity.x = 0;
			death.Reset();
			deathTimer = deathTimerConfig;
			status = PLAYER_DEATH;
			input = false;
			dead = true;
		}
	}
	//Input
	if (input) 
	{
		if (app->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) //Activate godmode
		{ 
			ResetStates();
			status = PLAYER_IDLE;
			godmode = !godmode;
		}
		if (!godmode) 
		{
			if (onGround) 
			{
				ResetStates();

				if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
				{
					status = PLAYER_JUMP;
				}
				else status = PLAYER_IDLE;
			}

			if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			{
				if(!leftColliding) status = PLAYER_BACKWARD;
				if (onGround && app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
				{
					status = PLAYER_JUMP;
					doubleJump = true;
				}
			}
			else if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			{
				if(!rightColliding) status = PLAYER_FORWARD;
				if (onGround && app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
				{
					status = PLAYER_JUMP;
					doubleJump = true;
				}
			}

			if (!onGround)
			{
				velocity.y += gravity;
				if (doubleJump && app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
				{
					doubleJump = false;
					jumpEnable = true;
					status = PLAYER_JUMP;
				}
			}
		}
		else 
		{ //Godmode input
			if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) 
			{
				position.x -= 2;
			}
			else if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) 
			{
				position.x += 3;
			}

			if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) 
			{
				position.y -= 2;
			}
			else if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) 
			{
				position.y += 2;
			}
		}

	}
	//Status
	switch (status)
	{
	case PLAYER_IDLE:
		velocity.x = 0;
		currentAnimation = &idle;
		break;
	case PLAYER_FORWARD:
		velocity.x = speed;
		flip = false;
		if (onGround)currentAnimation = &walk;
		else currentAnimation = &jump;
		break;
	case PLAYER_BACKWARD:
		velocity.x = -speed;
		flip = true;
		if(onGround)currentAnimation = &walk;
		else currentAnimation = &jump;
		break;
	case PLAYER_JUMP:
		if (jumpEnable == true) 
		{
			jumpEnable = false;
			currentAnimation = &jump;
			velocity.y = -3;
			jump.Reset();
			// Sound
			app->audio->PlayFx(jumpFx);
		}
		break;
	case PLAYER_DEATH:
		//Death animation
		currentAnimation = &death;
		if (deathTimer <= 0) 
		{
			currentAnimation = &idle;
			dead = false;
			life = 100;
			input = true;
			position = initialPos; //Return to start
			status = PLAYER_IN_AIR;
		}
		else deathTimer -= 0.1f;
		break;

	default:
		break;
	}

	//Change position from velocity
	position.x += velocity.x;
	position.y += velocity.y;

	//Collider position
	colPlayer->SetPos(position.x + 13, position.y + 17);
	colPlayerWalls->SetPos(position.x + 11, position.y + 18);

	rCollider.x = position.x + 13; rCollider.y = position.y + 17;

	//Function to draw the player
	ret = Draw(dt);
	onGround = false;
	rightColliding = false;
	leftColliding = false;
	return true;
}

bool Player::Draw(float dt)
{
	bool ret = false;
	r = currentAnimation->GetCurrentFrame(dt);
	if (graphics != nullptr) 
	{
		ret = app->render->DrawTexture(graphics, position.x, position.y, &r, 1, 1.0f, 0.0f, INT_MAX, INT_MAX, flip);
	}
	else LOG("No available graphics to draw.");

	r.x = position.x;
	r.y = position.y;
	return ret;
}

bool Player::OnCollision(Collider* c1, Collider* c2) 
{
	bool ret = false;
	if (!godmode)
	{
		if (c1 == colPlayer && c2->type == COLLIDER_GROUND)
		{
			if (c2->rect.y > c1->rect.y + c1->rect.h - 5)
			{
				position.y = c2->rect.y - c2->rect.h * 2;
				velocity.y = 0;
				onGround = true;
			}
			else if (c2->rect.y + c2->rect.h < c1->rect.y + 5)
			{
				velocity.y = 0;
				position.y = c2->rect.y;
			}
			ret = true;
		}
		if (c1 == colPlayerWalls && c2->type == COLLIDER_GROUND) 
		{
			if (c2->rect.x > c1->rect.x + c1->rect.w - 5 && c2->rect.y < c1->rect.y + c1->rect.h)
			{
				//Collider in the right
				position.x = c2->rect.x - c2->rect.w - 5;
				velocity.x = 0;
				rightColliding = true;
			}
			else if (c2->rect.x + c2->rect.w < c1->rect.x + 5 && c2->rect.y < c1->rect.y + c1->rect.h)
			{
				//Collider on the left
				position.x = c2->rect.x + 5;
				velocity.x = 0;
				leftColliding = true;
			}
			else
			{
				leftColliding = false;
				rightColliding = false;
			}
			ret = true;
		}
		
	}
	else ret = true;
	return ret;
}

bool Player::EnablePlayer() //Enable function for changing scene
{
	bool ret = false;
	active = true;
	Start();
	return ret;
}

bool Player::DisablePlayer() //Disable function for changing scene
{
	LOG("Unloading player");
	active = false;

	return true;
}

bool Player::ResetStates() //Reset all states before checking input
{
	velocity.x = 0;
	velocity.y = 0;
	jumpEnable = true;
	doubleJump = true;

	app->scene->loaded = false;

	return true;
}

bool Player::SaveState(pugi::xml_node& data) const 
{
	pugi::xml_node ply = data.append_child("player");

	ply.append_attribute("x") = position.x;
	ply.append_attribute("y") = position.y;

	return true;
}

bool Player::LoadState(pugi::xml_node& data)
{
	position.x = data.child("player").attribute("x").as_int();
	position.y = data.child("player").attribute("y").as_int();

	colPlayer->SetPos(position.x + 13, position.y + 17);
	colPlayerWalls->SetPos(position.x + 11, position.y + 18);

	rCollider.x = position.x + 13; rCollider.y = position.y + 17;

	r.x = position.x;
	r.y = position.y;

	onGround = false;
	rightColliding = false;
	leftColliding = false;

	return true;
}