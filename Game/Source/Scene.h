#ifndef __SCENE_H__
#define __SCENE_H__

#include "Module.h"
#include "Point.h"
#include <map>

struct SDL_Texture;
struct SDL_Rect;
class UI;

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

	bool CreateUI();

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

	struct newobj {
		iPoint pos;
		int type;
	};
	List<newobj*> objects;

	bool ended;

	//UI	
	bool menu = true;
	bool pause_menu = false;
	bool quit = false;
	bool fullscreen = false;
	UI* window = nullptr;
	UI* image = nullptr;
	UI* image2 = nullptr;
	UI* startButton = nullptr;
	UI* optionsButton = nullptr;
	UI* creditsButton = nullptr;
	UI* quitButton = nullptr;
	UI* backButton = nullptr;
	UI* resumeButton = nullptr;
	UI* saveButton = nullptr;
	UI* loadButton = nullptr;
	UI* menuButton = nullptr;
	UI* fullscreeenButton = nullptr;
	UI* audioText = nullptr;
	UI* audioSlider = nullptr;
	UI* tick = nullptr;
	UI* coinImage[6];
	int coinOffset = 5;
	UI* lifeBar = nullptr;
	UI* lifeUI = nullptr;
};

#endif // __SCENE_H__