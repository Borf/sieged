#pragma once

#include <glm/glm.hpp>
#include "Character.h"
#include "GameSettings.h"

class Knight;
class Building;

class Enemy : public Character
{
public:
	Enemy(glm::vec2 p, Flowmap* flowMap, const GameSettings &gameSettings);
};