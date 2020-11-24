#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"

struct SDL_Texture;
struct SDL_Rect;

enum GameScene
{
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

	// Detects collisions
	bool OnCollision(Collider* c1, Collider* c2);

	// Public variables
	bool loaded = false;


private:

	GameScene currentScene;	
	SDL_Texture* introScreen;
	SDL_Texture* endScreen;
	SDL_Rect* fullscreenRect;

	Collider* endCol = nullptr;

	SString menuAudioPath;
	SString gameAudioPath;
	SString winAudioPath;

	SString introTexturePath;
	SString endTexturePath;
	SString level1Map;
	int audioVol;

	bool ended;
};

#endif // __SCENE_H__