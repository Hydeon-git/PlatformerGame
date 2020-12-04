#include "App.h"
#include "Input.h"
#include "Render.h"
#include "Collision.h"
#include "Module.h"
#include "Log.h"


Collision::Collision()
{
	name.Create("collision");
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
		colliders[i] = nullptr;

	matrix[COLLIDER_GROUND][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_GROUND][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_GROUND][COLLIDER_GROUND] = false;
	matrix[COLLIDER_GROUND][COLLIDER_BULLET] = true;
	matrix[COLLIDER_GROUND][COLLIDER_OBJECT] = true;
	matrix[COLLIDER_GROUND][COLLIDER_CHECKPOINT] = false;
	matrix[COLLIDER_GROUND][COLLIDER_END] = false;

	matrix[COLLIDER_PLAYER][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_GROUND] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_BULLET] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_OBJECT] = false;
	matrix[COLLIDER_PLAYER][COLLIDER_CHECKPOINT] = true;
	matrix[COLLIDER_PLAYER][COLLIDER_END] = true;
	
	matrix[COLLIDER_ENEMY][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_ENEMY][COLLIDER_GROUND] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_BULLET] = true;
	matrix[COLLIDER_ENEMY][COLLIDER_OBJECT] = false;
	matrix[COLLIDER_ENEMY][COLLIDER_CHECKPOINT] = false;
	matrix[COLLIDER_ENEMY][COLLIDER_END] = false;

	matrix[COLLIDER_BULLET][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_BULLET][COLLIDER_ENEMY] = true;
	matrix[COLLIDER_BULLET][COLLIDER_GROUND] = true;
	matrix[COLLIDER_BULLET][COLLIDER_BULLET] = false;
	matrix[COLLIDER_BULLET][COLLIDER_OBJECT] = false;
	matrix[COLLIDER_BULLET][COLLIDER_CHECKPOINT] = false;
	matrix[COLLIDER_BULLET][COLLIDER_END] = false;

	matrix[COLLIDER_CHECKPOINT][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_CHECKPOINT][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_CHECKPOINT][COLLIDER_GROUND] = false;
	matrix[COLLIDER_CHECKPOINT][COLLIDER_BULLET] = false;
	matrix[COLLIDER_CHECKPOINT][COLLIDER_OBJECT] = false;
	matrix[COLLIDER_CHECKPOINT][COLLIDER_CHECKPOINT] = false;
	matrix[COLLIDER_CHECKPOINT][COLLIDER_END] = false;

	matrix[COLLIDER_END][COLLIDER_PLAYER] = true;
	matrix[COLLIDER_END][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_END][COLLIDER_GROUND] = false;
	matrix[COLLIDER_END][COLLIDER_BULLET] = false;
	matrix[COLLIDER_END][COLLIDER_OBJECT] = false;
	matrix[COLLIDER_END][COLLIDER_CHECKPOINT] = false;
	matrix[COLLIDER_END][COLLIDER_END] = false;

	matrix[COLLIDER_OBJECT][COLLIDER_PLAYER] = false;
	matrix[COLLIDER_OBJECT][COLLIDER_ENEMY] = false;
	matrix[COLLIDER_OBJECT][COLLIDER_GROUND] = true;
	matrix[COLLIDER_OBJECT][COLLIDER_BULLET] = false;
	matrix[COLLIDER_OBJECT][COLLIDER_OBJECT] = false;
	matrix[COLLIDER_OBJECT][COLLIDER_CHECKPOINT] = false;
	matrix[COLLIDER_OBJECT][COLLIDER_END] = false;
	
}

// Destructor
Collision::~Collision()
{}

bool Collision::PreUpdate()
{
	// Remove all colliders scheduled for deletion
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] != nullptr && colliders[i]->toDelete == true)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}
	
	// Calculate collisions
	Collider* c1;
	Collider* c2;
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		// skip empty colliders
		if (colliders[i] == nullptr)
			continue;
		c1 = colliders[i];

		// avoid checking collisions already checked
		for (uint k = i + 1; k < MAX_COLLIDERS; ++k)
		{
			// skip empty colliders
			if (colliders[k] == nullptr)
				continue;
			c2 = colliders[k];

			if (c1->CheckCollision(c2->rect) == true)
			{
				if (matrix[c1->type][c2->type] && c1->callback)
					c1->callback->OnCollision(c1, c2);
				if (matrix[c2->type][c1->type] && c2->callback)
					c2->callback->OnCollision(c2, c1);
			}
		}
	}
	
	return true;
}

// Called before render is available
bool Collision::Update(float dt)
{

	if (app->debug) DebugDraw();

	return true;
}

void Collision::DebugDraw()
{
	Uint8 alpha = 80;
	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
			continue;

		switch (colliders[i]->type)
		{
		case COLLIDER_NONE: // white
			app->render->DrawRectangle(colliders[i]->rect, 255, 255, 255, alpha);
			break;		
		case COLLIDER_PLAYER: // green
			app->render->DrawRectangle(colliders[i]->rect, 0, 255, 0, alpha);
			break;
		case COLLIDER_ENEMY: // red
			app->render->DrawRectangle(colliders[i]->rect, 255, 0, 0, alpha);
			break;
		case COLLIDER_BULLET: // violet
			app->render->DrawRectangle(colliders[i]->rect, 238, 130, 238, alpha);
			break;
		case COLLIDER_CHECKPOINT: // blue
			app->render->DrawRectangle(colliders[i]->rect, 0, 0, 255, alpha);
			break;
		case COLLIDER_GROUND: // brown
			app->render->DrawRectangle(colliders[i]->rect, 139, 69, 19, alpha);
			break;
		case COLLIDER_END: // oranje
			app->render->DrawRectangle(colliders[i]->rect, 255, 140, 0, alpha);
			break;
		}
	}
}

// Called before quitting
bool Collision::CleanUp()
{
	LOG("Freeing all colliders");

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] != nullptr)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	return true;
}

Collider* Collision::AddCollider(SDL_Rect rect, ColliderType type, Module* callback)
{
	Collider* ret = nullptr;

	for (uint i = 0; i < MAX_COLLIDERS; ++i)
	{
		if (colliders[i] == nullptr)
		{
			ret = colliders[i] = new Collider(rect, type, callback);
			break;
		}
	}

	return ret;
}

// -----------------------------------------------------

bool Collider::CheckCollision(const SDL_Rect& r) const
{
	bool ret = false;

	if (rect.x <= r.x + r.w && rect.x + rect.w >= r.x && rect.y <= r.y + r.h && rect.h + rect.y >= r.y) ret = true;

	return ret;
}