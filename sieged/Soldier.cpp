#include "Soldier.h"

#include <blib/Math.h>

Soldier::Soldier(glm::vec2 p)
{
	this->position = p; 
	this->speed = blib::math::randomFloat(1.25f, 1.5f); 
	health = 5; 
	timeLeftForAttack = 0; 
	flag = NULL;
	lastAttackedCharacter = NULL;

	modelState = NULL;
}

