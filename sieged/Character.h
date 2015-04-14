#pragma once

#include <glm/glm.hpp>
#include <vector>

class Building;
class Flowmap;
class Tile;
typedef std::vector<std::vector<Tile*>> TileMap;

class Character
{
public:
	glm::vec2 position;
	glm::vec2 movementDirection;
	glm::vec2 movementTarget;
	float speed;
	Flowmap* flowmap;


	void move(TileMap& tiles, float elapsedTime, bool ignoreCollision = false);
	glm::vec2 directionFromFlowMap();
};