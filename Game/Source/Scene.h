#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"

struct SDL_Texture;

enum GameScene
{
	SceneIntro,
	Scene1,
	GameOver
};


class Scene : public Module
{
public:

	Scene();

	// Destructor
	virtual ~Scene();

	// Called before render is available
	bool Awake();

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

	//Public variables
	bool loaded = false;

private:

	GameScene currentScene;
	void Scene::ChangeScene(GameScene nextScene);
		
	SDL_Texture* introScreen;
};

#endif // __SCENE_H__