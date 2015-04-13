#pragma once

#include <glm/glm.hpp>
#include "Character.h"

class Soldier;
class Building;

class Enemy : public Character
{
public:
	float timeLeftForAttack;
	int health;

	Soldier* lastAttackedCharacter;
	Building* lastAttackedBuilding;

	Enemy(glm::vec2 p, Flowmap* flowMap);
};