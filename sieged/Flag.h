#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "FlowMap.h"

class Soldier;

class Flag
{
public:
	Flowmap flowmap;
	glm::ivec2 position;
	bool soldierFlag;
	std::vector<Soldier*> soldiers;
	std::vector<Archer*> archers;

	Flag(const glm::ivec2 &p)
	{
		position = p;
		flowmap.srcPosition = p;
	}

private:
	Flag(const Flag& other) { throw "argh"; }
};