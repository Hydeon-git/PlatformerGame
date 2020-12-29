#ifndef __j1GUI_H__
#define __j1GUI_H__

#include "Module.h"
#include "Input.h"

#define CURSOR_WIDTH 2

enum class Type
{
	NONE,
	WINDOW,
	BUTTON,
	IMAGE,
	CHECKBOX,
	TEXT,
	UNKNOWN
};

class UI : public Module
{
public:
	UI(Type s_type, SDL_Rect r, UI* p, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~UI() {}

	// Called when before render is available
	virtual bool Awake(pugi::xml_node&) { return true; }

	// Call before first frame
	virtual bool Start() { return true; }

	// Called before all Updates
	virtual bool PreUpdate();

	// Called after all Updates
	virtual bool PostUpdate();

	// Called before quitting
	virtual bool CleanUp() { return true; }

	bool Move();

	SDL_Rect GetScreenRect();
	SDL_Rect GetLocalRect();
	iPoint GetScreenPos();
	iPoint GetScreenToWorldPos();
	iPoint GetLocalPos();
	void SetLocalPos(iPoint pos);
	bool CheckMouse();
	bool CheckFocusable() { return focusable; }
	SDL_Rect GetDragArea() { return dragArea; }
	SDL_Rect Check_Printable_Rect(SDL_Rect sprite, iPoint& dif_sprite);

public:
	SDL_Rect spriteOver;
	SDL_Rect quad;
	bool debug;
	bool focus;
	Module* listener;
	Type type;

private:
	SDL_Rect screenRect;
	SDL_Rect localRect;
	SDL_Rect maskRect;
	UI* parent;
	bool drageable;
	bool focusable;
	SDL_Rect dragArea;
};
class ImageUI : public UI
{
public:
	ImageUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~ImageUI() {}

	// Called before all Updates
	bool PreUpdate();

	// Called after all Updates
	bool PostUpdate();

	fPoint GetDragPositionNormalized();

public:
	iPoint dragPosition0;
	iPoint dragPosition1;
};
class WindowUI :public UI
{
public:
	WindowUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~WindowUI() {}

	// Called after all Updates
	bool PostUpdate();
};
class TextUI : public UI
{
public:
	TextUI(Type type, UI* p, SDL_Rect r, SString str, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~TextUI() {}

	// Called after all Updates
	bool PostUpdate();

public:

	SString stri;
};
class ButtonUI : public UI
{
public:
	ButtonUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriten2, SDL_Rect spriten3, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~ButtonUI() {}

	// Called when before render is available
	bool Awake(pugi::xml_node&) { return true; };

	// Call before first frame
	bool Start() { return true; };

	// Called before all Updates
	bool PreUpdate();

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp() { return true; };

public:
	SDL_Rect spritePushed;
	SDL_Rect spriteNormal;
	bool pushed;
	bool isMouseOver;
};
// ---------------------------------------------------
class ModuleGUI : public Module
{
public:
	ModuleGUI();

	// Destructor
	virtual ~ModuleGUI();

	// Called when before render is available
	bool Awake(pugi::xml_node&);

	// Call before first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called after all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Gui creation functions
	UI* CreateUIElement(Type type, UI* p, SDL_Rect r, SDL_Rect sprite = { 0,0,0,0 }, SString str = "", SDL_Rect spritePushed = { 0,0,0,0 }, SDL_Rect spriteNormal = { 0,0,0,0 }, bool drageable = false,
		SDL_Rect dragArea = { 0,0,0,0 }, Module* s_listener = nullptr);
	bool DeleteUIElement(UI*);

	void ChangeDebug();

	void ChangeFocus();

	void DeleteFocus();

	const SDL_Texture* GetAtlas() const;

	void ClearUI();

private:
	List <UI*> uiElements;
	SDL_Texture* atlas;
	SString atlasFileName;

public:
	//Audio
	int clickSfx;

};

#endif // __j1GUI_H__