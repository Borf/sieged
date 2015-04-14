#include "Tile.h"

#include "Building.h"
#include "BuildingTemplate.h"

bool Tile::isWall()
{
	if (!building) 
		return false; 
	
	return building->buildingTemplate->type == BuildingTemplate::Wall;
}

Tile::Tile()
{
	building = NULL;
}

