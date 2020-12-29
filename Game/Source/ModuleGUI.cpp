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

	return ret;
}

// Called before the first frame
bool ModuleGUI::Start()
{
	atlas = app->tex->Load(atlasFileName.GetString());

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

// Called after all Updates
bool ModuleGUI::PostUpdate()
{
	for (int i = 0; i < uiElements.Count(); i++) {
		uiElements.At(i)->data->PostUpdate();
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

UI* ModuleGUI::CreateUIElement(Type type, UI* parent, SDL_Rect r, SDL_Rect sprite, SString textString, SDL_Rect spritePushed, SDL_Rect spriteNormal, bool drageable, SDL_Rect dragArea, Module* s_listener)
{
	UI* ui = nullptr;
	switch (type)
	{
	case Type::BUTTON:
		ui = new ButtonUI(Type::BUTTON, parent, r, sprite, spritePushed, spriteNormal, true, true, dragArea);
		break;
	case Type::IMAGE:
		ui = new ImageUI(Type::IMAGE, parent, r, sprite, drageable, drageable, dragArea);
		break;
	case Type::WINDOW:
		ui = new WindowUI(Type::WINDOW, parent, r, sprite, drageable, drageable, dragArea);
		break;
	case Type::TEXT:
		ui = new TextUI(Type::TEXT, parent, r, textString, drageable, drageable, dragArea);
		break;
	}

	ui->name = textString;

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

void ModuleGUI::ChangeDebug() {
	for (int i = 0; i < uiElements.Count(); i++) 
	{
		uiElements.At(i)->data->debug = !uiElements.At(i)->data->debug;
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

UI::UI(Type s_type, SDL_Rect r, UI* p, bool d, bool f, SDL_Rect d_area)
{
	name.Create("UI");
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
	debug = false;
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

bool UI::PostUpdate() 
{
	if (debug == true)
	{
		app->render->DrawRectangle(screenRect, 255, 0, 0, 255, false, false);
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

SDL_Rect UI::Check_Printable_Rect(SDL_Rect sprite, iPoint& dif_sprite) 
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

ImageUI::ImageUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area) :UI(type, r, p, d, f, d_area) 
{
	name.Create("ImageUI");
	spriteOver = sprite;
	quad = r;
	SDL_Rect dragArea = GetDragArea();
	dragPosition0 = { dragArea.x, dragArea.y };
	dragPosition1 = { dragArea.w + dragArea.x - GetLocalRect().w,dragArea.h + dragArea.y - GetLocalRect().h };
}

bool ImageUI::PreUpdate() {
	int x, y;
	iPoint initial_position = GetScreenPos();
	app->input->GetMousePosition(x, y);
	if (CheckFocusable() == true && (x >= GetScreenPos().x && x <= GetScreenPos().x + GetScreenRect().w && y >= GetScreenPos().y && y <= GetScreenPos().y + GetScreenRect().h)) 
	{
		if (app->input->GetMouseButtonDown(1) == KEY_DOWN)
		{
			app->gui->DeleteFocus();
			focus = true;
		}
	}
	if (focus == true && app->input->GetMouseButtonDown(1) == KEY_UP) 
	{
		focus = false;
	}

	UI::PreUpdate();

	if (initial_position != GetScreenPos()) 
	{
		fPoint drag_position = GetDragPositionNormalized();
		/////HERE LISTENER WITH DRAG POSITION
	}
	return true;
}

bool ImageUI::PostUpdate() 
{
	iPoint dif_sprite = { 0,0 };
	SDL_Rect sprite = UI::Check_Printable_Rect(spriteOver, dif_sprite);
	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;

	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, quad);
	UI::PostUpdate();
	return true;
}

fPoint ImageUI::GetDragPositionNormalized() 
{
	fPoint position_normalized;
	position_normalized.x = GetScreenPos().x - dragPosition0.x;
	position_normalized.y = GetScreenPos().y - dragPosition0.y;
	position_normalized.x /= dragPosition1.x - dragPosition0.x;
	position_normalized.y /= dragPosition1.y - dragPosition0.y;
	return position_normalized;
}

WindowUI::WindowUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite, bool d, bool f, SDL_Rect d_area) :UI(type, r, p, d, f, d_area)
{
	name.Create("WindowUI");
	spriteOver = sprite;
	quad = r;
}

bool WindowUI::PostUpdate() 
{
	iPoint dif_sprite = { 0,0 };
	SDL_Rect sprite = UI::Check_Printable_Rect(spriteOver, dif_sprite);
	app->render->DrawTexture((SDL_Texture*)app->gui->GetAtlas(), GetScreenPos().x + dif_sprite.x, GetScreenPos().y + dif_sprite.y, &sprite, 1);
	UI::PostUpdate();
	return true;
}

TextUI::TextUI(Type type, UI* p, SDL_Rect r, SString str, bool d, bool f, SDL_Rect d_area) : UI(type, r, p, d, f, d_area)
{
	name.Create("TextUI");
	stri = str;
	quad = r;
}

bool TextUI::PostUpdate() 
{
	SDL_Rect rect = { 0,0,0,0 };
	iPoint dif_sprite = { 0,0 };

	SDL_Texture* text = app->fonts->Print(stri.GetString());

	SDL_QueryTexture(text, NULL, NULL, &rect.w, &rect.h);


	SDL_Rect sprite = UI::Check_Printable_Rect(rect, dif_sprite);
	app->render->DrawTexture(text, GetScreenToWorldPos().x + dif_sprite.x, GetScreenToWorldPos().y + dif_sprite.y, &sprite, 1);
	UI::PostUpdate();

	app->tex->UnLoad(text);

	return true;
}

ButtonUI::ButtonUI(Type type, UI* p, SDL_Rect r, SDL_Rect sprite_over, SDL_Rect sprite_pushed, SDL_Rect sprite_normal, bool d, bool f, SDL_Rect d_area) :UI(type, r, p, d, f, d_area) 
{
	name.Create("ButtonUI");
	spriteOver = sprite_over;
	spritePushed = sprite_pushed;
	spriteNormal = sprite_normal;
	isMouseOver = false;
	pushed = false;
	quad = r;
}

bool ButtonUI::PostUpdate() 
{
	SDL_Rect sprite;
	iPoint dif_sprite = { 0,0 };
	if (pushed == true) {
		sprite = UI::Check_Printable_Rect(spritePushed, dif_sprite);
	}
	else if (isMouseOver == true) {
		sprite = UI::Check_Printable_Rect(spriteOver, dif_sprite);
	}
	else {
		sprite = UI::Check_Printable_Rect(spriteNormal, dif_sprite);
	}
	//App->render->Blit((SDL_Texture*)App->gui->GetAtlas(), GetScreenToWorldPos().x + dif_sprite.x, GetScreenToWorldPos().y + dif_sprite.y, &sprite, 0.f);

	quad.x = GetScreenPos().x + dif_sprite.x;
	quad.y = GetScreenPos().y + dif_sprite.y;
	app->render->BlitInRect((SDL_Texture*)app->gui->GetAtlas(), sprite, quad);

	UI::PostUpdate();
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
		LOG("Click");
		pushed = false;
	}

	UI::PreUpdate();

	return true;
}
