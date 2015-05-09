#pragma once

#include "PlayerCharacter.h"
#include "GameSettings.h"

#include <blib/SkelAnimatedModel.h>

class Flag;
class Enemy;

class Archer : public PlayerCharacter
{
public:
	Archer(const glm::vec2 &p, const GameSettings &gameSettings);
	void calculateWallPosition(TileMap tiles);


	bool atFlag;


	blib::SkelAnimatedModel::State* modelState;
};
