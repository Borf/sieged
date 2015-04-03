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
#include <blib/util/Profiler.h>

#include <blib/util/Log.h>
using blib::util::Log;


std::vector<blib::VertexP3N3C4> cube;

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
	conveyorTexture = resourceManager->getResource<blib::Texture>("assets/textures/conveyor.png");
	conveyorTexture->setTextureRepeat(true);
	font = resourceManager->getResource<blib::Font>("tahoma");

	conveyorBuildingTextureMap = resourceManager->getResource<blib::TextureMap>();

	buildingTemplates[BuildingTemplate::TownHall] = new BuildingTemplate(BuildingTemplate::TownHall, glm::ivec2(2, 2), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/TownHall.png"));
	buildingTemplates[BuildingTemplate::StoneMason] = new BuildingTemplate(BuildingTemplate::StoneMason, glm::ivec2(2, 3), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/StoneMason.png"));
	buildingTemplates[BuildingTemplate::Farm] = new BuildingTemplate(BuildingTemplate::Farm, glm::ivec2(), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/Farm.png"));
	buildingTemplates[BuildingTemplate::MarketPlace] = new BuildingTemplate(BuildingTemplate::MarketPlace, glm::ivec2(4, 5), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/MarketPlace.png"));
	buildingTemplates[BuildingTemplate::ArcheryRange] = new BuildingTemplate(BuildingTemplate::ArcheryRange, glm::ivec2(3, 6), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/ArcheryRange.png"));
	buildingTemplates[BuildingTemplate::WizardTower] = new BuildingTemplate(BuildingTemplate::WizardTower, glm::ivec2(), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/WizardTower.png"));
	buildingTemplates[BuildingTemplate::Smithy] = new BuildingTemplate(BuildingTemplate::Smithy, glm::ivec2(), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/Smithy.png"));
	buildingTemplates[BuildingTemplate::Tavern] = new BuildingTemplate(BuildingTemplate::Tavern, glm::ivec2(), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/Tavern.png"));
	buildingTemplates[BuildingTemplate::WatchTower] = new BuildingTemplate(BuildingTemplate::WatchTower, glm::ivec2(), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/WatchTower.png"));
	buildingTemplates[BuildingTemplate::AlchemyLabs] = new BuildingTemplate(BuildingTemplate::AlchemyLabs, glm::ivec2(), conveyorBuildingTextureMap->addTexture("assets/textures/buildings/AlchemyLabs.png"));

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
		glm::vec2 pos(blib::math::randomFloat(0, 32), blib::math::randomFloat(0, 20));
		if (tiles[(int)(pos.x)][(int)(pos.y)]->building)
			continue;
		enemies.push_back(new Enemy(pos));
	}

	glm::vec4 color(1, 1, 1, 1);

	for (float i = -0.5f; i <= 0.5f; i += 1)
	{
		cube.push_back(blib::VertexP3N3C4(glm::vec3(i, -0.5f, -0.5f), glm::vec3(i, 0, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(i, 0.5f, -0.5f), glm::vec3(i, 0, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(i, -0.5f, 0.5f), glm::vec3(i, 0, 0), color));

		cube.push_back(blib::VertexP3N3C4(glm::vec3(i, 0.5f, 0.5f), glm::vec3(i, 0, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(i, 0.5f, -0.5f), glm::vec3(i, 0, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(i, -0.5f, 0.5f), glm::vec3(i, 0, 0), color));

		cube.push_back(blib::VertexP3N3C4(glm::vec3(-0.5f, i, -0.5f), glm::vec3(0, i, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(0.5f, i, -0.5f), glm::vec3(0, i, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(-0.5f, i, 0.5f), glm::vec3(0, i, 0), color));

		cube.push_back(blib::VertexP3N3C4(glm::vec3(0.5f, i, 0.5f), glm::vec3(0, i, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(0.5f, i, -0.5f), glm::vec3(0, i, 0), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(-0.5f, i, 0.5f), glm::vec3(0, i, 0), color));

		cube.push_back(blib::VertexP3N3C4(glm::vec3(-0.5f, -0.5f, i), glm::vec3(0, 0, i), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(0.5f, -0.5f, i), glm::vec3(0, 0, i), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(-0.5f, 0.5f, i), glm::vec3(0, 0, i), color));

		cube.push_back(blib::VertexP3N3C4(glm::vec3(0.5f, 0.5f, i), glm::vec3(0, 0, i), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(0.5f, -0.5f, i), glm::vec3(0, 0, i), color));
		cube.push_back(blib::VertexP3N3C4(glm::vec3(-0.5f, 0.5f, i), glm::vec3(0, 0, i), color));
	}



	backgroundShader = resourceManager->getResource<blib::Shader>("simple");
	backgroundShader->bindAttributeLocation("a_position", 0);
	backgroundShader->bindAttributeLocation("a_normal", 1);
	backgroundShader->bindAttributeLocation("a_color", 2);
	backgroundShader->setUniformName(Uniforms::projectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::cameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::colorMult, "colorMult", blib::Shader::UniformType::Vec4);
	
	backgroundShader->finishUniformSetup();

	conveyorOffset = 0;
	conveyerBuildings.push_back(std::pair<BuildingTemplate*, float>(buildingTemplates[BuildingTemplate::TownHall], 1920.0f));

	cameraCenter = glm::vec3(16, 0, 15);
	cameraAngle = 70;
	cameraDistance = 30;
	cameraRotation = 0;
}

void Sieged::update(double elapsedTime)
{
	if (keyState.isPressed(blib::Key::ESC))
	{
		running = false;
		return;
	}

	cameraDistance -= (mouseState.scrollPosition - prevMouseState.scrollPosition) / 100.0f;
	cameraDistance = glm::clamp(cameraDistance, 5.0f, 100.0f);
	if (mouseState.rightButton)
	{
		cameraRotation += (mouseState.position.x - prevMouseState.position.x) / 3.0f;
		cameraAngle += (mouseState.position.y - prevMouseState.position.y) / 3.0f;
		cameraAngle = glm::clamp(cameraAngle, 10.0f, 90.0f);
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
		glm::ivec2 tile = glm::ivec2(e->position);
		if (tile.x < 0 || tile.y < 0)
			continue;
		int direction = tiles[tile.x][tile.y]->toBase;

		glm::vec2 oldPos = e->position;

		if ((direction & Tile::Left) != 0)
			e->position.x -= elapsedTime * 1;
		if ((direction & Tile::Right) != 0)
			e->position.x += elapsedTime * 1;
		if ((direction & Tile::Down) != 0)
			e->position.y += elapsedTime * 1;
		if ((direction & Tile::Up) != 0)
			e->position.y -= elapsedTime * 1;

		if (tiles[(int)(e->position.x / 64)][(int)(e->position.y / 64)]->building)
			e->position = oldPos;


		for (auto ee : enemies)
		{
			if (e == ee)
				continue;
			glm::vec2 diff = ee->position - e->position;
			float len = glm::length(diff);
			if (len < 0.1f && len > 0.001f)
			{
				diff /= len;
				e->position += (0.1f - len) * -0.5f * diff;
				ee->position += (0.1f - len) * 0.5f * diff;
			}
		}
		
	}
	


	conveyorOffset += elapsedTime * conveyerSpeed;
	while (conveyorOffset > 128)
		conveyorOffset -= 128;

	for (size_t i = 0; i < conveyerBuildings.size(); i++)
	{
		conveyerBuildings[i].second = glm::max(conveyerBuildings[i].second - (float)elapsedTime * conveyerSpeed, 128.0f * i);
	}

	prevMouseState = mouseState;
}



void Sieged::draw()
{
	renderer->clear(glm::vec4(0, 1, 0, 1), blib::Renderer::Color | blib::Renderer::Depth);


//	spriteBatch->begin();
	
/*	for (int x = 0; x < 32; x++)
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
	*/


	glm::mat4 cameraMatrix;
	cameraMatrix = glm::rotate(cameraMatrix, -10.0f, glm::vec3(1, 0, 0));
	cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0, 0, -cameraDistance));
	cameraMatrix = glm::rotate(cameraMatrix, cameraAngle, glm::vec3(1, 0, 0));

	cameraMatrix = glm::rotate(cameraMatrix, cameraRotation, glm::vec3(0, 1, 0));
	cameraMatrix = glm::translate(cameraMatrix, -cameraCenter);



	std::vector<blib::VertexP3N3C4> verts;
	verts.push_back(blib::VertexP3N3C4(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1)));
	verts.push_back(blib::VertexP3N3C4(glm::vec3(100, 0, 0), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1)));
	verts.push_back(blib::VertexP3N3C4(glm::vec3(0, 0, 100), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1)));

	verts.push_back(blib::VertexP3N3C4(glm::vec3(100, 0, 100), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1)));
	verts.push_back(blib::VertexP3N3C4(glm::vec3(100, 0, 0), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1)));
	verts.push_back(blib::VertexP3N3C4(glm::vec3(0, 0, 100), glm::vec3(0, 1, 0), glm::vec4(1, 1, 1, 1)));

	renderState.depthTest = true;
	renderState.activeShader = backgroundShader;
	renderState.activeShader->setUniform(Uniforms::cameraMatrix, cameraMatrix);
	renderState.activeShader->setUniform(Uniforms::projectionMatrix, glm::perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 500.0f));
	renderState.activeShader->setUniform(Uniforms::modelMatrix, glm::mat4());
	renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1,1,1,1));
	renderer->drawTriangles(verts, renderState);




	for (int x = 0; x < 100; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			if (tiles[x][y]->building)
			{
				glm::mat4 mat;
				//mat = glm::scale(mat, glm::vec3(2, 1, 2));
				mat = glm::translate(mat, glm::vec3(x + 0.5f, 0.5f, y + 0.5f));
				renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
				renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 0, 1));
				renderer->drawTriangles(cube, renderState);
			}
		}
	}


	for (auto e : enemies)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(e->position.x, 0.5f, e->position.y));
		mat = glm::scale(mat, glm::vec3(0.1f, 2, 0.1f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 0, 0, 1));
		renderer->drawTriangles(cube, renderState);
	}




	spriteBatch->begin();

	for (int i = -128; i < 1920+128; i+=128)
		spriteBatch->draw(conveyorTexture, blib::math::easyMatrix(glm::vec2(-conveyorOffset + i, 1080 - 128)));

	for (auto b : conveyerBuildings)
		spriteBatch->draw(b.first->texInfo, blib::math::easyMatrix(glm::vec2(b.second, 1080 - 128+32)));


	spriteBatch->draw(font, "Enemies: " + std::to_string(enemies.size()), blib::math::easyMatrix(glm::vec2(1, 129)), blib::Color::black);
	spriteBatch->draw(font, "Enemies: " + std::to_string(enemies.size()), blib::math::easyMatrix(glm::vec2(0, 128)));
	spriteBatch->end();

}

void Sieged::calcPaths()
{
	double beginTime = blib::util::Profiler::getAppTime();
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


	Log::out << "Path calculations: " << (blib::util::Profiler::getAppTime() - beginTime) << " s " << Log::newline;

}











Building::Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap)
{
	this->buildingTemplate = buildingTemplate;
	this->position = position;
	for (int x = 0; x < buildingTemplate->size.x; x++)
		for (int y = 0; y < buildingTemplate->size.y; y++)
			tilemap[position.x + x][position.y + y]->building = this;
}
