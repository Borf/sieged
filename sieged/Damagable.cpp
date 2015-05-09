#include "Damagable.h"
#include "Sieged.h"

#include <blib/SpriteBatch.h>
#include <blib/Color.h>




void Damagable::drawHealthBarActual(Sieged* sieged, const glm::vec3& position, float healthbarSize, float healthFactor)
{
	float barWidth = healthbarSize / sieged->cameraDistance;
	float barHeight = glm::max(300 / sieged->cameraDistance, 4.0f);

	float borderSize = glm::round(glm::min(4.0f, 30 / sieged->cameraDistance));

	float healthBarWidth = (barWidth - 2 * borderSize) * healthFactor;


	glm::vec4 color = glm::mix(blib::Color::reddish, blib::Color::limeGreen, healthFactor);

	sieged->spriteBatch->draw(sieged->whitePixel, blib::math::easyMatrix(glm::vec2(position.x - barWidth / 2 - 1, 1079 - position.y - 1), 0, glm::vec2(barWidth + 2, barHeight + 2)), blib::Color::black);
	sieged->spriteBatch->draw(sieged->whitePixel, blib::math::easyMatrix(glm::vec2(position.x - barWidth / 2, 1079 - position.y), 0, glm::vec2(barWidth, barHeight)));
	sieged->spriteBatch->draw(sieged->whitePixel, blib::math::easyMatrix(glm::vec2(position.x - barWidth / 2 + borderSize, 1079 - position.y + borderSize), 0, glm::vec2(healthBarWidth, barHeight - 2 * borderSize)), color);

}
