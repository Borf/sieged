#pragma once

#include <vector>

class Building;

enum Direction
{
	Left = 1, Right = 2, Up = 4, Down = 8
};
class Flowmap
{
public:
	std::vector<std::vector<int> > flow;
	Building* srcBuilding;
	glm::ivec2 srcPosition;
	Building* stopAtBuilding;
	Flowmap()
	{
		flow.resize(100, std::vector<int>(100, 0));
		srcBuilding = NULL;
		stopAtBuilding = NULL;
	}

private:
	Flowmap(const Flowmap& other) { throw "argh"; }
};