#include "App.h"
#include "Log.h"
#include "Render.h"
#include "Textures.h"
#include "Objects.h"
#include "Scene.h"
#include "Player.h"
#include "Scene.h"

Objects::Objects() : Module() 
{
	name.Create("objects");
}

Objects::~Objects() {}

bool Objects::Awake(pugi::xml_node& config)
{
	diamondTexPath = config.child("diamond").attribute("tex").as_string();
	healthPotionTexPath = config.child("healthPotion").attribute("tex").as_string();
	
	diamondFx = app->audio->LoadFx(config.child("diamond").attribute("fx").as_string());
	healthPotionFx = app->audio->LoadFx(config.child("healthPotion").attribute("fx").as_string());

	return true;
}

bool Objects::Start()
{
	diamondTex = app->tex->Load(diamondTexPath.GetString());
	healthPotionTex = app->tex->Load(healthPotionTexPath.GetString());
	return true;
}

bool Objects::Update(float dt)
{
	bool ret = true;
	for (uint i = 0; i < objects.Count(); i++)
	{
		if (objects[i] != nullptr)
		{
			ret = objects[i]->Draw(dt);
		}
	}
	return ret;
}

Object* Objects::CreateObject(iPoint pos, ObjectType type)
{
	Object* newObj = nullptr;
	switch (type)
	{
	case NO_TYPE:
		newObj = new Object(pos, type, nullptr);
		break;
	case DIAMOND:
		newObj = new Object(pos, type, diamondTex);
		break;
	case HEALTH_POTION:
		newObj = new Object(pos, type, healthPotionTex);
		break;
	default:
		break;
	}
	objects.Add(newObj);
	return newObj;
}

// Called before quitting
bool Objects::CleanUp()
{
	DeleteObjects();

	if (diamondTex != nullptr) app->tex->UnLoad(diamondTex);
	if (healthPotionTex != nullptr) app->tex->UnLoad(healthPotionTex);

	return true;
}

bool Objects::DeleteObjects()
{
	objects.Clear();

	return true;
}

bool Objects::DeleteObject(Object* obj)
{
	if (obj != nullptr)
	{
		int i = objects.Find(obj);
		delete objects[i];
		objects[i] = nullptr;
	}
	
	return true;
}

bool Objects::OnCollision(Collider* c1, Collider* c2)
{
	for (uint i = 0; i < objects.Count(); ++i)
	{
		if ((objects[i] != nullptr) && (c1 == objects[i]->collider) && (!app->scene->player->godmode))
		{
			switch (objects[i]->type)
			{
			case NO_TYPE:
				
				break;
			case DIAMOND:
				LOG("Got a diamond");
				app->scene->player->diamonds++;
				app->scene->GameUI();
				app->audio->PlayFx(diamondFx);
				break;
			case HEALTH_POTION:
				LOG("Got a potion");
				app->scene->player->Heal(10);
				app->scene->GameUI();
				app->audio->PlayFx(healthPotionFx);
				break;
			default:
				break;
			}
			DeleteObject(objects[i]);
		}
	}
	return false;
}

Object::Object(iPoint objectPos, ObjectType tp, SDL_Texture* tex)
{
	LOG("Creating object");
	position = objectPos;
	type = tp;
	texture = tex;

	rect = { 0, 0, 16, 16 };
	collider = app->collision->AddCollider(rect, COLLIDER_OBJECT, app->obj);
	collider->SetPos(position.x, position.y);

	anim.PushBack({ 0,0,16,16 });
	anim.PushBack({ 16,0,16,16 });
	anim.PushBack({ 32,0,16,16 });
	anim.PushBack({ 48,0,16,16 });
	anim.PushBack({ 32,0,16,16 });
	anim.PushBack({ 16,0,16,16 });
	anim.PushBack({ 0,0,16,16 });
	anim.PushBack({ 16,16,16,16 });
	anim.PushBack({ 32,16,16,16 });
	anim.PushBack({ 48,16,16,16 });
	anim.PushBack({ 32,16,16,16 });
	anim.PushBack({ 16,16,16,16 });
	anim.speed = 0.2f;
}

Object::~Object() 
{
	collider->toDelete = true;
}

bool Object::Draw(float dt)
{
	bool ret = false;

	if (!app->scene->pauseMenu)
	{
		rect = anim.GetCurrentFrame(dt);
	}	
	if (texture != nullptr)
	{
		ret = app->render->DrawTexture(texture, position.x, position.y, &rect, 1, 1.0f, 0.0f, INT_MAX, INT_MAX);
	}
	else LOG("No available graphics to draw.");

	return ret;
}