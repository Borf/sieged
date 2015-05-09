#pragma once

#include <glm/glm.hpp>
#include "PlayerCharacter.h"
#include "GameSettings.h"

#include <blib/SkelAnimatedModel.h>

class Flag;
class Enemy;

class Knight : public PlayerCharacter
{
public:
	Knight(const glm::vec2 &p, const GameSettings &gameSettings);


	blib::SkelAnimatedModel::State* modelState;
};