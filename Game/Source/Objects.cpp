#include "App.h"
#include "Render.h"
#include "Input.h"
#include "Scene.h"
#include "Map.h"
#include "Textures.h"
#include "Objects.h"


#define SPAWN_MARGIN 50


Objects::Objects() : Module() {}
Objects::Objects(fPoint objectPos, ObjectType tp) : Module()
{
	position = objectPos;
	type = tp;

	dRect = { 0,0,36,36 };
	dCol = app->collision->AddCollider(dRect, COLLIDER_ENEMY, this);
	dCol->SetPos(objectPos.x, objectPos.y);
}
Objects::~Objects() 
{
	dCol->toDelete = true;
}

bool Objects::Start()
{
	// Diamond
	if (diamondTex == nullptr) diamondTex = app->tex->Load(diamondTexPath.GetString());


	return true;
}

bool Objects::Update(float dt)
{
	bool ret = false;
	int diamond;
	for (uint i = 0; i < objects.Count(); i++)
	{
		if (objects[i] != nullptr)
		{
			diamond = i;
		}
	}

	if (diamondTex != nullptr)
	{
		//ret = app->render->DrawTexture(diamondTex, objects[diamond]->position.x, objects[diamond]->position.y, &objects[diamond]->dCol, 1, 1.0f, 0.0f, INT_MAX, INT_MAX);
	}
}

// Called before quitting
bool Objects::CleanUp()
{
	for (uint i = 0; i < objects.Count(); ++i)
	{
		if (objects[i] != nullptr)
		{
			delete objects[i];
			objects[i] = nullptr;
		}
	}
	return true;
}

bool Objects::OnCollision(Collider* c1, Collider* c2)
{
	for (uint i = 0; i < objects.Count(); ++i)
	{

	}
	return false;
}