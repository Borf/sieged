#include "Character.h"

#include "FlowMap.h"
#include "Tile.h"
#include "Building.h"
#include "Sieged.h"

#include <glm/gtc/matrix_transform.hpp>

glm::vec2 Character::directionFromFlowMap()
{
	glm::ivec2 tile = glm::ivec2(position);
	int direction = flowmap->flow[tile.x][tile.y];

	glm::vec2 tileCenter = glm::vec2(tile) + glm::vec2(0.5f, 0.5f);
	glm::vec2 dir;

	//todo: use a 360° direction
	if ((direction & Left) != 0)
		dir.x = -1;
	else if ((direction & Right) != 0)
		dir.x = 1;

	if ((direction & Up) != 0)
		dir.y = -1;
	else if ((direction & Down) != 0)
		dir.y = 1;


	if ((direction & Left) == 0 && (direction & Right) == 0 && fabs(tileCenter.x - position.x) > 0.1f)
		dir.x = glm::normalize(tileCenter.x - position.x);
	if ((direction & Down) == 0 && (direction & Up) == 0 && fabs(tileCenter.y - position.y) > 0.1f)
		dir.y = glm::normalize(tileCenter.y - position.y);

	return dir;
}



void Character::move(TileMap& tiles, float elapsedTime, bool ignoreCollision)
{
	if (glm::distance(position, movementTarget) < glm::length(movementDirection) * elapsedTime * speed)
	{
		position = movementTarget;
		return;
	}
	glm::vec2 originalPos;

	if (tiles[(int)(position.x)][(int)(position.y)]->building)
		ignoreCollision = true;

	originalPos = position;
	position.x += movementDirection.x * elapsedTime * speed;
	if (!ignoreCollision && tiles[(int)(position.x)][(int)(position.y)]->building)
		position = originalPos;
	originalPos = position;
	position.y += movementDirection.y * elapsedTime * speed;
	if (!ignoreCollision && tiles[(int)(position.x)][(int)(position.y)]->building)
		position = originalPos;
}

void Character::drawHealthBar(Sieged* sieged)
{
	glm::vec3 p = glm::project(glm::vec3(position.x, 1, position.y), sieged->cameraMatrix, sieged->projectionMatrix, glm::uvec4(0, 0, 1920, 1079));

	float healthFactor = health / 5.0f;


	drawHealthBarActual(sieged, p, 1000, healthFactor);
}
