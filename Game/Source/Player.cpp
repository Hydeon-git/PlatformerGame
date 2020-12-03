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

	death.PushBack({ 0, 64, 32, 32 });
	death.PushBack({ 32, 64, 32, 32 });
	death.PushBack({ 64, 64, 32, 32 });
	death.PushBack({ 96, 64, 32, 32 });
	death.PushBack({ 128, 64, 32, 32 });
	death.PushBack({ 160, 64, 32, 32 });
	death.speed = 0.2f;
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
	jumpForce = config.child("propierties").attribute("jumpForce").as_float();
	gravity = config.child("propierties").attribute("gravity").as_float();
	deathTimerConfig = config.child("death").attribute("time").as_float();
	initialPos.x = config.child("initialPos1").attribute("x").as_int();
	initialPos.y = config.child("initialPos1").attribute("y").as_int();
	jumpFx = app->audio->LoadFx(config.child("sounds").attribute("jumpFx").as_string());
	shotFx = app->audio->LoadFx(config.child("sounds").attribute("shotFx").as_string());
	wallHitFx = app->audio->LoadFx(config.child("sounds").attribute("wallHitFx").as_string());
	deathFx = app->audio->LoadFx(config.child("sounds").attribute("deathFx").as_string());

	//Bullet
	bulletTexPath = config.child("bullets").attribute("tex").as_string();
	bulletSpeed = config.child("bullets").attribute("speed").as_float();
	bulletDamage = config.child("bullets").attribute("damage").as_int();

	deathLimit = app->scene->deathLimit;
	
	return ret;
}

bool Player::Start()
{
	bool ret = true;

	//Loading assets and propierties from config file
	position.x = initialPos.x;
	position.y = initialPos.y;
	velocity.SetToZero();
	onGround = true;

	if(graphics == nullptr) graphics = app->tex->Load(texPath.GetString());
	flip = false;

	if(bulletGraphics == nullptr) 
		bulletGraphics = app->tex->Load(bulletTexPath.GetString());
	gunOffset.x = 28;
	gunOffset.y = 23;

	LOG("Creating player colliders");
	rCollider = { positionPixelPerfect.x+13, positionPixelPerfect.y+17, 6, 15 };
	colPlayer = app->collision->AddCollider(rCollider, COLLIDER_PLAYER, this);
	colPlayerWalls = app->collision->AddCollider({ positionPixelPerfect.x+11, positionPixelPerfect.y+18, 10, 13 }, COLLIDER_PLAYER, this);

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

bool Player::PreUpdate() 
{
	for (uint i = 0; i < bullets.Count(); ++i)
	{
		// Remove all bullets scheduled for deletion
		if (bullets[i] != nullptr && bullets[i]->toDelete == true)
		{
			delete bullets[i];
			bullets[i] = nullptr;
		}
	}
	return true;
}

bool Player::Update(float dt) 
{
	bool ret = false;
	if (positionPixelPerfect.y > deathLimit || life <= 0) 
	{
		if (!dead) 
		{
			ResetStates();
			velocity.x = 0;
			death.Reset();
			deathTimer = deathTimerConfig;
			status = PLAYER_DEATH;

			app->audio->PlayFx(deathFx);

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

			if (app->input->GetKey(SDL_SCANCODE_K) == KEY_DOWN)
			{
				life = 0;
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

			if (app->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN) 
			{
				fPoint spawnPoint;
				spawnPoint.x = (flip) == false ? positionPixelPerfect.x + gunOffset.x : positionPixelPerfect.x;
				spawnPoint.y = positionPixelPerfect.y + gunOffset.y;
				bullets.Add(new Bullet(bulletGraphics, bulletSpeed, spawnPoint, flip, wallHitFx));

				// Sound
				app->audio->PlayFx(shotFx);
			}

			if (!onGround)
			{
				velocity.y += gravity * dt;
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
			position.y -= 1;
			velocity.y = -jumpForce;
			jump.Reset();
			// Sound
			app->audio->PlayFx(jumpFx);
		}
		break;
	case PLAYER_DEATH:
		//Death animation
		currentAnimation = &death;
		if (death.Finished()) 
		{
			app->scene->LoadLastSave();
		}
		break;

	default:
		break;
	}

	//Change position from velocity
	position.x += (velocity.x * dt);
	position.y += (velocity.y * dt);

	positionPixelPerfect.x = round(position.x);
	positionPixelPerfect.y = round(position.y);

	//Collider position
	colPlayer->SetPos(positionPixelPerfect.x + 13, positionPixelPerfect.y + 17);
	colPlayerWalls->SetPos(positionPixelPerfect.x + 11, positionPixelPerfect.y + 18);

	rCollider.x = positionPixelPerfect.x + 13; rCollider.y = positionPixelPerfect.y + 17;

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
		ret = app->render->DrawTexture(graphics, positionPixelPerfect.x, positionPixelPerfect.y, &r, 1, 1.0f, 0.0f, INT_MAX, INT_MAX, flip);
	}
	else LOG("No available graphics to draw.");

	//Update bullets
	for (uint i = 0; i < bullets.Count(); ++i)
	{
		if (bullets[i] != nullptr)
		{
			bullets[i]->Update(dt);
		}
	}

	r.x = positionPixelPerfect.x;
	r.y = positionPixelPerfect.y;
	return ret;
}

bool Player::OnCollision(Collider* c1, Collider* c2) 
{
	bool ret = false;
	if (!godmode)
	{
		if (c1 == colPlayer && c2->type == COLLIDER_GROUND)
		{
			if (c2->rect.y >= c1->rect.y + c1->rect.h - 5)
			{
				if(velocity.y != 0) position.y = c2->rect.y - c2->rect.h * 2;
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
			else if (c2->rect.x + c2->rect.w < c1->rect.x + 6 && c2->rect.y < c1->rect.y + c1->rect.h)
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
	ret = Start();
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

	ply.append_attribute("x") = positionPixelPerfect.x;
	ply.append_attribute("y") = positionPixelPerfect.y;

	ply.append_attribute("life") = life;

	return true;
}

bool Player::LoadState(pugi::xml_node& data)
{
	pugi::xml_node ply = data.child("player");

	position.x = ply.attribute("x").as_int();
	position.y = ply.attribute("y").as_int();

	life = ply.attribute("life").as_int();

	positionPixelPerfect.x = round(position.x);
	positionPixelPerfect.y = round(position.y);

	colPlayer->SetPos(positionPixelPerfect.x + 13, positionPixelPerfect.y + 17);
	colPlayerWalls->SetPos(positionPixelPerfect.x + 11, positionPixelPerfect.y + 18);

	rCollider.x = positionPixelPerfect.x + 13; rCollider.y = positionPixelPerfect.y + 17;

	r.x = positionPixelPerfect.x;
	r.y = positionPixelPerfect.y;

	onGround = false;
	rightColliding = false;
	leftColliding = false;
	dead = false;
	input = true;

	status = PLAYER_IDLE;

	return true;
}

//--------------BULLETS---------------

Bullet::Bullet(SDL_Texture* graph, float sp, fPoint pos, bool fp, int sound)
{
	graphics = graph;
	position = pos;
	flip = fp;
	wallHitFx = sound;

	if (flip) speed = -sp;
	else speed = sp;

	rect = { 0, 0, 5, 4 };
	bulletCollider = app->collision->AddCollider(rect, COLLIDER_BULLET, this);
	bulletCollider->SetPos(position.x, position.y);
}

Bullet::~Bullet()
{
	bulletCollider->toDelete = true;
}

bool Bullet::Update(float dt)
{
	bool ret = false;

	position.x += speed * dt;

	bulletCollider->SetPos(position.x, position.y);

	if (graphics != nullptr)
	{
		ret = app->render->DrawTexture(graphics, position.x, position.y, &rect, 1, 1.0f, 0.0f, INT_MAX, INT_MAX, flip);
	}
	else LOG("No available graphics to draw.");

	return ret;
}

bool Bullet::OnCollision(Collider* c1, Collider* c2)
{
	bool ret = false;

	if (c1 == bulletCollider && c2->type == COLLIDER_GROUND) 
	{
		//Play sound
		app->audio->PlayFx(wallHitFx);
	}

	toDelete = true;
	return ret;
}