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

	bool PauseMenu();
	bool GameUI();
	bool MenuUI();

	void OnClick(UI* interaction);

	// Public variables
	bool loaded = false;
	int deathLimit;
	int gravity;
	bool pauseMenu = false;

private:

	GameScene currentScene;	
	SDL_Texture* introScreen;
	SDL_Texture* endScreen;
	SDL_Rect* fullscreenRect;

	Collider* endCol = nullptr;
	bool exitGame = false;

	SString menuAudioPath;
	SString gameAudioPath;
	SString winAudioPath;

	SString introTexturePath;
	SString endTexturePath;
	SString mapLevel1;

	struct newobj 
	{
		iPoint pos;
		int type;
	};
	List<newobj*> objects;

	bool ended;

	//UI	
	bool optionsMenu = false;
	bool fullscreen = false;
	UI* image1 = nullptr;
	UI* image2 = nullptr;
	UI* button1 = nullptr;
	UI* text1 = nullptr;
	UI* button2 = nullptr;
	UI* text2 = nullptr;
	UI* button3 = nullptr;
	UI* text3 = nullptr;
	UI* button4 = nullptr;
	UI* text4 = nullptr;
	UI* button5 = nullptr;
	UI* text5 = nullptr;
};

#endif // __SCENE_H__