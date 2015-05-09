#pragma once

#include <glm/glm.hpp>

class Sieged;
namespace blib { class SpriteBatch;  }

class Damagable
{
public:
	int health;

	bool isAlive()
	{
		return health > 0;
	}

	virtual void damage(int dmg)
	{
		health -= dmg;
	}
	virtual void drawHealthBar(Sieged* sieged) = 0;
protected:
	virtual void drawHealthBarActual(Sieged* sieged, const glm::vec3& position, float healthbarSize, float healthFactor);


};