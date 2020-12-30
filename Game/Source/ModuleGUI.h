#ifndef __j1GUI_H__
#define __j1GUI_H__

#include "Module.h"
#include "Input.h"

#define CURSOR_WIDTH 2

enum class UiType
{
	NONE,
	BUTTON,
	IMAGE,
	CHECKBOX,
	TEXT,
	SLIDER,
	UNKNOWN
};

class UI
{
public:
	UI(UiType s_type, SDL_Rect r, UI* p, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~UI() {}

	// Called before all Updates
	virtual bool PreUpdate();

	// Called after all Updates
	virtual bool Draw();

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
	SDL_Rect CheckPrintableRect(SDL_Rect sprite, iPoint& dif_sprite);

public:
	SDL_Rect spriteOver;
	SDL_Rect quad;
	bool debug;
	bool focus;
	Module* listener;
	UiType type;

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
	ImageUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~ImageUI() {}

	// Called after all Updates
	bool Draw();
};
class CheckboxUI : public UI
{
public:
	CheckboxUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriten2, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~CheckboxUI() {}

	// Called before all Updates
	bool PreUpdate();

	// Called after all Updates
	bool Draw();
private:
	SDL_Rect spriteTick;
	SDL_Rect base;
	bool isMouseOver;
	bool state = false;
};
class TextUI : public UI
{
public:
	TextUI(UiType type, UI* p, SDL_Rect r, float size, SString str, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~TextUI() {}

	// Called after all Updates
	bool Draw();

public:
	float size;
	SString stri;
};
class ButtonUI : public UI
{
public:
	ButtonUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriten2, SDL_Rect spriten3, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~ButtonUI() {}

	// Called when before render is available
	bool Awake(pugi::xml_node&) { return true; };

	// Call before first frame
	bool Start() { return true; };

	// Called before all Updates
	bool PreUpdate();

	// Called after all Updates
	bool Draw();

	// Called before quitting
	bool CleanUp() { return true; };

public:
	SDL_Rect spritePushed;
	SDL_Rect spriteNormal;
	bool pushed;
	bool isMouseOver;
};
class SliderUI : public UI {
public:

	SliderUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriten2, bool d, bool f, SDL_Rect d_area);

	// Destructor
	virtual ~SliderUI() {}

	bool Awake(pugi::xml_node&) { return true; };
	// Call before first frame
	bool Start() { return true; };
	// Called before all Updates
	bool PreUpdate();
	// Called after all Updates
	bool Draw();
	// Called before quitting
	bool CleanUp() { return true; };
	bool OnClick();

private:
	SDL_Rect base;
	SDL_Rect sprite2;

	iPoint mouse;
	bool clickRet;
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
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Gui creation functions
	UI* CreateUIElement(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite = { 0,0,0,0 }, SString str = "", SDL_Rect spritePushed = { 0,0,0,0 }, SDL_Rect spriteNormal = { 0,0,0,0 }, bool drageable = false,
		SDL_Rect dragArea = { 0,0,0,0 }, Module* s_listener = nullptr);
	bool DeleteUIElement(UI*);

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

	int slider;

};

#endif // __j1GUI_H__