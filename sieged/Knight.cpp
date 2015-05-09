#include "Knight.h"

#include <blib/Math.h>

Knight::Knight(glm::vec2 p)
{
	this->position = p; 
	this->speed = blib::math::randomFloat(1.25f, 1.5f); 
	health = 5; 
	timeLeftForAttack = 0; 
	modelState = NULL;
}

