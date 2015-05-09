#pragma once

#include "PlayerCharacter.h"

#include <blib/SkelAnimatedModel.h>

class Flag;
class Enemy;

class Archer : public PlayerCharacter
{
public:
	Archer(glm::vec2 p);
	void calculateWallPosition(TileMap tiles);


	bool atFlag;


	blib::SkelAnimatedModel::State* modelState;
};
