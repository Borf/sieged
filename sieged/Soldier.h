#pragma once

#include <glm/glm.hpp>
#include "Character.h"

class Flag;
class Enemy;

class Soldier : public Character
{
public:
	Soldier(glm::vec2 p);
	float timeLeftForAttack;
	int health;

	Flag* flag;
	Enemy* lastAttackedCharacter;
};