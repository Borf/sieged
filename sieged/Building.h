#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Damagable.h"


class BuildingTemplate;
class Tile;
typedef std::vector<std::vector<Tile*>> TileMap;

class Building : public Damagable
{
public:
	glm::ivec2 position;
	BuildingTemplate* buildingTemplate;
	float buildTimeLeft;

	Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap);

	void drawHealthBar(Sieged* sieged) override;;

};
