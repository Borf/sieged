#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Damagable.h"

class Building;
class Flowmap;
class Tile;
typedef std::vector<std::vector<Tile*>> TileMap;

class Character : public Damagable
{
public:
	glm::vec2 position;
	glm::vec2 movementDirection;
	glm::vec2 movementTarget;
	float speed;
	Flowmap* flowmap;

	int			strength = 1;
	Damagable*	lastAttackedEntity = NULL;
	float		timeLeftForAttack;

	void move(TileMap& tiles, float elapsedTime, bool ignoreCollision = false);
	glm::vec2 directionFromFlowMap();

	void drawHealthBar(Sieged* sieged) override;;
};