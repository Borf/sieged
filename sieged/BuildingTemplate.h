#pragma once

#include <glm/glm.hpp>
#include <blib/TextureMap.h>

namespace blib {
	class StaticModel; 
	namespace json { class Value; }
}

class BuildingTemplate
{
public:
	enum Type
	{
		Wall = 0,
		Gate,
		TownHall,
		StoneMason,
		Bank,
		MineralMine,
		MarketPlace,
		Recycler,
		ArcheryRange,
		Barracks,
		BattleArena,
		ImposingTauntingStatue,
		TeslaTower,
		Smithy,
		Tavern,
		WatchTower,
		AlchemyLabs,
		Workshop,
		Refinery
	} type;
	glm::ivec2 size;
	blib::TextureMap::TexInfo* texInfo;
	blib::StaticModel* model;

	int cost;
	int rngWeight;
	float buildTime;
	int hitpoints;
	float healthbarSize;

	BuildingTemplate(const blib::json::Value &data, blib::TextureMap* textureMap, blib::StaticModel* model);
};

