#include "Archer.h"
#include "Flag.h"
#include "Tile.h"

#include <blib/Math.h>
#include <blib/linq.h>

Archer::Archer(const glm::vec2 &p, const GameSettings &gameSettings)
{
	this->position = p;
	this->speed = blib::math::randomFloat(1.25f, 1.5f);
	health = gameSettings.archerHealth;
	atFlag = false;
	modelState = NULL;
}

void Archer::calculateWallPosition(TileMap tiles)
{
	int archerIndex = blib::linq::indexOf(flag->archers, this);
	for (int distance = 0; distance < 20; distance++)
	{
		//TODO: out of bounds checck
		if (tiles[flag->position.x + distance][flag->position.y]->isWall())
		{
			archerIndex--;
			if (archerIndex < 0)
			{
				movementTarget = glm::vec2(flag->position) + glm::vec2(0.5f + distance, 0.5f);
				break;
			}
		}
		if (distance == 0)
			continue;
		if (tiles[flag->position.x - distance][flag->position.y]->isWall())
		{
			archerIndex--;
			if (archerIndex < 0)
			{
				movementTarget = glm::vec2(flag->position) + glm::vec2(0.5f - distance, 0.5f);
				break;
			}
		}
		if (tiles[flag->position.x][flag->position.y + distance]->isWall())
		{
			archerIndex--;
			if (archerIndex < 0)
			{
				movementTarget = glm::vec2(flag->position) + glm::vec2(0.5f, 0.5f + distance);
				break;
			}
		}
		if (tiles[flag->position.x][flag->position.y - distance]->isWall())
		{
			archerIndex--;
			if (archerIndex < 0)
			{
				movementTarget = glm::vec2(flag->position) + glm::vec2(0.5f, 0.5f - distance);
				break;
			}
		}
	}
}

