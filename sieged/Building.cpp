#include "Building.h"
#include "BuildingTemplate.h"
#include "Tile.h"
#include "Sieged.h"

#include <blib/Window.h>


#include <glm/gtc/matrix_transform.hpp>

Building::Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap)
{
	this->buildingTemplate = buildingTemplate;
	this->position = position;
	for (int x = 0; x < buildingTemplate->size.x; x++)
		for (int y = 0; y < buildingTemplate->size.y; y++)
			tilemap[position.x + x][position.y + y]->building = this;
	this->buildTimeLeft = buildingTemplate->buildTime;
	this->health = buildingTemplate->hitpoints;
}

void Building::drawHealthBar(Sieged* sieged)
{
	if (health == buildingTemplate->hitpoints && buildTimeLeft == 0)
		return;

	glm::vec3 p = glm::project(glm::vec3(position.x + buildingTemplate->size.x / 2.0f, 4, position.y + buildingTemplate->size.y / 2.0f), sieged->cameraMatrix, sieged->projectionMatrix, glm::uvec4(0, 0, sieged->window->getWidth(), sieged->window->getHeight()));

	float buildFactor = 1.0f - glm::min(1.0f, buildTimeLeft / buildingTemplate->buildTime);
	float healthFactor = health / (float)buildingTemplate->hitpoints * buildFactor;


	drawHealthBarActual(sieged, p, buildingTemplate->healthbarSize, healthFactor);

}

