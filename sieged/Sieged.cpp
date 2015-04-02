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
	tileTexture = resourceManager->getResource<blib::Texture>("assets/textures/tiles.png");
	enemyTexture = resourceManager->getResource<blib::Texture>("assets/textures/enemy.png");
	arrowsTexture = resourceManager->getResource<blib::Texture>("assets/textures/arrows.png");


	buildingTemplates[BuildingTemplate::TownHall] = new BuildingTemplate(BuildingTemplate::TownHall, glm::ivec2(2,2));
	buildingTemplates[BuildingTemplate::StoneMason] = new BuildingTemplate(BuildingTemplate::StoneMason, glm::ivec2(2,3));
	buildingTemplates[BuildingTemplate::Farm] = new BuildingTemplate(BuildingTemplate::Farm, glm::ivec2());
	buildingTemplates[BuildingTemplate::MarketPlace] = new BuildingTemplate(BuildingTemplate::MarketPlace, glm::ivec2(4,5));
	buildingTemplates[BuildingTemplate::ArcheryRange] = new BuildingTemplate(BuildingTemplate::ArcheryRange, glm::ivec2(3,6));
	buildingTemplates[BuildingTemplate::WizardTower] = new BuildingTemplate(BuildingTemplate::WizardTower, glm::ivec2());
	buildingTemplates[BuildingTemplate::Smithy] = new BuildingTemplate(BuildingTemplate::Smithy, glm::ivec2());
	buildingTemplates[BuildingTemplate::Tavern] = new BuildingTemplate(BuildingTemplate::Tavern, glm::ivec2());
	buildingTemplates[BuildingTemplate::WatchTower] = new BuildingTemplate(BuildingTemplate::WatchTower, glm::ivec2());
	buildingTemplates[BuildingTemplate::AlchemyLabs] = new BuildingTemplate(BuildingTemplate::AlchemyLabs, glm::ivec2());

	tiles.resize(100, std::vector<Tile*>(100, nullptr));
	for (int x = 0; x < 100; x++)
		for (int y = 0; y < 100; y++)
			tiles[x][y] = new Tile();

	buildings.push_back(new Building(glm::ivec2(15,7), buildingTemplates[BuildingTemplate::TownHall], tiles));


	for (int ii = 0; ii < 6; ii+=2)
		for (int i = 0; i < 6+ii*2; i++)
		{
			tiles[13 + i - ii][5-ii]->building = (Building*)1;
			tiles[13-ii][5 + i - ii]->building = (Building*)1;
			tiles[13+i-ii][10+ii]->building = (Building*)1;
			tiles[18 + ii][5 + i-ii]->building = (Building*)1;
		}

	tiles[13][7]->building = NULL;
	tiles[20][7]->building = NULL;
	tiles[9][8]->building = NULL;

	calcPaths();

	while (enemies.size() < 100)
	{
		glm::vec2 pos(blib::math::randomFloat(0, 1920), blib::math::randomFloat(0, 1080));
		if (tiles[(int)(pos.x / 64)][(int)(pos.y / 64)]->building)
			continue;
		enemies.push_back(new Enemy(pos));
	}
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

	for (Enemy* e : enemies)
	{
		glm::ivec2 tile = glm::ivec2(e->position / 64.0f);

		int direction = tiles[tile.x][tile.y]->toBase;

		glm::vec2 oldPos;
		glm::vec2 oldOldPos = e->position;


		oldPos = e->position;
		if ((direction & Tile::Left) != 0)
			e->position.x -= elapsedTime * 128;
		if (tiles[(int)(e->position.x / 64)][(int)(e->position.y / 64)]->building)
			e->position = oldPos;
		oldPos = e->position;
		if ((direction & Tile::Right) != 0)
			e->position.x += elapsedTime * 128;
		if (tiles[(int)(e->position.x / 64)][(int)(e->position.y / 64)]->building)
			e->position = oldPos;
		oldPos = e->position;
		if ((direction & Tile::Down) != 0)
			e->position.y += elapsedTime * 128;
		if (tiles[(int)(e->position.x / 64)][(int)(e->position.y / 64)]->building)
			e->position = oldPos;
		oldPos = e->position;
		if ((direction & Tile::Up) != 0)
			e->position.y -= elapsedTime * 128;
		if (tiles[(int)(e->position.x / 64)][(int)(e->position.y / 64)]->building)
			e->position = oldPos;
		oldPos = e->position;

		for (auto ee : enemies)
		{
			if (e != ee && glm::distance(ee->position, e->position) < 10)
				e->position = oldOldPos;
		}

	}


	while (enemies.size() < 100 + time*5)
	{
		glm::vec2 pos(blib::math::randomFloat(0, 1920), blib::math::randomFloat(0, 1080));
		if (tiles[(int)(pos.x / 64)][(int)(pos.y / 64)]->building)
			continue;
		enemies.push_back(new Enemy(pos));
	}


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
			if (tiles[x][y]->building)
				continue;
			spriteBatch->draw(tileTexture, blib::math::easyMatrix(glm::vec2(64 * x, 64 * y)), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.0f, 0.25f, 0.25f));
		}
	}

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 18; y++)
		{
			if (tiles[x][y]->building)
				continue;

			glm::vec2 dir(0, 0);
			if ((tiles[x][y]->toBase & Tile::Left) != 0)
				dir.x--;
			if ((tiles[x][y]->toBase & Tile::Right) != 0)
				dir.x++;
			if ((tiles[x][y]->toBase & Tile::Up) != 0)
				dir.y--;
			if ((tiles[x][y]->toBase & Tile::Down) != 0)
				dir.y++;

			spriteBatch->draw(arrowsTexture, blib::math::easyMatrix(glm::vec2(64 * x, 64 * y)), glm::vec2(0, 0), blib::math::Rectangle(glm::vec2(0.33f, 0.33f) + .33f * dir, 0.33f, 0.33f));

		}
	}

	for (auto b : buildings)
		spriteBatch->draw(tileTexture, blib::math::easyMatrix(glm::vec2(64 * b->position.x, 64 * (b->position.y + b->buildingTemplate->size.y) - 0.75f * tileTexture->originalHeight)), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.25f, 0.5f, 0.75f));

	for (auto e : enemies)
	{
		spriteBatch->draw(enemyTexture, blib::math::easyMatrix(e->position), glm::vec2(enemyTexture->originalWidth/2, enemyTexture->originalHeight-2));
	}


	glm::ivec2 mousePos(mouseState.position / glm::ivec2(64,64));
	spriteBatch->draw(tileTexture, blib::math::easyMatrix(glm::vec2(64 * mousePos.x, 64 * mousePos.y)), glm::vec2(0, 0), blib::math::Rectangle(0.25f, 0.0f, 0.25f, 0.25f));





	spriteBatch->end();


}

void Sieged::calcPaths()
{
	std::vector<std::vector<float>> costs(100, std::vector<float>(100,9999999));
	
	std::list<glm::ivec2> queue;
	for (int x = 0; x < buildings[0]->buildingTemplate->size.x; x++)
		for (int y = 0; y < buildings[0]->buildingTemplate->size.y; y++)
			queue.push_back(buildings[0]->position + glm::ivec2(x,y));

	for (auto p : queue)
		costs[p.x][p.y] = 0;
	int a = 0;
	while (!queue.empty())
	{
		glm::ivec2 pos = queue.back();
		queue.pop_back();

		for (int x = -1; x <= 1; x++)
		{
			for (int y = -1; y <= 1; y++)
			{
				if (x == 0 && y == 0)
					continue;
				glm::ivec2 offset(x, y);
				glm::ivec2 newPos = pos + offset;
				if (newPos.x < 0 || newPos.x >= 100 || newPos.y < 0 || newPos.y >= 100)
					continue;

				if (tiles[newPos.x][newPos.y]->building)
					continue;
				
				float newCost = costs[pos.x][pos.y] + glm::length(glm::vec2(offset));
				if (costs[newPos.x][newPos.y] < newCost)
					continue;
				
				if (!blib::linq::containsValue(queue, newPos))
					queue.push_front(newPos);
				costs[newPos.x][newPos.y] = newCost;
			}
		}
	}


	for (int x = 0; x < 100; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			tiles[x][y]->toBase = 0;
			if (tiles[x][y]->building)
				continue;
			glm::ivec2 m(0, 0);
			for (int xx = -1; xx <= 1; xx++)
			{
				for (int yy = -1; yy <= 1; yy++)
				{
					int xxx = x + xx;
					int yyy = y + yy;
					if (xxx < 0 || xxx >= 100 || yyy < 0 || yyy >= 100)
						continue;
					if (costs[xxx][yyy] < costs[x + m.x][y + m.y])
						m = glm::ivec2(xx, yy);
				}
			}
			if (m.x < 0)
				tiles[x][y]->toBase |= Tile::Left;
			if (m.x > 0)
				tiles[x][y]->toBase |= Tile::Right;
			if (m.y < 0)
				tiles[x][y]->toBase |= Tile::Up;
			if (m.y > 0)
				tiles[x][y]->toBase |= Tile::Down;
		}
	}



}











Building::Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap)
{
	this->buildingTemplate = buildingTemplate;
	this->position = position;
	for (int x = 0; x < buildingTemplate->size.x; x++)
		for (int y = 0; y < buildingTemplate->size.y; y++)
			tilemap[position.x + x][position.y + y]->building = this;
}
