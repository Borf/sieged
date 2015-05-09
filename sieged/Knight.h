#pragma once

#include <glm/glm.hpp>
#include "PlayerCharacter.h"

#include <blib/SkelAnimatedModel.h>

class Flag;
class Enemy;

class Knight : public PlayerCharacter
{
public:
	Knight(glm::vec2 p);


	blib::SkelAnimatedModel::State* modelState;
};