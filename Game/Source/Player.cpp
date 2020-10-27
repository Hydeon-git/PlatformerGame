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

	texPath = config.child("path").attribute("tex").as_string();
	life = config.child("propierties").attribute("life").as_int();
	speed = config.child("propierties").attribute("speed").as_float();
	gravity = config.child("propierties").attribute("gravity").as_float();
	deathTimer_config = config.child("death").attribute("time").as_float();
	deathLimit = config.child("death").attribute("height").as_int();
	initialPos.x = config.child("initialPos1").attribute("x").as_int();
	initialPos.y = config.child("initialPos1").attribute("y").as_int();
	return ret;
}

bool Player::Start()
{
	bool ret = true;
	//Loading assets and propierties from config file
	position.x = initialPos.x;
	position.y = initialPos.y;
	graphics = app->tex->Load(texPath.GetString());
	LOG("creating player colliders");
	r_collider = { position.x+11, position.y+17, 10, 15 };
	colPlayer = app->collision->AddCollider(r_collider, COLLIDER_PLAYER);
	colPlayerWalls = app->collision->AddCollider({position.x, position.y+14, 12, 2 }, COLLIDER_PLAYER);
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

bool Player::Disable() //Disable function for changing scene
{ 
	bool ret = false;
	ret = app->tex->UnLoad(graphics);
	ret = app->audio->CleanUp();
	return ret;
}

bool Player::ResetStates() //Reset all states before checking input
{ 
	velocity.y = 0;
	jumpEnable = true;

	app->scene->loaded = false;

	return true;
}

bool Player::Update(float dt) 
{
	bool ret = false;
	if (position.y > deathLimit || status == PLAYER_DEATH) {
		if (!dead) {
			velocity.x = 0;
			death.Reset();
			deathTimer = deathTimer_config;
			status = PLAYER_DEATH;
			input = false;
			dead = true;
		}
	}
	//Input
	if (input) {
		if (app->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) //Activate godmode
		{ 
			ResetStates();
			godmode = !godmode;
		}
		if (!godmode) 
		{
			if (OnGround()) 
			{
				ResetStates();

				if (app->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
					status = PLAYER_JUMP;

				else status = PLAYER_IDLE;
			}
			else 
			{
				velocity.y += gravity;
			}

			if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			{
				status = PLAYER_BACKWARD;
				if (app->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
					status = PLAYER_JUMP;
			}
			else if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			{
				status = PLAYER_FORWARD;
				if (app->input->GetKey(SDL_SCANCODE_W) == KEY_DOWN)
					status = PLAYER_JUMP;
			}
			WallCollision(); //Detect horizontal collision
		}
		else 
		{ //Godmode input
			if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) 
			{
				position.x -= 1;
			}
			else if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) 
			{
				position.x += 1;
			}
			else if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) 
			{
				position.y -= 1;
			}
			else if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) 
			{
				position.y += 1;
			}
		}

	}
	//Status
	switch (status)
	{
	case PLAYER_IDLE:
		velocity.x = 0;
		current_animation = &idle;
		break;
	case PLAYER_FORWARD:
		velocity.x = speed;
		flip = false;
		current_animation = &walk;
		break;
	case PLAYER_BACKWARD:
		velocity.x = -speed;
		flip = true;
		current_animation = &walk;
		break;
	case PLAYER_JUMP:
		if (jumpEnable == true) {
			jumpEnable = false;
			current_animation = &jump;
			velocity.y = -3;
			jump.Reset();
			// Sound
		}
		break;
	case PLAYER_DEATH:
		//Death animation
		current_animation = &death;
		if (deathTimer <= 0) {
			current_animation = &idle;
			dead = false;
			life = 100;
			input = true;
			position = initialPos;//Return to start
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
	if (velocity.y > 0) colPlayer->SetPos(position.x + 11, position.y + 17);
	else colPlayer->SetPos(position.x + 11, position.y + 17);

	if (velocity.x > 0) 	colPlayerWalls->SetPos(position.x + 11, position.y + 15);
	else if (velocity.x < 0) 	colPlayerWalls->SetPos(position.x + 9, position.y + 15);
	else colPlayerWalls->SetPos(position.x + 10, position.y + 15);

	r_collider.x = position.x + 11; r_collider.y = position.y + 17;

	//Function to draw the player
	ret = Draw(dt);

	return true;
}

bool Player::Draw(float dt)
{
	bool ret = false;
	r = current_animation->GetCurrentFrame(dt);
	if (graphics != nullptr) {
		ret = app->render->DrawTexture(graphics, position.x, position.y, &r, 1.0f, 0.0f, INT_MAX, INT_MAX, flip);
	}
	else LOG("No available graphics to draw.");

	r.x = position.x;
	r.y = position.y;
	return ret;
}

bool Player::OnGround() {

	bool ret = false;

	for (int i = 0; i < app->map->groundCol.count(); i++) {
		ret = colPlayer->CheckCollision(app->map->groundCol.At(i)->data->rect);
		if (ret) {
			if (velocity.y > 0) {
				position.y = app->map->groundCol.At(i)->data->rect.y - 32;
				velocity.x = 0;
			}
			else if (velocity.y < 0) {
				position.y = app->map->groundCol.At(i)->data->rect.y;
				velocity.x = 0;
			}
			return ret;
		}
	}

	return ret;
}

bool Player::WallCollision() {
	bool ret = false;

	for (int i = 0; i < app->map->groundCol.count(); i++) {
		ret = colPlayerWalls->CheckCollision(app->map->groundCol.At(i)->data->rect);
		if (ret) {
			if (velocity.x > 0) {
				position.x = app->map->groundCol.At(i)->data->rect.x - 23;
			}
			else if (velocity.x < 0) {
				position.x = app->map->groundCol.At(i)->data->rect.x + 8;
			}
			break;
		}
	}
	return ret;
}

bool Player::SaveState(pugi::xml_node& data) const {
	pugi::xml_node ply = data.append_child("player");

	ply.append_attribute("x") = position.x;
	ply.append_attribute("y") = position.y;

	return true;
}

bool Player::LoadState(pugi::xml_node& data)
{
	SDL_Delay(1000);
	position.x = data.child("player").attribute("x").as_int();
	position.y = data.child("player").attribute("y").as_int();

	return true;
}