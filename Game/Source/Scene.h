#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Point.h"

struct SDL_Texture;
struct SDL_Rect;

enum GameScene
{
	SCENE_NONE,
	SCENE_INTRO,
	SCENE_1,
	SCENE_END
};


class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Change the current scene 
	void ChangeScene(GameScene nextScene);
	void LoadLastSave();

	bool MovePlayer(iPoint pos);

	// Detects collisions
	bool OnCollision(Collider* c1, Collider* c2);

	// Public variables
	bool loaded = false;
	int deathLimit;
	int gravity;

private:

	GameScene currentScene;	
	SDL_Texture* introScreen;
	SDL_Texture* endScreen;
	SDL_Rect* fullscreenRect;

	Collider* endCol = nullptr;

	SString menuAudioPath;
	SString gameAudioPath;
	SString winAudioPath;
	int audioVol;

	SString introTexturePath;
	SString endTexturePath;
	SString mapLevel1;

	//DynArray<iPoint> objectsPosConfig;
	//DynArray<SString> objectsTypeConfig;

	bool ended;
};

#endif // __SCENE_H__