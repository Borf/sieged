#include "Building.h"
#include "BuildingTemplate.h"
#include "Tile.h"


Building::Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap)
{
	this->buildingTemplate = buildingTemplate;
	this->position = position;
	for (int x = 0; x < buildingTemplate->size.x; x++)
		for (int y = 0; y < buildingTemplate->size.y; y++)
			tilemap[position.x + x][position.y + y]->building = this;
	this->buildTimeLeft = buildingTemplate->buildTime;
	this->damage = 0;
}

