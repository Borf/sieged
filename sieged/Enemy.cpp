#include "Enemy.h"

#include <blib/Math.h>

Enemy::Enemy(glm::vec2 p, Flowmap* flowMap)
{
	this->position = p; 
	this->speed = blib::math::randomFloat(0.75f, 1.5f); 
	timeLeftForAttack = 0; 
	health = 5; 
	this->flowmap = flowMap; 
}

