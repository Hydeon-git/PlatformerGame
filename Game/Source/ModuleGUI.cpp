#include "Defs.h"
#include "Log.h"
#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Fonts.h"
#include "Input.h"
#include "Window.h"
#include "ModuleGUI.h"
#include "Audio.h"

#include "SDL_mixer/include/SDL_mixer.h"

ModuleGUI::ModuleGUI() : Module()
{
	name.Create("gui");
}

// Destructor
ModuleGUI::~ModuleGUI()
{}

// Called before render is available
bool ModuleGUI::Awake(pugi::xml_node& config)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	atlasFileName = config.attribute("atlas").as_string();
	clickSfx = app->audio->LoadFx(config.attribute("clickFx").as_string());
	slider = config.attribute("defaultVolume").as_int();

	return ret;
}

// Called before the first frame
bool ModuleGUI::Start()
{
	atlas = app->tex->Load(atlasFileName.GetString());
	app->audio->SetVolume(slider);
	return true;
}

// Update all guis
bool ModuleGUI::PreUpdate()
{
	bool mouse = false;
	int count = 0;
	if (app->input->GetMouseButtonDown(1) == KEY_DOWN || app->input->GetMouseButtonDown(1) == KEY_REPEAT) {
		for (int i = uiElements.Count() - 1; i >= 0 && mouse == false; i--) {
			mouse = uiElements.At(i)->data->CheckMouse();
			if (mouse == true)count = i;
		}
	}
	if (mouse == true) {
		uiElements.At(count)->data->Move();
	}
	for (int i = 0; i < uiElements.Count(); i++) {
		uiElements.At(i)->data->PreUpdate();
	}
	return true;
}
bool ModuleGUI::Update(float dt)
{
	for (int i = 0; i < uiElements.Count(); i++) {
		uiElements.At(i)->data->Draw();
	}
	return true;
}
// Called before quitting
bool ModuleGUI::CleanUp()
{
	LOG("Freeing GUI");

	for (int i = uiElements.Count() - 1; i >= 0; i--)
	{
		uiElements.At(i)->data->CleanUp();
		uiElements.Del(uiElements.At(i));
	}
	uiElements.Clear();
	if (atlas)
	{
		app->tex->UnLoad(atlas);
	}
	return true;
}

// const getter for atlas
const SDL_Texture* ModuleGUI::GetAtlas() const
{
	return atlas;
}

// class Gui ---------------------------------------------------

UI* ModuleGUI::CreateUIElement(UiType type, UI* parent, SDL_Rect r, SDL_Rect sprite, SString textString, SDL_Rect spritePushed, SDL_Rect spriteNormal, bool drageable, SDL_Rect dragArea, Module* sListener)
{
	UI* ui = nullptr;
	switch (type)
	{
	case UiType::BUTTON:
		ui = new ButtonUI(UiType::BUTTON, parent, r, sprite, spritePushed, spriteNormal, true, true, dragArea);
		break;
	case UiType::IMAGE:
		ui = new ImageUI(UiType::IMAGE, parent, r, sprite, drageable, drageable, dragArea);
		break;
	case UiType::TEXT:
		ui = new TextUI(UiType::TEXT, parent, r, (float)sprite.x/10, textString, drageable, drageable, dragArea);
		break;
	case UiType::CHECKBOX:
		ui = new CheckboxUI(UiType::CHECKBOX, parent, r, sprite, spritePushed, drageable, drageable, dragArea);
		break;
	case UiType::SLIDER:
		ui = new SliderUI(UiType::SLIDER, parent, r, sprite, spritePushed, drageable, drageable, dragArea);
		break;
	}

	if (sListener)
	{
		ui->listener = sListener;
	}
	else
	{
		ui->listener = nullptr;
	}

	return uiElements.Add(ui)->data;
}

bool ModuleGUI::DeleteUIElement(UI* ui) 
{
	int n = uiElements.Find(ui);
	if (n == -1)return false;
	else
	{
		uiElements.At(n)->data->CleanUp();
		uiElements.Del(uiElements.At(n));
		return true;
	}
}

void ModuleGUI::ChangeFocus() 
{
	bool exit = false;
	bool focus = false;
	int count = 0;
	for (int i = 0; i < uiElements.Count() && exit == false; i++) 
	{
		bool focusable = uiElements.At(i)->data->CheckFocusable();
		if (focusable == true) 
		{
			count++;
			if (focus == true) 
			{
				uiElements.At(i)->data->focus = true;
				exit = true;
			}
			else 
			{
				focus = uiElements.At(i)->data->focus;
				uiElements.At(i)->data->focus = false;
			}
		}
	}
	if (count > 0 && exit == false) {
		for (int i = 0; i < uiElements.Count() && exit == false; i++) {
			bool focusable = uiElements.At(i)->data->CheckFocusable();
			if (focusable == true) {
				uiElements.At(i)->data->focus = true;
				exit = true;
			}
		}
	}
}

void ModuleGUI::DeleteFocus() 
{
	for (int i = 0; i < uiElements.Count(); i++) 
	{
		uiElements.At(i)->data->focus = false;
	}
}

void ModuleGUI::ClearUI()
{
	uiElements.Clear();
}

UI::UI(UiType sType, SDL_Rect r, UI* p, bool d, bool f, SDL_Rect dArea)
{
	type = sType;
	drageable = d;
	focusable = f;
	screenRect = { r.x,r.y,r.w,r.h };
	parent = p;
	if (parent != nullptr) 
	{
		localRect = { screenRect.x - parent->screenRect.x, screenRect.y - parent->screenRect.y, screenRect.w, screenRect.h };
	}
	else
	{
		localRect = screenRect;
	}
	maskRect = screenRect;
	focus = false;
	dragArea = dArea;
}

bool UI::PreUpdate() 
{
	UI* ui = this;

	screenRect.x = localRect.x;
	screenRect.y = localRect.y;
	while (ui->parent != nullptr) 
	{
		screenRect.x += ui->parent->localRect.x;
		screenRect.y += ui->parent->localRect.y;
		ui = ui->parent;
	}

	uint winX, winY;
	app->win->GetWindowSize(winX, winY);
	maskRect = screenRect;

	if (parent != nullptr) 
	{
		if (maskRect.x < parent->maskRect.x)
		{
			maskRect.x += parent->maskRect.x - maskRect.x;
			maskRect.w -= parent->maskRect.x - maskRect.x;
		}
		else if (maskRect.x + maskRect.w > parent->maskRect.x + parent->maskRect.w) 
		{
			maskRect.w -= maskRect.x + maskRect.w - parent->maskRect.x - parent->maskRect.w;
		}
		if (maskRect.y < parent->maskRect.y) 
		{
			maskRect.y += parent->maskRect.y - maskRect.y;
			maskRect.h -= parent->maskRect.y - maskRect.y;
		}
		else if (maskRect.y + maskRect.h > parent->maskRect.y + parent->maskRect.h) 
		{
			maskRect.h -= maskRect.y + maskRect.h - parent->maskRect.y - parent->maskRect.h;
		}
	}
	else {
		if (maskRect.x < 0) 
		{
			maskRect.w -= maskRect.x;
			maskRect.x = 0;
		}
		else if (maskRect.x + maskRect.w > winX) 
		{
			maskRect.w -= maskRect.x + maskRect.w - winX;
		}
		if (maskRect.y < 0) 
		{
			maskRect.h -= maskRect.y;
			maskRect.y = 0;
		}
		else if (maskRect.y + maskRect.h > winY) 
		{
			maskRect.h -= maskRect.y + maskRect.h - winY;
		}
	}
	return true;
}

bool UI::Draw() 
{
	if (app->guiDebug == true)
	{
		SDL_Rect scr = screenRect;
		scr.x *= app->win->GetScale();
		scr.y *= app->win->GetScale();
		scr.w *= app->win->GetScale();
		scr.h *= app->win->GetScale();
		app->render->DrawRectangle(scr, 255, 0, 0, 255, false, false);
	}
	return true;
}

SDL_Rect UI::GetScreenRect()
{
	return screenRect;
}
SDL_Rect UI::GetLocalRect() 
{
	return localRect;
}
iPoint UI::GetScreenPos() 
{
	return { screenRect.x,screenRect.y };
}
iPoint UI::GetScreenToWorldPos() 
{
	return { screenRect.x / (int)app->win->GetScale(),screenRect.y / (int)app->win->GetScale() };
}
iPoint UI::GetLocalPos()
{
	return { localRect.x,localRect.y };
}
void UI::SetLocalPos(iPoint pos) 
{
	iPoint r = { -localRect.x + pos.x,-localRect.y + pos.y };
	localRect.x = pos.x;
	localRect.y = pos.y;
	screenRect.x += r.x;
	screenRect.y += r.y;
}

bool UI::CheckMouse() 
{
	if (drageable == true)
	{
		int x, y;
		app->input->GetMousePosition(x, y);
		if (x >= screenRect.x && x <= screenRect.x + screenRect.w && y >= screenRect.y && y <= screenRect.y + screenRect.h || focus == true)
			return true;
	}
	return false;
}

bool UI::Move() 
{
	int x, y;
	app->input->GetMouseMotion(x, y);
	if (screenRect.x + x >= dragArea.x && screenRect.x + screenRect.w + x <= dragArea.x + dragArea.w)
		localRect.x += x;
	else if (screenRect.y + y >= dragArea.y && screenRect.y + screenRect.h + y <= dragArea.y + dragArea.h)
		localRect.y += y;
	return true;
}

SDL_Rect UI::CheckPrintableRect(SDL_Rect sprite, iPoint& difSprite) 
{
	if (maskRect.x > screenRect.x)
	{
		difSprite.x = maskRect.x - screenRect.x;
		sprite.x += difSprite.x;
		sprite.w -= difSprite.x;
	}
	else if (maskRect.w < screenRect.w)
	{
		sprite.w -= screenRect.w - maskRect.w;
	}
	if (maskRect.y > screenRect.y)
	{
		difSprite.y = maskRect.y - screenRect.y;
		sprite.y += difSprite.y;
		sprite.h -= difSprite.y;
	}
	else if (maskRect.h < screenRect.h)
	{
		sprite.h -= screenRect.h - maskRect.h;
	}
	return sprite;
}

ImageUI::ImageUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect dArea) : UI(type, r, p, d, f, dArea)
{
	spriteOver = sprite;
	quad = r;
}

bool ImageUI::Draw() 
{
	iPoint difSprite = { 0,0 };
	SDL_Rect sprite = UI::CheckPrintableRect(spriteOver, difSprite);
	//app->render->DrawTexture((SDL_Texture*)app->gui->GetAtlas(), GetScreenPos().x + difSprite.x, GetScreenPos().y + difSprite.y, &sprite, 1);
	
	quad.x = GetScreenPos().x + difSprite.x;
	quad.y = GetScreenPos().y + difSprite.y;
	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, quad);
	
	UI::Draw();
	return true;
}

TextUI::TextUI(UiType type, UI* p, SDL_Rect r, float s, SString str, bool d, bool f, SDL_Rect dArea) : UI(type, r, p, d, f, dArea)
{
	stri = str;
	quad = r;
	size = s;
}

bool TextUI::Draw() 
{
	SDL_Rect rect = { 0,0,0,0 };
	iPoint difSprite = { 0,0 };

	SDL_Texture* text = app->fonts->Print(stri.GetString());
	
	SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);


	SDL_Rect sprite = UI::CheckPrintableRect(rect, difSprite);

	//app->render->DrawTexture(text, GetScreenPos().x + difSprite.x, GetScreenPos().y + difSprite.y, &sprite, 1);
	quad.x = GetScreenPos().x + difSprite.x;
	quad.y = GetScreenPos().y + difSprite.y;
	quad.w = rect.w*size;
	quad.h = rect.h*size;
	app->render->BlitInRect(text, sprite, quad);
	UI::Draw();

	app->tex->UnLoad(text);

	return true;
}

ButtonUI::ButtonUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprOver, SDL_Rect sprPushed, SDL_Rect sprNormal, bool d, bool f, SDL_Rect dArea) :UI(type, r, p, d, f, dArea) 
{
	spriteOver = sprOver;
	spritePushed = sprPushed;
	spriteNormal = sprNormal;
	isMouseOver = false;
	pushed = false;
	quad = r;
}

bool ButtonUI::Draw() 
{
	SDL_Rect sprite;
	iPoint difSprite = { 0,0 };
	if (pushed == true) {
		sprite = UI::CheckPrintableRect(spritePushed, difSprite);
	}
	else if (isMouseOver == true) {
		sprite = UI::CheckPrintableRect(spriteOver, difSprite);
	}
	else {
		sprite = UI::CheckPrintableRect(spriteNormal, difSprite);
	}
	//app->render->Blit((SDL_Texture*)app->gui->GetAtlas(), GetScreenToWorldPos().x + difSprite.x, GetScreenToWorldPos().y + difSprite.y, &sprite, 0.f);

	quad.x = GetScreenPos().x + difSprite.x;
	quad.y = GetScreenPos().y + difSprite.y;
	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, quad);

	UI::Draw();
	return true;
}

bool ButtonUI::PreUpdate() 
{
	int x, y;
	app->input->GetMousePosition(x, y);
	if ((x >= GetScreenPos().x && x <= GetScreenPos().x + GetScreenRect().w && y >= GetScreenPos().y && y <= GetScreenPos().y + GetScreenRect().h) || focus == true)
		isMouseOver = true;
	else isMouseOver = false;

	if (app->input->GetMouseButtonDown(1) == KEY_DOWN && isMouseOver == true)
		pushed = true;
	else if (pushed == true && app->input->GetMouseButtonDown(1) == KEY_UP)
	{
		app->audio->PlayFx(app->gui->clickSfx);
		//Button clicked
		if (listener)
		{
			listener->OnClick(this);
		}
		pushed = false;
	}

	UI::PreUpdate();

	return true;
}

CheckboxUI::CheckboxUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect sprite2, bool checked, bool f, SDL_Rect dArea) : UI(type, r, p, false, f, dArea)
{
	spriteOver = sprite;
	spriteTick = sprite2;

	state = checked;

	base = r;
	quad.x = r.x;
	quad.y = r.y;
	quad.w = spriteTick.w*1.8f;
	quad.h = spriteTick.h*1.8f;
}

bool CheckboxUI::Draw()
{
	SDL_Rect sprite;
	iPoint difSprite = { 0,0 };
	sprite = UI::CheckPrintableRect(spriteOver, difSprite);
	base.x = GetScreenPos().x + difSprite.x;
	base.y = GetScreenPos().y + difSprite.y;
	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, base);
	if (state) 
	{
		sprite = UI::CheckPrintableRect(spriteTick, difSprite);
		quad.x = GetScreenPos().x + difSprite.x + 4;
		quad.y = GetScreenPos().y + difSprite.y + 5;
		app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, quad);
	}

	UI::Draw();
	return true;
}

bool CheckboxUI::PreUpdate()
{
	int x, y;
	app->input->GetMousePosition(x, y);
	if ((x >= GetScreenPos().x && x <= GetScreenPos().x + GetScreenRect().w && y >= GetScreenPos().y && y <= GetScreenPos().y + GetScreenRect().h) || focus == true)
		isMouseOver = true;
	else isMouseOver = false;

	if (app->input->GetMouseButtonDown(1) == KEY_DOWN && isMouseOver == true)
	{
		state = !state;
		app->audio->PlayFx(app->gui->clickSfx);
		//Button clicked
		if (listener)
		{
			listener->OnClick(this);
		}
	}

	UI::PreUpdate();

	return true;
}

SliderUI::SliderUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriteHandle, bool d, bool f, SDL_Rect dArea) :UI(type, r, p, d, f, dArea) {
	spriteOver = sprite;
	sprite2 = spriteHandle;
	base = r;

	quad.x = app->gui->slider + base.x;
	quad.y = r.y - 6;
	quad.w = spriteHandle.w*2;
	quad.h = spriteHandle.h*2;

	clickRet = false;
}

bool SliderUI::Draw()
{
	iPoint spirteDif = { 0,0 };
	iPoint spirteDif1 = { 0,0 };
	SDL_Rect sprite = UI::CheckPrintableRect(spriteOver, spirteDif);
	SDL_Rect sprite1 = UI::CheckPrintableRect(sprite2, spirteDif1);
	base.x = GetScreenPos().x + spirteDif.x;
	base.y = GetScreenPos().y + spirteDif.y;

	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, base);

	if (OnClick()) {
		int xpos;
		int ypos;
		app->input->GetMousePosition(xpos, ypos);
		app->render->Clamp(&xpos, base.x + 2, base.x + base.w-8);
		quad.x = xpos - 2;
	}

	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite1, quad);
	float vol = ((float)(quad.x - base.x) / (base.w - 10))*100;
	app->gui->slider = vol;

	UI::Draw();
	return true;
}

bool SliderUI::PreUpdate()
{
	app->audio->SetVolume(app->gui->slider);
	UI::PreUpdate();
	return true;
}

bool SliderUI::OnClick()
{
	app->input->GetMousePosition(mouse.x, mouse.y);
	if (mouse.x < quad.x + quad.w && mouse.x > quad.x) {
		if (mouse.y<quad.y + quad.h && mouse.y>quad.y) {
			if (app->input->GetMouseButtonDown(SDL_BUTTON_LEFT))
				clickRet = true;
		}
	}

	if (!app->input->GetMouseButtonDown(SDL_BUTTON_LEFT)) {
		clickRet = false;
	}

	return clickRet;
}