#include "Sieged.h"

#include <glm/gtc/matrix_transform.hpp>
#include <fstream>

#include <blib/Util.h>
#include <blib/Renderer.h>
#include <blib/ResourceManager.h>
#include <blib/SpriteBatch.h>
#include <blib/LineBatch.h>
#include <blib/Math.h>
#include <blib/Animation.h>
#include <blib/Color.h>
#include <blib/linq.h>
#include <blib/json.h>
#include <blib/util/FileSystem.h>
#include <blib/math/Line.h>
#include <blib/Window.h>
#include <blib/AnimatableSprite.h>
#include <clipper/clipper.hpp>

#include <blib/util/Log.h>
using blib::util::Log;

Sieged::Sieged()
{
	appSetup.renderer = blib::AppSetup::GlRenderer;
	appSetup.title = "Sieged";
	appSetup.window.setWidth(1920);
	appSetup.window.setHeight(1079);

	appSetup.vsync = false;
	appSetup.joystickDriver = blib::AppSetup::DirectInput;
    
    appSetup.threaded = false;

}

void Sieged::init()
{
	tiles = resourceManager->getResource<blib::Texture>("assets/textures/tiles.png");
}

void Sieged::update(double elapsedTime)
{
	if (keyState.isPressed(blib::Key::ESC))
	{
		running = false;
		return;
	}


	/*if (keyState.isPressed(blib::Key::LEFT))
		cameraPos.x -= (float)(500 * elapsedTime);
	if (keyState.isPressed(blib::Key::RIGHT))
		cameraPos.x += (float)(500 * elapsedTime);
	if (keyState.isPressed(blib::Key::UP))
		cameraPos.y -= (float)(500 * elapsedTime);
	if (keyState.isPressed(blib::Key::DOWN))
		cameraPos.y += (float)(500 * elapsedTime);*/
//	if (keyState.isPressed(blib::Key::PLUS))
//		zoom *= (float)(1 + elapsedTime);
//	if (keyState.isPressed(blib::Key::MINUS))
//		zoom *= (float)(1 - elapsedTime);




	prevMouseState = mouseState;
}



void Sieged::draw()
{
	renderer->clear(glm::vec4(0, 1, 0, 1), blib::Renderer::Color);


	spriteBatch->begin();
	
	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 18; y++)
		{
			spriteBatch->draw(tiles, blib::math::easyMatrix(glm::vec2(64*x, 64*y)), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.0f, 0.25f, 0.25f));
		}
	}


	spriteBatch->draw(tiles, blib::math::easyMatrix(glm::vec2(64 * 4, 64 * 5)), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.25f, 0.5f, 0.75f));
	spriteBatch->draw(tiles, blib::math::easyMatrix(glm::vec2(64 * 10, 64 * 7)), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.25f, 0.5f, 0.75f));


	glm::ivec2 mousePos(mouseState.position / glm::ivec2(64,64));
	spriteBatch->draw(tiles, blib::math::easyMatrix(glm::vec2(64 * mousePos.x, 64 * mousePos.y)), glm::vec2(0, 0), blib::math::Rectangle(0.25f, 0.0f, 0.25f, 0.25f));





	spriteBatch->end();


}