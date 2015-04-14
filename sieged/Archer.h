#pragma once

#include "Character.h"

#include <blib/SkelAnimatedModel.h>

class Flag;
class Enemy;

class Archer : public Character
{
public:
	Archer(glm::vec2 p);
	void calculateWallPosition(TileMap tiles);
	float timeLeftForAttack;
	int health;


	bool atFlag;
	Flag* flag;
	Enemy* lastAttackedCharacter;


	blib::SkelAnimatedModel::State* modelState;
};
