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
	float speed;
	Flowmap* flowmap;


	void move(TileMap& tiles, float elapsedTime);
	glm::vec2 directionFromFlowMap();

	Building* updateMovement(float elapsedTime, TileMap &tiles);
};