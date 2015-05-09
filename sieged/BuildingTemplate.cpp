#include "BuildingTemplate.h"
#include <blib/json.h>

BuildingTemplate::BuildingTemplate(const blib::json::Value &data, blib::TextureMap* textureMap, blib::StaticModel* model)
{
	this->type = (BuildingTemplate::Type)data["id"].asInt();
	this->size = glm::ivec2(data["size"][0].asInt(), data["size"][1].asInt());
	this->texInfo = textureMap->addTexture(data["beltthumb"]);
	this->model = model;
	this->buildTime = data["constructiontime"].asFloat();

	this->rngWeight = -1;
	if (data.isMember("rng"))
		rngWeight = data["rng"];
	cost = data["cost"];
	hitpoints = data["hitpoints"];

	healthbarSize = 7000;
	if (data.isMember("healthbarsize"))
		healthbarSize = data["healthbarsize"];
}


