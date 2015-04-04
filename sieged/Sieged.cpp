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
#include <blib/BackgroundTask.h>
#include <blib/StaticModel.h>

#include <blib/util/Log.h>
using blib::util::Log;


std::vector<blib::VertexP3T2N3> cube;

Sieged::Sieged()
{
	appSetup.renderer = blib::AppSetup::GlRenderer;
	appSetup.title = "Sieged";
	appSetup.window.setWidth(1920);
	appSetup.window.setHeight(1079);

	appSetup.vsync = false;
	appSetup.joystickDriver = blib::AppSetup::DirectInput;
    
	appSetup.threaded = true;

}

void Sieged::init()
{
	gridTexture = resourceManager->getResource<blib::Texture>("assets/textures/grid.png");
	enemyTexture = resourceManager->getResource<blib::Texture>("assets/textures/enemy.png");
	arrowsTexture = resourceManager->getResource<blib::Texture>("assets/textures/arrows.png");
	conveyorTexture = resourceManager->getResource<blib::Texture>("assets/textures/conveyor.png");
	conveyorTexture->setTextureRepeat(true);
	gridTexture->setTextureRepeat(true);
	font = resourceManager->getResource<blib::Font>("tahoma");

	conveyorBuildingTextureMap = resourceManager->getResource<blib::TextureMap>();
	
	blib::json::Value buildingDb = blib::util::FileSystem::getJson("assets/buildings.json");

	for (const blib::json::Value& b : buildingDb)
	{
		buildingTemplates[(BuildingTemplate::Type)b["id"].asInt()] = new BuildingTemplate(
			(BuildingTemplate::Type)b["id"].asInt() , glm::ivec2(b["size"][0].asInt(), b["size"][1].asInt()),
			conveyorBuildingTextureMap->addTexture(b["beltthumb"]),
			new blib::StaticModel(b["model"], resourceManager, renderer));

		std::string texFile = b["model"];
		texFile = texFile.substr(0, texFile.rfind("."));
		texFile = texFile.substr(0, texFile.rfind("."))+".png";

		buildingTemplates[(BuildingTemplate::Type)b["id"].asInt()]->model->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>(texFile);
	}

	buttons.wall = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnWall.png"), blib::math::Rectangle(glm::vec2(32, 200), 48,48));




	tiles.resize(100, std::vector<Tile*>(100, nullptr));
	for (int x = 0; x < 100; x++)
		for (int y = 0; y < 100; y++)
			tiles[x][y] = new Tile();

//	buildings.push_back(new Building(glm::ivec2(15, 7), buildingTemplates[BuildingTemplate::TownHall], tiles));
//	buildings.push_back(new Building(glm::ivec2(20, 15), buildingTemplates[BuildingTemplate::StoneMason], tiles));


	calcPaths();

	while (enemies.size() < 10)
	{
		glm::vec2 pos(blib::math::randomFloat(0, 32), blib::math::randomFloat(0, 20));
		if (tiles[(int)(pos.x)][(int)(pos.y)]->building)
			continue;
		enemies.push_back(new Enemy(pos));
	}

	for (float i = -0.5f; i <= 0.5f; i += 1)
	{
		cube.push_back(blib::VertexP3T2N3(glm::vec3(i, -0.5f, -0.5f), glm::vec2(0,0), glm::vec3(i, 0, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(i, 0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(i, 0, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(i, -0.5f, 0.5f), glm::vec2(0, 0), glm::vec3(i, 0, 0)));

		cube.push_back(blib::VertexP3T2N3(glm::vec3(i, 0.5f, 0.5f), glm::vec2(0, 0), glm::vec3(i, 0, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(i, 0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(i, 0, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(i, -0.5f, 0.5f), glm::vec2(0, 0), glm::vec3(i, 0, 0)));

		cube.push_back(blib::VertexP3T2N3(glm::vec3(-0.5f, i, -0.5f), glm::vec2(0, 0), glm::vec3(0, i, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(0.5f, i, -0.5f), glm::vec2(0, 0), glm::vec3(0, i, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(-0.5f, i, 0.5f), glm::vec2(0, 0), glm::vec3(0, i, 0)));

		cube.push_back(blib::VertexP3T2N3(glm::vec3(0.5f, i, 0.5f), glm::vec2(0, 0), glm::vec3(0, i, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(0.5f, i, -0.5f), glm::vec2(0, 0), glm::vec3(0, i, 0)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(-0.5f, i, 0.5f), glm::vec2(0, 0), glm::vec3(0, i, 0)));

		cube.push_back(blib::VertexP3T2N3(glm::vec3(-0.5f, -0.5f, i), glm::vec2(0, 0), glm::vec3(0, 0, i)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(0.5f, -0.5f, i), glm::vec2(0, 0), glm::vec3(0, 0, i)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(-0.5f, 0.5f, i), glm::vec2(0, 0), glm::vec3(0, 0, i)));

		cube.push_back(blib::VertexP3T2N3(glm::vec3(0.5f, 0.5f, i), glm::vec2(0, 0), glm::vec3(0, 0, i)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(0.5f, -0.5f, i), glm::vec2(0, 0), glm::vec3(0, 0, i)));
		cube.push_back(blib::VertexP3T2N3(glm::vec3(-0.5f, 0.5f, i), glm::vec2(0, 0), glm::vec3(0, 0, i)));
	}



	backgroundShader = resourceManager->getResource<blib::Shader>("simple");
	backgroundShader->bindAttributeLocation("a_position", 0);
	backgroundShader->bindAttributeLocation("a_texcoord", 1);
	backgroundShader->bindAttributeLocation("a_normal", 2);
	backgroundShader->setUniformName(Uniforms::projectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::cameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::colorMult, "colorMult", blib::Shader::UniformType::Vec4);
	backgroundShader->setUniformName(Uniforms::s_texture, "s_texture", blib::Shader::UniformType::Int);
	
	backgroundShader->finishUniformSetup();

	conveyorOffset = 0;
	int i = 0;
	for (auto b : buildingTemplates)
	{
		if (b.second->type == BuildingTemplate::Gate || b.second->type == BuildingTemplate::Wall)
			continue;

		conveyerBuildings.push_back(std::pair<BuildingTemplate*, float>(b.second, 1920.0f + i));
		i += 75;
	}
	draggingBuilding = NULL;
	conveyerDragIndex = -1;


	cameraCenter = glm::vec3(16, 0, 15);
	cameraAngle = 70;
	cameraDistance = 30;
	cameraRotation = 0;
}

void Sieged::update(double elapsedTime)
{
	if (elapsedTime > 0.1)
		elapsedTime = 0.1;
	if (keyState.isPressed(blib::Key::ESC))
	{
		running = false;
		return;
	}

	cameraDistance -= (mouseState.scrollPosition - prevMouseState.scrollPosition) / 100.0f;
	cameraDistance = glm::clamp(cameraDistance, 5.0f, 100.0f);
	if (mouseState.rightButton)
	{
		if (!keyState.isPressed(blib::Key::SHIFT))
		{
			glm::vec2 diff = glm::vec2(mouseState.position - prevMouseState.position) * 0.05f;
			diff = glm::length(diff) * blib::util::fromAngle(atan2(diff.y, diff.x) + glm::radians(cameraRotation));
				 
			cameraCenter -= glm::vec3(diff.x, 0, diff.y);
		}
		else
		{
			cameraRotation += (mouseState.position.x - prevMouseState.position.x) / 3.0f;
			cameraAngle += (mouseState.position.y - prevMouseState.position.y) / 3.0f;
			cameraAngle = glm::clamp(cameraAngle, 10.0f, 90.0f);
		}
	}


	for (Enemy* e : enemies)
	{
		glm::ivec2 tile = glm::ivec2(e->position);
		if (tile.x < 0 || tile.y < 0)
			continue;
		int direction = tiles[tile.x][tile.y]->toBase;

		glm::vec2 oldPos = e->position;


		if ((direction & Tile::Left) != 0)
			e->position.x -= elapsedTime * e->speed;
		if (tiles[(int)(e->position.x)][(int)(e->position.y)]->building)
			e->position = oldPos;
		oldPos = e->position;
		if ((direction & Tile::Right) != 0)
			e->position.x += elapsedTime * e->speed;
		if (tiles[(int)(e->position.x)][(int)(e->position.y)]->building)
			e->position = oldPos;
		oldPos = e->position;
		if ((direction & Tile::Down) != 0)
			e->position.y += elapsedTime * e->speed;
		if (tiles[(int)(e->position.x)][(int)(e->position.y)]->building)
			e->position = oldPos;
		oldPos = e->position;
		if ((direction & Tile::Up) != 0)
			e->position.y -= elapsedTime * e->speed;

		if (tiles[(int)(e->position.x)][(int)(e->position.y)]->building)
			e->position = oldPos;
		oldPos = e->position;


		if (direction == 0) // oops
		{
			//find nearest wall
			glm::vec2 closestPoint;
			for (const blib::math::Polygon &p : collisionWalls)
			{
				for (size_t i = 0; i < p.size(); i++)
				{
					int ii = (i + 1) % p.size();
					glm::vec2 point = blib::math::Line(p[i], p[ii]).project(e->position);
					if (glm::distance(point, e->position) < glm::distance(closestPoint, e->position))
						closestPoint = point;
				}
			}
			oldPos = e->position = closestPoint;
		}


		for (auto ee : enemies)
		{
			if (e == ee)
				continue;
			glm::vec2 diff = ee->position - e->position;
			float len = glm::length(diff);
			if (len < 0.2f && len > 0.001f)
			{
				diff /= len;
				e->position += (0.2f - len) * -0.5f * diff;
				ee->position += (0.2f - len) * 0.5f * diff;
			}
		}
		if (tiles[(int)(e->position.x)][(int)(e->position.y)]->building)
			e->position = oldPos;
		oldPos = e->position;

	}
	




	conveyorOffset += elapsedTime * conveyerSpeed;
	while (conveyorOffset > 128)
		conveyorOffset -= 128;

	for (size_t i = 0; i < conveyerBuildings.size(); i++)
	{
		conveyerBuildings[i].second = glm::max(conveyerBuildings[i].second - (float)elapsedTime * conveyerSpeed, 64.0f * i);
	}


	if (mouseState.leftButton && !prevMouseState.leftButton)
	{
		if (mouseState.position.y > 1080 - 128)
		{
			for (size_t i = 0; i < conveyerBuildings.size(); i++)
			{
				if (blib::math::Rectangle(glm::vec2(conveyerBuildings[i].second, 1080 - 128 + 32), 64, 64).contains(glm::vec2(mouseState.position)))
				{
					draggingBuilding = conveyerBuildings[i].first;
					conveyerDragIndex = i;
				}
			}

		}
		else
		{
			mousePos3dBegin = mousePos3d;
			if (buttons.wall->contains(glm::vec2(mouseState.position)))
			{
				if (buttons.wall->animations.empty())
				{
					buttons.wall->resizeTo(glm::vec2(1.25f, 1.25f), 0.1f, [this]()
					{
						buttons.wall->resizeTo(glm::vec2(0.8f, 0.8f), 0.1f);
					});
				}
				if (mode == BuildMode::Wall)
					mode = BuildMode::Normal;
				else
					mode = BuildMode::Wall;
			}
		}
	}

	if (!mouseState.leftButton && prevMouseState.leftButton)
	{
		if (draggingBuilding)
		{
			if (mouseState.position.y < 1080 - 128)
			{
				buildings.push_back(new Building(glm::ivec2(mousePos3d.x - draggingBuilding->size.x/2, mousePos3d.z - draggingBuilding->size.y/2), draggingBuilding, tiles));
				conveyerBuildings.erase(conveyerBuildings.begin() + conveyerDragIndex);
				calcPaths();
			}
			draggingBuilding = NULL;
		}
	}


	buttons.wall->update(elapsedTime);

	prevMouseState = mouseState;
}















void Sieged::draw()
{
	renderer->clear(glm::vec4(0, 1, 0, 1), blib::Renderer::Color | blib::Renderer::Depth);

	glm::mat4 projectionMatrix = glm::perspective(45.0f, 1920.0f / 1080.0f, 0.1f, 500.0f);

	glm::mat4 cameraMatrix;
	cameraMatrix = glm::rotate(cameraMatrix, -10.0f, glm::vec3(1, 0, 0));
	cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0, 0, -cameraDistance));
	cameraMatrix = glm::rotate(cameraMatrix, cameraAngle, glm::vec3(1, 0, 0));

	cameraMatrix = glm::rotate(cameraMatrix, cameraRotation, glm::vec3(0, 1, 0));
	cameraMatrix = glm::translate(cameraMatrix, -cameraCenter);



	std::vector<blib::VertexP3T2N3> verts;
	verts.push_back(blib::VertexP3T2N3(glm::vec3(0, 0, 0), glm::vec2(0, 0), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(100, 0, 0), glm::vec2(100/8.0f, 0), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(0, 0, 100), glm::vec2(0, 100 / 8.0f), glm::vec3(0, 1, 0)));

	verts.push_back(blib::VertexP3T2N3(glm::vec3(100, 0, 100), glm::vec2(100 / 8.0f, 100 / 8.0f), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(100, 0, 0), glm::vec2(100 / 8.0f, 0), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(0, 0, 100), glm::vec2(0, 100 / 8.0f), glm::vec3(0, 1, 0)));

	renderState.depthTest = true;
	renderState.activeShader = backgroundShader;
	renderState.activeShader->setUniform(Uniforms::cameraMatrix, cameraMatrix);
	renderState.activeShader->setUniform(Uniforms::projectionMatrix, projectionMatrix);
	renderState.activeShader->setUniform(Uniforms::modelMatrix, glm::mat4());
	renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1,1,1,1));
	renderState.activeTexture[0] = gridTexture;
	renderer->drawTriangles(verts, renderState);



	renderer->unproject(glm::vec2(mouseState.position), &mousePos3d, NULL, cameraMatrix, projectionMatrix);



	for (int x = 0; x < 100; x++)
	{
		for (int y = 0; y < 100; y++)
		{
			if (tiles[x][y]->building == (Building*)1)
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


	for (auto b : buildings)
	{
		glm::mat4 mat;
		//mat = glm::scale(mat, glm::vec3(2, 1, 2));
		mat = glm::translate(mat, glm::vec3(b->position.x + b->buildingTemplate->size.x / 2.0f, 0, b->position.y + b->buildingTemplate->size.y / 2.0f));
		mat = glm::rotate(mat, 180.0f, glm::vec3(0, 1, 0));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
		b->buildingTemplate->model->draw(renderState, renderer, -1);
	}

	if(draggingBuilding)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3((int)mousePos3d.x + (draggingBuilding->size.x % 2 == 0 ? 0 : 0.5f), 0, (int)mousePos3d.z + (draggingBuilding->size.y % 2 == 0 ? 0 : 0.5f)));
		mat = glm::rotate(mat, 180.0f, glm::vec3(0, 1, 0));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
		draggingBuilding->model->draw(renderState, renderer, -1);
	}

	if (!collisionWalls.empty())
	{
		std::vector<blib::VertexP3N3C4> lineVerts;
		for (blib::math::Polygon& e : collisionWalls)
		{
			for (int i = 0; i < e.size(); i++)
			{
				int ii = (i + 1) % e.size();
				lineVerts.push_back(blib::VertexP3N3C4(glm::vec3(e[i].x, 1, e[i].y), glm::vec3(0, 1, 0), glm::vec4(0, 0, 1, 1)));
				lineVerts.push_back(blib::VertexP3N3C4(glm::vec3(e[ii].x, 1, e[ii].y), glm::vec3(0, 1, 0), glm::vec4(0, 0, 1, 1)));
			}
		}
		renderState.activeShader->setUniform(Uniforms::modelMatrix, glm::mat4());
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		renderer->drawLines(lineVerts, renderState);
	}


	if (mode == BuildMode::Wall)
	{
		if (!mouseState.leftButton)
		{
			glm::mat4 mat;
			//mat = glm::scale(mat, glm::vec3(2, 1, 2));
			mat = glm::translate(mat, glm::vec3((int)mousePos3d.x + 0.5f, 0.5f, (int)mousePos3d.z + 0.5f));
			renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
			renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
			renderer->drawTriangles(cube, renderState);
		}
		else
		{
			glm::vec4 minValues = glm::min(mousePos3d, mousePos3dBegin);
			glm::vec4 maxValues = glm::max(mousePos3d, mousePos3dBegin);
			
			glm::vec4 diff = maxValues - minValues;
			glm::mat4 mat;

			if (abs(diff.x) > abs(diff.z))
			{
				diff.z = 0;
				diff.x = (int)diff.x+1;
			}
			else
			{
				diff.x = 0;
				diff.z = (int)diff.z+1;
			}
			
			glm::vec4 center = glm::vec4(glm::ivec4(minValues)) + diff * 0.5f;
			if (abs(diff.x) > abs(diff.z))
				center.z += 0.5f;
			else
				center.x += 0.5f;

			mat = glm::translate(mat, glm::vec3(center.x, 0.5f, center.z));
			mat = glm::scale(mat, glm::vec3(glm::max(1.0f, diff.x), 1, glm::max(1.0f, diff.z)));
			renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
			renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
			renderer->drawTriangles(cube, renderState);
		}

	}






	spriteBatch->begin();

	for (int i = -128; i < 1920+128; i+=128)
		spriteBatch->draw(conveyorTexture, blib::math::easyMatrix(glm::vec2(-conveyorOffset + i, 1080 - 128)));

	for (auto b : conveyerBuildings)
		spriteBatch->draw(b.first->texInfo, blib::math::easyMatrix(glm::vec2(b.second, 1080 - 128+32)));


	buttons.wall->draw(spriteBatch);

	spriteBatch->draw(font, "Enemies: " + std::to_string(enemies.size()), blib::math::easyMatrix(glm::vec2(1, 129)), blib::Color::black);
	spriteBatch->draw(font, "Enemies: " + std::to_string(enemies.size()), blib::math::easyMatrix(glm::vec2(0, 128)));

	spriteBatch->draw(font, "Mouse: " + std::to_string(mousePos3d.x) + ", " + std::to_string(mousePos3d.y) + ", " + std::to_string(mousePos3d.z), blib::math::easyMatrix(glm::vec2(1, 141)), blib::Color::black);
	spriteBatch->draw(font, "Mouse: " + std::to_string(mousePos3d.x) + ", " + std::to_string(mousePos3d.y) + ", " + std::to_string(mousePos3d.z), blib::math::easyMatrix(glm::vec2(0, 140)));

	spriteBatch->end();

}

void Sieged::calcPaths()
{
	if (buildings.empty())
		return;
	double beginTime = blib::util::Profiler::getAppTime();

	new blib::BackgroundTask<std::vector<std::vector<float>>>(this, [=, this]()
	{
		double beginTime_ = blib::util::Profiler::getAppTime();
		std::vector<std::vector<float>> costs(100, std::vector<float>(100, 9999999));
		std::list<glm::ivec2> queue;
		for (int x = 0; x < buildings[0]->buildingTemplate->size.x; x++)
			for (int y = 0; y < buildings[0]->buildingTemplate->size.y; y++)
				queue.push_back(buildings[0]->position + glm::ivec2(x, y));

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
		Log::out << "Finding paths: " << (blib::util::Profiler::getAppTime() - beginTime_) << " s " << Log::newline;
		return costs;
	}, [=, this](const std::vector<std::vector<float>>& costs)
	{
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
		ClipperLib::Clipper clipper;
		ClipperLib::Polygons subject;
		ClipperLib::Polygons result;

		for (int x = 0; x < 100; x++)
		{
			for (int y = 0; y < 100; y++)
			{
				if (!tiles[x][y]->building)
					continue;
				subject.push_back(blib::math::Polygon({
					glm::vec2(x - 0.05f, y - 0.05f),
					glm::vec2(x + 1.05f, y - 0.05f),
					glm::vec2(x + 1.05f, y + 1.05f),
					glm::vec2(x - 0.05f, y + 1.05f),
				}).toClipperPolygon());
			}
		}

		clipper.AddPolygons(subject, ClipperLib::ptClip);
		clipper.Execute(ClipperLib::ctUnion, result, ClipperLib::pftNonZero, ClipperLib::pftNonZero);

		collisionWalls.clear();
		for (ClipperLib::Polygon& p : result)
			collisionWalls.push_back(p);

	});




}











Building::Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap)
{
	this->buildingTemplate = buildingTemplate;
	this->position = position;
	for (int x = 0; x < buildingTemplate->size.x; x++)
		for (int y = 0; y < buildingTemplate->size.y; y++)
			tilemap[position.x + x][position.y + y]->building = this;
}
