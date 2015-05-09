#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "FlowMap.h"

class Knight;

class Flag
{
public:
	Flowmap flowmap;
	glm::ivec2 position;
	bool knightFlag;
	std::vector<Knight*> knights;
	std::vector<Archer*> archers;

	Flag(const glm::ivec2 &p)
	{
		position = p;
		flowmap.srcPosition = p;
	}

private:
	Flag(const Flag& other) { throw "argh"; }
};