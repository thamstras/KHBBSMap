#pragma once
#include <string>
#include "Render.h"
void LoadBBSMap(std::string filepath);
void ParseLoadedMap();
void LoadMapTextures();
void LoadMapObjects();
void RenderBBSMap(RenderContext& context);

void gui_MapData();
void gui_loaded_data();
void gui_tex_view();
unsigned int max_object_id();
void gui_object_view(unsigned int object_id);
