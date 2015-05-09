#include "Knight.h"

#include <blib/Math.h>

Knight::Knight(const glm::vec2 &p, const GameSettings &gameSettings)
{
	this->position = p; 
	this->speed = blib::math::randomFloat(1.25f, 1.5f); 
	health = gameSettings.knightHealth; 
	timeLeftForAttack = 0; 
	modelState = NULL;
}

