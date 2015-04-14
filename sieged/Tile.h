#pragma once

class Building;

class Tile
{
public:
	enum FloorType
	{
		Grass
	} floor;
	Building* building;

	Tile();

	bool isWall();
};