#pragma once

#include <glm/glm.hpp>
#include <vector>

class Soldier;
class Flowmap;

class Flag
{
public:
	Flowmap flowmap;
	glm::ivec2 position;

	std::vector<Soldier*> soldiers;

	Flag(const glm::ivec2 &p)
	{
		position = p;
		flowmap.srcPosition = p;
	}

private:
	Flag(const Flag& other) { throw "argh"; }
};