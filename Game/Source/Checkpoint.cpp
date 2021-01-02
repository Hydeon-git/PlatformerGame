#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Scene.h"
#include "Player.h"
#include "Checkpoint.h"

Checkpoint::Checkpoint() : Entity(EntityType::CHECKPOINT)
{
	name.Create("checkpoint");

	//animations
	unchecked.PushBack({ 0, 0, 32, 32 });
	unchecked.speed = 0.2f;
	unchecked.loop = false;

	checking.PushBack({ 32, 0, 32, 32 });
	checking.PushBack({ 64, 0, 32, 32 });
	checking.PushBack({ 96, 0, 32, 32 });
	checking.speed = 0.2f;

	checked.PushBack({ 128, 0, 32, 32 });
	checked.speed = 0.2f;
	checked.loop = false;
}

// Destructor
Checkpoint::~Checkpoint()
{}

bool Checkpoint::Awake(pugi::xml_node& config)
{
	bool ret = true;
	LOG("Loading checkpoint from config_file");

	texPath = config.child("texPath").attribute("tex").as_string();
	position.x = config.child("position").attribute("x").as_int();
	position.y = config.child("position").attribute("y").as_int();
	checkTime = config.child("properties").attribute("time").as_float();
	checkpointFx = app->audio->LoadFx(config.child("sounds").attribute("checkpointFx").as_string());

	return ret;
}

bool Checkpoint::Load()
{
	bool ret = true;
	LOG("Creating checkpoint");

	//Loading assets and properties from config file
	if (graphics == nullptr) graphics = app->tex->Load(texPath.GetString());

	rCollider = { (int)position.x + 8, (int)position.y, 16, 32 };
	if (colCheckpoint == nullptr) colCheckpoint = app->collision->AddCollider(rCollider, COLLIDER_CHECKPOINT, this);
	
	unchecked.Reset();
	checking.Reset();
	checked.Reset();

	checkTimer = checkTime;

	status = CHECKPOINT_UNCHECKED;

	return ret;
}

// Unload assets
bool Checkpoint::CleanUp()
{
	bool ret = true;
	LOG("Unloading checkpoint");
	if (colCheckpoint != nullptr)
	{
		colCheckpoint->toDelete = true;
		colCheckpoint = nullptr;
	}
	if (graphics != nullptr)
	{
		ret = app->tex->UnLoad(graphics);
		graphics = nullptr;
	}
	return ret;
}

bool Checkpoint::Draw(float dt)
{
	bool ret = false;

	switch (status)
	{
	case CHECKPOINT_UNCHECKED:
		currentAnimation = &unchecked;
		break;
	case CHECKPOINT_CHECKING:
		currentAnimation = &checking;
		if (checkTimer <= 0)
		{
			status = CHECKPOINT_CHECKED;
			check = true;
		}
		else checkTimer -= dt;
		break;
	case CHECKPOINT_CHECKED:
		currentAnimation = &checked;
		break;
	default:
		break;
	}

	if (!app->scene->pauseMenu)
		r = currentAnimation->GetCurrentFrame(dt);
	else
		r = currentAnimation->GetCurrentFrame(0);

	if (graphics != nullptr)
	{
		ret = app->render->DrawTexture(graphics, position.x, position.y, &r, 1, 1.0f, 0.0f, INT_MAX, INT_MAX);
	}
	else LOG("No available graphics to draw.");

	r.x = position.x;
	r.y = position.y;
	return ret;
}

bool Checkpoint::OnCollision(Collider* c1, Collider* c2)
{
	bool ret = false;

	if (c1 == colCheckpoint && c2->type == COLLIDER_PLAYER && status == CHECKPOINT_UNCHECKED)
	{
		//Sound
		app->audio->PlayFx(checkpointFx);
		app->SaveGameRequest();
		app->scene->player->checkpoint += 1;
		status = CHECKPOINT_CHECKING;

		ret = true;
	}

	return ret;
}