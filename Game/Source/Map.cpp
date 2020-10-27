#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"

#include "Defs.h"
#include "Log.h"

#include <math.h>

Map::Map() : Module(), mapLoaded(false)
{
    name.Create("map");
}

// Destructor
Map::~Map()
{}

// L06: TODO 7: Ask for the value of a custom property
int Properties::GetProperty(const char *value, int defaultValue) const
{
	int ret = defaultValue;
	if (propertyList.count() != 0)
	{
		for (int i = 0; i < propertyList.count() - 1; i++)
		{
			if (strcmp(propertyList.At(i)->data->name, value) == 0) 
			{
				ret = propertyList.At(i)->data->value;
				LOG("%i", ret);
			}
		}
	}	
	return ret;
}

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
    LOG("Loading Map Parser");
    bool ret = true;

    folder.Create(config.child("folder").child_value());

    return ret;
}

// Draw the map (all requried layers)
void Map::Draw()
{
	if (mapLoaded == false) return;

	// L04: TODO 5: Prepare the loop to draw all tilesets + DrawTexture()
	// L06: TODO 4: Make sure we draw all the layers and not just the first one
	ListItem<MapLayer*>* itemlayer = data.maplayers.start;
	while (itemlayer != NULL)
	{
		MapLayer* layer = itemlayer->data;		
		itemlayer = itemlayer->next;
		if (layer->properties.GetProperty("Draw", 1) == 1)
		{
			for (int i = 0; i < layer->width; i++)
			{
				for (int j = 0; j < layer->height; j++)
				{
					if (layer->data[layer->Get(i, j)] != 0)
					{
						layer->Get(i, j);
						SDL_Texture* texture = data.tilesets.start->data->texture;
						iPoint position = MapToWorld(i, j);
						SDL_Rect sect = data.tilesets.start->data->GetTileRect(layer->data[layer->Get(i, j)]);

						app->render->DrawTexture(texture, position.x, position.y, &sect);
					}
				}
			}
		}		
	}
	// L04: TODO 9: Complete the draw function

}

// L04: DONE 8: Create a method that translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	if (data.type == MAPTYPE_ORTHOGONAL)
	{
		ret.x = x * data.tileWidth;
		ret.y = y * data.tileHeight;
	}
	else if (data.type == MAPTYPE_ISOMETRIC)
	{
		ret.x = (x - y) * (data.tileWidth * 0.5f);
		ret.y = (x + y) * (data.tileHeight * 0.5f);
	}
	else
	{
		LOG("Unknown map type");
		ret.x = x; ret.y = y;
	}


	return ret;
}

// L06: TODO 3: Pick the right Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int id) const
{
	ListItem<TileSet*>* item = data.tilesets.start;
	TileSet* set = item->data;

	if (id == (item->data->firstgid == 1)|| id == (item->data->firstgid == 65))
	{
		return set;
	}	
}

// Get relative Tile rectangle
SDL_Rect TileSet::GetTileRect(int tileid) const
{
	SDL_Rect rect = { 0 };
	
	// L04: TODO 7: Get relative Tile rectangle
	uint id = tileid - firstgid;

	rect.w = tileWidth;
	rect.h = tileHeight;
	rect.x = margin + ((rect.w + spacing) * (id % numTilesWidth));
	rect.y = margin + ((rect.h + spacing) * (id / numTilesWidth));

	return rect;
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    // L03: DONE 2: Make sure you clean up any memory allocated from tilesets/map
    // Remove all tilesets
	ListItem<TileSet*>* item;
	item = data.tilesets.start;

	while (item != NULL)
	{
		RELEASE(item->data);
		item = item->next;
	}
	data.tilesets.clear();

	// L04: TODO 2: clean up all layer data
	// Remove all layers
	ListItem<MapLayer*>* item2;
	item2 = data.maplayers.start;

	while (item2 != NULL)
	{
		RELEASE(item2->data);
		item2 = item2->next;
	}
	data.maplayers.clear();
	// Clean up the pugui tree
	mapFile.reset();

    return true;
}

// Load new map
bool Map::Load(const char* filename)
{
    bool ret = true;
    SString tmp("%s%s", folder.GetString(), filename);

    pugi::xml_parse_result result = mapFile.load_file(tmp.GetString());

    if(result == NULL)
    {
        LOG("Could not load map xml file %s. pugi error: %s", filename, result.description());
        ret = false;
    }

    if(ret == true)
    {
        // L03: DONE 3: Create and call a private function to load and fill all your map data
		ret = LoadMap();
	}

    // L03: DONE 4: Create and call a private function to load a tileset
    // remember to support more any number of tilesets!
	pugi::xml_node tileset;
	for (tileset = mapFile.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if (ret == true) ret = LoadTilesetDetails(tileset, set);

		if (ret == true) ret = LoadTilesetImage(tileset, set);

		data.tilesets.add(set);
	}
	// L04: TODO 4: Iterate all layers and load each of them
	pugi::xml_node layernode;
	for (layernode = mapFile.child("map").child("layer"); layernode && ret; layernode = layernode.next_sibling("layer"))
	{
		MapLayer* set1 = new MapLayer();

		if (ret == true)
		{
			ret = LoadLayer(layernode, set1);
		}
		
		data.maplayers.add(set1);
	}

    if(ret == true)
    {
        // L03: TODO 5: LOG all the data loaded iterate all tilesets and LOG everything
		LOG("Successfully parsed map XML file: %s", filename);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tileWidth, data.tileHeight);

		ListItem<TileSet*>* item = data.tilesets.start;
		while (item != NULL)
		{
			TileSet* s = item->data;
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", s->name.GetString(), s->firstgid);
			LOG("tile width: %d tile height: %d", s->tileWidth, s->tileHeight);
			LOG("spacing: %d margin: %d", s->spacing, s->margin);
			item = item->next;
		}

		// L04: TODO 4: LOG the info for each loaded layer
		ListItem<MapLayer*>* itemLayer = data.maplayers.start;
		while (itemLayer != NULL)
		{
			MapLayer* layer = itemLayer->data;
			LOG("Layer ----");
			LOG("name: %s", layer->name.GetString());
			LOG("tile width: %d tile height: %d", layer->width, layer->height);
						
			itemLayer = itemLayer->next;
		}
    }

	mapLoaded = ret;

    return ret;
}

// L03: TODO: Load map general properties
bool Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = mapFile.child("map");

	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		// L03: TODO: Load map general properties
		data.width = map.attribute("width").as_int();
		data.height = map.attribute("height").as_int();
		data.tileWidth = map.attribute("tilewidth").as_int();
		data.tileHeight = map.attribute("tileheight").as_int();
		SString bg_color(map.attribute("backgroundcolor").as_string());

		data.backgroundColor.r = 0;
		data.backgroundColor.g = 0;
		data.backgroundColor.b = 0;
		data.backgroundColor.a = 0;

		if (bg_color.Length() > 0)
		{
			SString red, green, blue;
			bg_color.SubString(1, 2, red);
			bg_color.SubString(3, 4, green);
			bg_color.SubString(5, 6, blue);

			int v = 0;

			sscanf_s(red.GetString(), "%x", &v);
			if (v >= 0 && v <= 255) data.backgroundColor.r = v;

			sscanf_s(green.GetString(), "%x", &v);
			if (v >= 0 && v <= 255) data.backgroundColor.g = v;

			sscanf_s(blue.GetString(), "%x", &v);
			if (v >= 0 && v <= 255) data.backgroundColor.b = v;
		}

		SString orientation(map.attribute("orientation").as_string());

		if (orientation == "orthogonal")
		{
			data.type = MAPTYPE_ORTHOGONAL;
		}
		else if (orientation == "isometric")
		{
			data.type = MAPTYPE_ISOMETRIC;
		}
		else if (orientation == "staggered")
		{
			data.type = MAPTYPE_STAGGERED;
		}
		else
		{
			data.type = MAPTYPE_UNKNOWN;
		}
	}

	return ret;
}

// L03: TODO: Load Tileset attributes
bool Map::LoadTilesetDetails(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	
	// L03: TODO: Load Tileset attributes
	set->name.Create(tileset_node.attribute("name").as_string());
	set->firstgid = tileset_node.attribute("firstgid").as_int();
	set->tileWidth = tileset_node.attribute("tilewidth").as_int();
	set->tileHeight = tileset_node.attribute("tileheight").as_int();
	set->margin = tileset_node.attribute("margin").as_int();
	set->spacing = tileset_node.attribute("spacing").as_int();
	pugi::xml_node offset = tileset_node.child("tileoffset");

	if (offset != NULL)
	{
		set->offsetX = offset.attribute("x").as_int();
		set->offsetY = offset.attribute("y").as_int();
	}
	else
	{
		set->offsetX = 0;
		set->offsetY = 0;
	}

	return ret;
}

// L03: TODO: Load Tileset image
bool Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if (image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		// L03: TODO: Load Tileset image
		set->texture = app->tex->Load(PATH(folder.GetString(), image.attribute("source").as_string()));
		if (set->texture == NULL) {
			LOG("Error loading texture: Cannot load texture.");
			ret = false;
		}
		int w, h;
		SDL_QueryTexture(set->texture, NULL, NULL, &w, &h);
		set->texWidth = image.attribute("width").as_int();

		if (set->texWidth <= 0)
		{
			set->texWidth = w;
		}

		set->texHeight = image.attribute("height").as_int();

		if (set->texHeight <= 0)
		{
			set->texHeight = h;
		}

		set->numTilesWidth = set->texWidth / set->tileWidth;
		set->numTilesHeight = set->texHeight / set->tileHeight;
	}

	return ret;
}

// L04: TODO 3: Create the definition for a function that loads a single layer
bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;
	
	// L04: TODO 3: Load a single layer
	layer->name.Create(node.attribute("name").as_string());
	layer->width = node.attribute("width").as_int();
	layer->height = node.attribute("height").as_int();
	layer->data = new uint[layer->width * layer->height];
	LoadProperties(node.child("properties"), layer->properties);
	//set->nav = layer.attribute("nav").as_int();

	memset(layer->data, 0, layer->width * layer->height);

	pugi::xml_node tilesgid;
	int i = 0;
	for (tilesgid = node.child("data").child("tile"); tilesgid && ret; tilesgid = tilesgid.next_sibling("tile"))
	{		
		layer->data[i] = tilesgid.attribute("gid").as_uint();
		i++;
	}
	//Load collision layer
	if (layer->name == "collision")
	{
		for (int i = 0; i < layer->width; i++)
		{
			for (int j = 0; j < layer->height; j++)
			{
				if (layer->data[layer->Get(i, j)] != 0)
				{
					layer->Get(i, j);
					iPoint position = MapToWorld(i, j);
					SDL_Rect sect = data.tilesets.start->data->GetTileRect(layer->data[layer->Get(i, j)]);

					groundCol.add(app->collision->AddCollider({ position.x,position.y,sect.w,sect.h }, COLLIDER_GROUND));
				}
			}
		}
	}

	return ret;
}


// L06: TODO 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
	bool ret = true;
	pugi::xml_node& node1 = node;

	for (node1 = node.child("property"); node1; node1 = node1.next_sibling("property"))
	{
		Properties::Property* p = new Properties::Property;
		p->name = node1.attribute("name").as_string();
		p->value = node1.attribute("value").as_int();
		properties.propertyList.add(p);		
	}
	
	return ret;
}
