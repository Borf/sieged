#pragma once

#include <vector>
#include <glm/glm.hpp>

class BuildingTemplate;
class Tile;
typedef std::vector<std::vector<Tile*>> TileMap;

class Building
{
public:
	glm::ivec2 position;
	BuildingTemplate* buildingTemplate;
	float buildTimeLeft;
	int damage;

	Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap);
};
