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

UI* ModuleGUI::CreateUIElement(UiType type, UI* parent, SDL_Rect r, SDL_Rect sprite, SString textString, SDL_Rect spritePushed, SDL_Rect spriteNormal, bool drageable, SDL_Rect dragArea, Module* s_listener)
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

	if (s_listener)
	{
		ui->listener = s_listener;
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

UI::UI(UiType s_type, SDL_Rect r, UI* p, bool d, bool f, SDL_Rect d_area)
{
	type = s_type;
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
	dragArea = d_area;
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

	uint win_x, win_y;
	app->win->GetWindowSize(win_x, win_y);
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
		else if (maskRect.x + maskRect.w > win_x) 
		{
			maskRect.w -= maskRect.x + maskRect.w - win_x;
		}
		if (maskRect.y < 0) 
		{
			maskRect.h -= maskRect.y;
			maskRect.y = 0;
		}
		else if (maskRect.y + maskRect.h > win_y) 
		{
			maskRect.h -= maskRect.y + maskRect.h - win_y;
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

SDL_Rect UI::CheckPrintableRect(SDL_Rect sprite, iPoint& dif_sprite) 
{
	if (maskRect.x > screenRect.x)
	{
		dif_sprite.x = maskRect.x - screenRect.x;
		sprite.x += dif_sprite.x;
		sprite.w -= dif_sprite.x;
	}
	else if (maskRect.w < screenRect.w)
	{
		sprite.w -= screenRect.w - maskRect.w;
	}
	if (maskRect.y > screenRect.y)
	{
		dif_sprite.y = maskRect.y - screenRect.y;
		sprite.y += dif_sprite.y;
		sprite.h -= dif_sprite.y;
	}
	else if (maskRect.h < screenRect.h)
	{
		sprite.h -= screenRect.h - maskRect.h;
	}
	return sprite;
}

ImageUI::ImageUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area) : UI(type, r, p, d, f, d_area)
{
	spriteOver = sprite;
	quad = r;
}

bool ImageUI::Draw() 
{
	iPoint dif_sprite = { 0,0 };
	SDL_Rect sprite = UI::CheckPrintableRect(spriteOver, dif_sprite);
	//app->render->DrawTexture((SDL_Texture*)app->gui->GetAtlas(), GetScreenPos().x + dif_sprite.x, GetScreenPos().y + dif_sprite.y, &sprite, 1);
	
	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;
	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, quad);
	
	UI::Draw();
	return true;
}

TextUI::TextUI(UiType type, UI* p, SDL_Rect r, float s, SString str, bool d, bool f, SDL_Rect d_area) : UI(type, r, p, d, f, d_area)
{
	stri = str;
	quad = r;
	size = s;
}

bool TextUI::Draw() 
{
	SDL_Rect rect = { 0,0,0,0 };
	iPoint dif_sprite = { 0,0 };

	SDL_Texture* text = app->fonts->Print(stri.GetString());
	
	SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);


	SDL_Rect sprite = UI::CheckPrintableRect(rect, dif_sprite);

	//app->render->DrawTexture(text, GetScreenPos().x + dif_sprite.x, GetScreenPos().y + dif_sprite.y, &sprite, 1);
	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;
	quad.w = rect.w*size;
	quad.h = rect.h*size;
	app->render->BlitInRect(text, sprite, quad);
	UI::Draw();

	app->tex->UnLoad(text);

	return true;
}

ButtonUI::ButtonUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite_over, SDL_Rect sprite_pushed, SDL_Rect sprite_normal, bool d, bool f, SDL_Rect d_area) :UI(type, r, p, d, f, d_area) 
{
	spriteOver = sprite_over;
	spritePushed = sprite_pushed;
	spriteNormal = sprite_normal;
	isMouseOver = false;
	pushed = false;
	quad = r;
}

bool ButtonUI::Draw() 
{
	SDL_Rect sprite;
	iPoint dif_sprite = { 0,0 };
	if (pushed == true) {
		sprite = UI::CheckPrintableRect(spritePushed, dif_sprite);
	}
	else if (isMouseOver == true) {
		sprite = UI::CheckPrintableRect(spriteOver, dif_sprite);
	}
	else {
		sprite = UI::CheckPrintableRect(spriteNormal, dif_sprite);
	}
	//app->render->Blit((SDL_Texture*)app->gui->GetAtlas(), GetScreenToWorldPos().x + dif_sprite.x, GetScreenToWorldPos().y + dif_sprite.y, &sprite, 0.f);

	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;
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

CheckboxUI::CheckboxUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect sprite2, bool checked, bool f, SDL_Rect d_area) : UI(type, r, p, false, f, d_area)
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
	iPoint dif_sprite = { 0,0 };
	sprite = UI::CheckPrintableRect(spriteOver, dif_sprite);
	base.x = GetScreenPos().x + dif_sprite.x;
	base.y = GetScreenPos().y + dif_sprite.y;
	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, base);
	if (state) 
	{
		sprite = UI::CheckPrintableRect(spriteTick, dif_sprite);
		quad.x = GetScreenPos().x + dif_sprite.x + 4;
		quad.y = GetScreenPos().y + dif_sprite.y + 5;
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

SliderUI::SliderUI(UiType type, UI* p, SDL_Rect r, SDL_Rect sprite, SDL_Rect spriteHandle, bool d, bool f, SDL_Rect d_area) :UI(type, r, p, d, f, d_area) {
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
	iPoint spirte_dif = { 0,0 };
	iPoint spirte_dif1 = { 0,0 };
	SDL_Rect sprite = UI::CheckPrintableRect(spriteOver, spirte_dif);
	SDL_Rect sprite_ = UI::CheckPrintableRect(sprite2, spirte_dif1);
	base.x = GetScreenPos().x + spirte_dif.x;
	base.y = GetScreenPos().y + spirte_dif.y;

	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, base);

	if (OnClick()) {
		int xpos;
		int ypos;
		app->input->GetMousePosition(xpos, ypos);
		app->render->Clamp(&xpos, base.x + 2, base.x + base.w-8);
		quad.x = xpos - 2;
	}

	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite_, quad);
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