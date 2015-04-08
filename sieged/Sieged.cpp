#include "Sieged.h"

#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <set>

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
	whitePixel = resourceManager->getResource<blib::Texture>("assets/textures/whitePixel.png");
	gridTexture = resourceManager->getResource<blib::Texture>("assets/textures/grid.png");
	enemyTexture = resourceManager->getResource<blib::Texture>("assets/textures/enemy.png");
	arrowsTexture = resourceManager->getResource<blib::Texture>("assets/textures/arrows.png");
	conveyorTexture = resourceManager->getResource<blib::Texture>("assets/textures/conveyor.png");
	conveyorTexture->setTextureRepeat(true);
	gridTexture->setTextureRepeat(true);
	font = resourceManager->getResource<blib::Font>("tahoma");
	font48 = resourceManager->getResource<blib::Font>("main48");
	conveyorBuildingTextureMap = resourceManager->getResource<blib::TextureMap>();

	notEnoughGoldTexture = resourceManager->getResource<blib::Texture>("assets/textures/notenoughgold.png");



	blib::json::Value settings = blib::util::FileSystem::getJson("assets/settings.json");
	conveyorBuildingsPerSecond = settings["blueprintspersecond"];
	stoneMasonFactor = settings["stonemasonfactor"];
	gold = settings["start"]["gold"];

	
	blib::json::Value buildingDb = blib::util::FileSystem::getJson("assets/buildings.json");
	for (const blib::json::Value& b : buildingDb)
	{
		blib::StaticModel* m = NULL;
		if (b["model"].asString() != "")
			m = new blib::StaticModel(b["model"], resourceManager, renderer);

		buildingTemplates[(BuildingTemplate::Type)b["id"].asInt()] = new BuildingTemplate(
			b, conveyorBuildingTextureMap,
			m);

		std::string texFile = b["model"];
		texFile = texFile.substr(0, texFile.rfind("."));
		texFile = texFile.substr(0, texFile.rfind("."))+".png";

		if (m)
			buildingTemplates[(BuildingTemplate::Type)b["id"].asInt()]->model->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>(texFile);
	}
	rngTotalWeight = blib::linq::sum<int>(buildingTemplates, [](std::pair < BuildingTemplate::Type, BuildingTemplate*> item){ return glm::max(0, item.second->rngWeight); });


	for (int i = 0; i < 6; i++)
	{
		wallModels[i] = new blib::StaticModel("assets/models/wall"+std::to_string(i+1)+".fbx.json", resourceManager, renderer);
		wallModels[i]->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/wall"+std::to_string(i+1)+".png");
	}

	enemyModel = new blib::StaticModel("assets/models/cube.fbx.json", resourceManager, renderer);
	enemyModel->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/cube.png");

	dudeModel= new blib::StaticModel("assets/models/cube.fbx.json", resourceManager, renderer);
	dudeModel->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/cube.png");

	flagModel = new blib::StaticModel("assets/models/flag.fbx.json", resourceManager, renderer);
	flagModel->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/flag.png");

	buttons.wall = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnWall.png"), blib::math::Rectangle(glm::vec2(16, 200), 48, 48));
	buttons.market = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnMarket.png"), blib::math::Rectangle(glm::vec2(16, 248), 48, 48));
	buttons.flag = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnFlag.png"), blib::math::Rectangle(glm::vec2(16, 296), 48, 48));
	buttons.soldiers = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnSoldiers.png"), blib::math::Rectangle(glm::vec2(16, 344), 48, 48));
	buttons.archers = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnArchers.png"), blib::math::Rectangle(glm::vec2(16, 392), 48, 48));

	buttons.wall->color = glm::vec4(1, 1, 1, 0);




	tiles.resize(100, std::vector<Tile*>(100, nullptr));
	for (int x = 0; x < 100; x++)
		for (int y = 0; y < 100; y++)
			tiles[x][y] = new Tile();


	flowMap.targetBuilding = (Building*)1;
	flowmaps.push_back(&flowMap);

	calcPaths();
	calcWalls();

/*	while (enemies.size() < 10)
	{
		glm::vec2 pos(blib::math::randomFloat(0, 32), blib::math::randomFloat(0, 20));
		if (tiles[(int)(pos.x)][(int)(pos.y)]->building)
			continue;
		enemies.push_back(new Enemy(pos));
	}*/



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

	shadowMap = resourceManager->getResource<blib::FBO>();
	shadowMap->setSize(4048*2, 4048*2);
	shadowMap->depth = false;
	shadowMap->depthTexture = true;
	shadowMap->stencil = false;
	shadowMap->textureCount = 0;



	backgroundShader = resourceManager->getResource<blib::Shader>("simple");
	backgroundShader->bindAttributeLocation("a_position", 0);
	backgroundShader->bindAttributeLocation("a_texcoord", 1);
	backgroundShader->bindAttributeLocation("a_normal", 2);
	backgroundShader->setUniformName(Uniforms::ProjectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::CameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::colorMult, "colorMult", blib::Shader::UniformType::Vec4);
	backgroundShader->setUniformName(Uniforms::s_texture, "s_texture", blib::Shader::UniformType::Int);
	backgroundShader->setUniformName(Uniforms::s_shadowmap, "s_shadowmap", blib::Shader::UniformType::Int);
	backgroundShader->setUniformName(Uniforms::buildFactor, "buildFactor", blib::Shader::UniformType::Float);
	backgroundShader->setUniformName(Uniforms::location, "location", blib::Shader::UniformType::Vec2);
	backgroundShader->setUniformName(Uniforms::shadowProjectionMatrix, "shadowProjectionMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::shadowCameraMatrix, "shadowCameraMatrix", blib::Shader::UniformType::Mat4);
	backgroundShader->setUniformName(Uniforms::lightDirection, "lightDirection", blib::Shader::UniformType::Vec3);
	backgroundShader->setUniformName(Uniforms::shadowFac, "shadowFac", blib::Shader::UniformType::Float);
	
	backgroundShader->finishUniformSetup();
	backgroundShader->setUniform(Uniforms::s_texture, 0);
	backgroundShader->setUniform(Uniforms::s_shadowmap, 1);

	shadowmapShader = resourceManager->getResource<blib::Shader>("shadowmap");
	shadowmapShader->bindAttributeLocation("a_position", 0);
	shadowmapShader->bindAttributeLocation("a_texcoord", 1);
	shadowmapShader->setUniformName(Uniforms::ProjectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	shadowmapShader->setUniformName(Uniforms::CameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	shadowmapShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	shadowmapShader->setUniformName(Uniforms::buildFactor, "buildFactor", blib::Shader::UniformType::Float);
	shadowmapShader->setUniformName(Uniforms::location, "location", blib::Shader::UniformType::Vec2);
	shadowmapShader->finishUniformSetup();


	renderState.depthTest = true;
	renderState.blendEnabled = true;
	renderState.srcBlendColor = blib::RenderState::SRC_ALPHA;
	renderState.srcBlendAlpha = blib::RenderState::SRC_ALPHA;
	renderState.dstBlendColor = blib::RenderState::ONE_MINUS_SRC_ALPHA;
	renderState.dstBlendAlpha = blib::RenderState::ONE_MINUS_SRC_ALPHA;
	renderState.activeShader = backgroundShader;



	conveyorOffset = 0;
	lastConveyorBuilding = 1 / conveyorBuildingsPerSecond;
	draggingBuilding = NULL;
	conveyorDragIndex = -1;
	conveyorBuildings.push_back(std::pair<BuildingTemplate*, float>(buildingTemplates[BuildingTemplate::TownHall], 1920.0f));


	cameraCenter = glm::vec3(50, 0, 50);
	cameraAngle = 70;
	cameraDistance = 30;
	cameraRotation = 0;
	threatLevel = 0;
	pathCalculateThread = NULL;
	lightDirection = 45.0f;
}

void Sieged::update(double elapsedTime)
{
	if (elapsedTime > 0.1)
		elapsedTime = 0.1;

	elapsedTime *= speed;

	if (keyState.isPressed(blib::Key::ESC))
	{
		running = false;
		while (pathCalculateThread)
		{
			Sleep(0);
		}
		return;
	}

	if (keyState.isPressed(blib::Key::_1))
		speed = 1;
	if (keyState.isPressed(blib::Key::_2))
		speed = 2;
	if (keyState.isPressed(blib::Key::_3))
		speed = 3;
	if (keyState.isPressed(blib::Key::_4))
		speed = 4;
	if (keyState.isPressed(blib::Key::_5))
		speed = 5;
	if (keyState.isPressed(blib::Key::_6))
		speed = 8;
	if (keyState.isPressed(blib::Key::_7))
		speed = 10;


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


	




	conveyorOffset += (float)elapsedTime * conveyorSpeed;
	while (conveyorOffset > 128)
		conveyorOffset -= 128;

	for (size_t i = 0; i < conveyorBuildings.size(); i++)
	{
		conveyorBuildings[i].second = glm::max(conveyorBuildings[i].second - (float)elapsedTime * conveyorSpeed, 64.0f * i);
	}


	if (mouseState.leftButton && !prevMouseState.leftButton)
	{
		if (mouseState.position.y > 1080 - 128)
		{
			for (size_t i = 0; i < conveyorBuildings.size(); i++)
			{
				if (blib::math::Rectangle(glm::vec2(conveyorBuildings[i].second, 1080 - 128 + 32), 64, 64).contains(glm::vec2(mouseState.position)))
				{
					draggingBuilding = conveyorBuildings[i].first;
					conveyorDragIndex = i;
					mode = BuildMode::Normal;
				}
			}

		}
		else
		{
			mousePos3dBegin = mousePos3d;
			for (blib::AnimatableSprite* button : buttons.buttons)
				if (button->contains(glm::vec2(mouseState.position)))
					if (button->animations.empty())
						button->resizeTo(glm::vec2(1.25f, 1.25f), 0.1f, [this, button]()
						{
							button->resizeTo(glm::vec2(0.8f, 0.8f), 0.1f);
						});



			if (buttons.wall->contains(glm::vec2(mouseState.position)))
			{
				if (mode == BuildMode::Wall)
					mode = BuildMode::Normal;
				else
					mode = BuildMode::Wall;
			}
			if (buttons.flag->contains(glm::vec2(mouseState.position)))
			{
				if (mode == BuildMode::Flag)
					mode = BuildMode::Normal;
				else
					mode = BuildMode::Flag;
			}
			if (buttons.soldiers->contains(glm::vec2(mouseState.position)))
			{
				Soldier* f = new Soldier(glm::vec2(50, 50));
				f->flowmap = &flags[0]->flowmap;
				soldiers.push_back(f);

			}
		}
	}

	if (!mouseState.leftButton && prevMouseState.leftButton)
	{
		if (draggingBuilding)
		{
			if (mouseState.position.y < 1080 - 128)
			{
				glm::ivec2 pos(mousePos3d.x - draggingBuilding->size.x / 2, mousePos3d.z - draggingBuilding->size.y / 2);
				bool ok = true;
				for (int x = 0; x < draggingBuilding->size.x; x++)
					for (int y = 0; y < draggingBuilding->size.y; y++)
						if (tiles[pos.x + x][pos.y + y]->building)
							ok = false;
				
				if (ok && gold < draggingBuilding->cost)
				{
					blib::AnimatableSprite* e = new blib::AnimatableSprite(notEnoughGoldTexture, glm::vec2(mouseState.position) - notEnoughGoldTexture->center);
					e->resizeTo(glm::vec2(2,2), 1);
					e->alphaTo(0, 1);
					effects.push_back(e);
					//ok = !ok;
				}


				if (ok)
				{
					gold -= draggingBuilding->cost;
					buildings.push_back(new Building(pos, draggingBuilding, tiles));
					conveyorBuildings.erase(conveyorBuildings.begin() + conveyorDragIndex);
					calcPaths();
				}
			}
			draggingBuilding = NULL;
		}
		if (mode == BuildMode::Wall)
		{
			glm::ivec4 start(mousePos3dBegin);
			glm::ivec4 end(mousePos3d);

			glm::ivec4 diff = end - start;
			if (!buttons.wall->contains(glm::vec2(mouseState.position)))
			{
				if (glm::abs(diff.x) > glm::abs(diff.z))
				{
					if (diff.x != 0)
						diff.x /= abs(diff.x);
					diff.z = 0;
				}
				else
				{
					diff.x = 0;
					if (diff.z != 0)
						diff.z /= abs(diff.z);
				}

				while ((diff.x != 0 && start.x != end.x) || (diff.z != 0 && start.z != end.z))
				{
					if (!tiles[start.x][start.z]->building)
						buildings.push_back(new Building(glm::ivec2(start.x, start.z), buildingTemplates[BuildingTemplate::Wall], tiles));
					start += diff;
				}
				if (!tiles[start.x][start.z]->building)
					buildings.push_back(new Building(glm::ivec2(start.x, start.z), buildingTemplates[BuildingTemplate::Wall], tiles));

				calcWalls();
				calcPaths();
			}
		}
		else if (mode == BuildMode::Flag)
		{
			Flag* f = blib::linq::firstOrDefault<Flag*>(flags, [this](Flag* f) { return f->position == glm::ivec2(mousePos3d.x, mousePos3d.z); }, NULL);
			if (f)
			{
				//TODO: only do this when there is no thread active !!!!!!!!!
				for (std::list<Flowmap*>::iterator it = flowmaps.begin(); it != flowmaps.end(); it++)
					if (*it == &f->flowmap)
					{
						flowmaps.erase(it);
						break;
					}
				blib::linq::deletewhere(flags, [this](Flag* f) { return f->position == glm::ivec2(mousePos3d.x, mousePos3d.z); });
			}
			else
				if (!buttons.flag->contains(glm::vec2(mouseState.position)))
				{
					f = new Flag(glm::ivec2(mousePos3d.x, mousePos3d.z));
					flowmaps.push_back(&f->flowmap);
					flags.push_back(f);
					calcPaths();
				}
		}
	}

	if (blib::linq::contains(buildings, [](Building* b){ return b->buildingTemplate->type == BuildingTemplate::TownHall; }))
	{
		lastConveyorBuilding -= (float)elapsedTime;
		if (lastConveyorBuilding <= 0)
		{
			int rng = rand() % rngTotalWeight;
			bool bla = false;
			for (auto b : buildingTemplates)
			{
				if (b.second->rngWeight <= 0)
					continue;
				if (rng < b.second->rngWeight)
				{
					conveyorBuildings.push_back(std::pair<BuildingTemplate*, float>(b.second, 1920.0f));
					bla = true;
					break;
				}
				rng -= b.second->rngWeight;
			}
			lastConveyorBuilding = 1 / conveyorBuildingsPerSecond;
		}
	}

	for (int i = 0; i < (int)effects.size(); i++)
	{
		effects[i]->update((float)elapsedTime);
		if (effects[i]->animations.empty())
		{
			delete effects[i];
			effects.erase(effects.begin() + i);
			i--;
		}
	}

	if (gamePlaying)
	{
		threatLevel += (float)elapsedTime / 60.0f;

		goldTimeLeft -= (float)elapsedTime;
		while (goldTimeLeft < 0)
		{
			goldTimeLeft += 1;
			gold += (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::MineralMine; })) * 5;
			gold = (int)(gold * (1 + (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::Bank; })) * 0.005f));
		}


		nextEnemySpawn -= (float)elapsedTime;
		if (nextEnemySpawn < 0)
		{
			nextEnemySpawn = 10 / (threatLevel + 1);

			while (true)
			{
				glm::vec2 pos = glm::vec2(50,50) + 49.0f * blib::util::fromAngle(blib::math::randomFloat(0, 2 * blib::math::pif));
				if (tiles[(int)(pos.x)][(int)(pos.y)]->building)
					continue;
				enemies.push_back(new Enemy(pos, &flowMap));
				break;
			}
		}


		for (auto s : soldiers)
		{
			glm::vec2 originalPos = s->position;
			s->updateMovement(elapsedTime, tiles);

			for (auto ee : soldiers)
			{
				if (s == ee)
					continue;
				glm::vec2 diff = ee->position - s->position;
				float len = glm::length(diff);
				if (len < 0.2f && len > 0.001f)
				{
					diff /= len;
					s->position += (0.2f - len) * -0.5f * diff;
					ee->position += (0.2f - len) * 0.5f * diff;
				}
			}
			if (tiles[(int)(s->position.x)][(int)(s->position.y)]->building)
				s->position = originalPos;
			originalPos = s->position;
		}


		for (size_t i = 0; i < enemies.size(); i++)
		{
			Enemy* e = enemies[i];
			
			glm::vec2 originalPos = e->position;
			Building* attackBuilding = e->updateMovement((float)elapsedTime, tiles);
			
			if (originalPos == e->position && !attackBuilding)
			{
				if (tiles[(int)(e->position.x)][(int)(e->position.y)]->building)
					attackBuilding = tiles[(int)(e->position.x)][(int)(e->position.y)]->building;

				glm::vec2 closestPoint;
				Building* closestBuilding = buildings[0];

				for (auto b : buildings)
				{
					blib::math::Rectangle buildRect(glm::vec2(b->position), b->buildingTemplate->size.x, b->buildingTemplate->size.y);
					glm::vec2 projection = buildRect.projectClosest(e->position);
					
					if (glm::distance(e->position, closestPoint) > glm::distance(e->position, projection))
					{
						closestBuilding = b;
						closestPoint = projection;
					}
				}
				attackBuilding = closestBuilding;
			}


			e->timeLeftForAttack = glm::max(0.0f, e->timeLeftForAttack - (float)elapsedTime);
			if (attackBuilding && e->timeLeftForAttack <= 0)
			{
				attackBuilding->damage++;
				e->timeLeftForAttack = 0.1f;
				float buildFactor = 1.0f - glm::min(1.0f, attackBuilding->buildTimeLeft / attackBuilding->buildingTemplate->buildTime);

				if (attackBuilding->damage >= attackBuilding->buildingTemplate->hitpoints * buildFactor)
				{
					Log::out << "Enemy " << i << Log::newline;
					for (int x = 0; x < attackBuilding->buildingTemplate->size.x; x++)
					{
						for (int y = 0; y < attackBuilding->buildingTemplate->size.y; y++)
						{
							tiles[attackBuilding->position.x + x][attackBuilding->position.y + y]->building = NULL;
						}
					}

					for (int i = 0; i < buildings.size(); i++)
					{
						if (buildings[i] == attackBuilding)
						{
							buildings.erase(buildings.begin() + i);
							break;
						}
					}
					delete attackBuilding;
					calcPaths();
					calcWalls();
				}
			}

			/*if (direction == 0) // oops
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
			}*/


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
				e->position = originalPos;
			originalPos = e->position;
		}








	}

	for (auto b : buildings)
	{
		if (b->buildTimeLeft > 0 && b->buildingTemplate->type == BuildingTemplate::Wall)
		{
			b->buildTimeLeft -= (float)elapsedTime * wallBuildSpeed;
			if (b->buildTimeLeft < 0)
			{
				b->buildTimeLeft = 0;
				calcPaths();
			}
			break;
		}
	}
	for (auto b : buildings)
	{
		if (b->buildTimeLeft > 0 && b->buildingTemplate->type != BuildingTemplate::Wall)
		{
			b->buildTimeLeft -= (float)elapsedTime;
			if (b->buildTimeLeft < 0)
			{
				calcPaths();
				b->buildTimeLeft = 0;
				//done with building
				if (b->buildingTemplate->type == BuildingTemplate::StoneMason)
				{
					buttons.wall->alphaTo(1.0f, 1);
					wallBuildSpeed = 1 + (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::StoneMason; }) - 1) * stoneMasonFactor;
				}
				if (b->buildingTemplate->type == BuildingTemplate::TownHall)
				{
					gamePlaying = true;
				}
				if (b->buildingTemplate->type == BuildingTemplate::Barracks)
				{
					maxFlagCount = blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::Barracks; }) * 1;
				}
			}
			break;
		}
	}


	lightDirection += (float)elapsedTime * 0.001f;

	for (blib::AnimatableSprite* button : buttons.buttons)
		button->update((float)elapsedTime);

	prevMouseState = mouseState;
}















void Sieged::draw()
{

	projectionMatrix = glm::perspective(45.0f, 1920.0f / 1080.0f, 1.0f, 250.0f);

	cameraMatrix = glm::mat4();
	cameraMatrix = glm::rotate(cameraMatrix, -10.0f, glm::vec3(1, 0, 0));
	cameraMatrix = glm::translate(cameraMatrix, glm::vec3(0, 0, -cameraDistance));
	cameraMatrix = glm::rotate(cameraMatrix, cameraAngle, glm::vec3(1, 0, 0));

	cameraMatrix = glm::rotate(cameraMatrix, cameraRotation, glm::vec3(0, 1, 0));
	cameraMatrix = glm::translate(cameraMatrix, -cameraCenter);


	glm::vec3 lightAngle(cos(lightDirection) * 3, 2, sin(lightDirection) * 3);

	float fac = 60.0f;
	glm::mat4 shadowProjectionMatrix = glm::ortho<float>(-fac, fac, -fac, fac, -50, 75);
	glm::mat4 shadowCameraMatrix = glm::lookAt(lightAngle + glm::vec3(50,0,50), glm::vec3(50, 0, 50), glm::vec3(0, 1, 0));

	renderState.cullFaces = blib::RenderState::CullFaces::CW;
	renderState.activeShader = shadowmapShader;
	renderState.activeShader->setUniform(Uniforms::CameraMatrix, shadowCameraMatrix);
	renderState.activeShader->setUniform(Uniforms::ProjectionMatrix, shadowProjectionMatrix);

	renderState.activeFbo = shadowMap;
	renderer->setViewPort(0, 0, shadowMap->width, shadowMap->height);
	renderer->clear(glm::vec4(1, 1, 1, 1), blib::Renderer::Color | blib::Renderer::Depth, renderState);
	drawWorld(RenderPass::ShadowMap);

	renderState.activeFbo = NULL;
	renderer->setViewPort(0, 0, 1920, 1079);
	renderer->clear(glm::vec4(0.5f, 0.5f, 0.5f, 1), blib::Renderer::Color | blib::Renderer::Depth, renderState);


	renderState.cullFaces = blib::RenderState::CullFaces::CCW;
	renderState.activeTexture[1] = shadowMap;
	renderState.activeShader = backgroundShader;
	renderState.activeShader->setUniform(Uniforms::shadowCameraMatrix, shadowCameraMatrix);
	renderState.activeShader->setUniform(Uniforms::shadowProjectionMatrix, shadowProjectionMatrix);

	renderState.activeShader->setUniform(Uniforms::CameraMatrix, cameraMatrix);
	renderState.activeShader->setUniform(Uniforms::ProjectionMatrix, projectionMatrix);
	drawWorld(RenderPass::Final);



	spriteBatch->begin();


	for (size_t i = 0; i < enemies.size(); i++)
	{
		Enemy* e = enemies[i];
		glm::vec3 position = glm::project(glm::vec3(e->position.x, 1, e->position.y), cameraMatrix, projectionMatrix, glm::uvec4(0, 0, 1920, 1079));
		spriteBatch->draw(font, std::to_string(i), blib::math::easyMatrix(glm::vec2(position.x, 1079 - position.y), 0, 2));
	}



	for (auto b : buildings)
	{
		if (b->damage == 0 && b->buildTimeLeft == 0)
			continue;

		glm::vec3 position = glm::project(glm::vec3(b->position.x + b->buildingTemplate->size.x / 2.0f, 4, b->position.y + b->buildingTemplate->size.y / 2.0f), cameraMatrix, projectionMatrix, glm::uvec4(0, 0, 1920, 1079));
				
		float barWidth = b->buildingTemplate->healthbarSize / cameraDistance;
		float barHeight = glm::max(300 / cameraDistance, 4.0f);

		float borderSize = glm::round(glm::min(4.0f, 30 / cameraDistance));

		float buildFactor = 1.0f-glm::min(1.0f, b->buildTimeLeft / b->buildingTemplate->buildTime);
		float health = (b->buildingTemplate->hitpoints - b->damage) / (float)b->buildingTemplate->hitpoints * buildFactor;
		float healthBarWidth = (barWidth - 2 * borderSize) * health;
	
		glm::vec4 color = glm::mix(blib::Color::reddish, blib::Color::limeGreen, health);

		spriteBatch->draw(whitePixel, blib::math::easyMatrix(glm::vec2(position.x - barWidth / 2, 1079 - position.y), 0, glm::vec2(barWidth, barHeight)));
		spriteBatch->draw(whitePixel, blib::math::easyMatrix(glm::vec2(position.x - barWidth / 2 + borderSize, 1079 - position.y + borderSize), 0, glm::vec2(healthBarWidth, barHeight - 2 * borderSize)), color);
	}


	//spriteBatch->draw(shadowMap, blib::math::easyMatrix(glm::vec2(250,224), 0, glm::vec2(0.05f, -0.05f)));

	for (int i = -128; i < 1920+128; i+=128)
		spriteBatch->draw(conveyorTexture, blib::math::easyMatrix(glm::vec2(-conveyorOffset + i, 1080 - 128)));

	for (auto b : conveyorBuildings)
		spriteBatch->draw(b.first->texInfo, blib::math::easyMatrix(glm::vec2(b.second, 1080 - 128+32)));

	for (auto e : effects)
		e->draw(spriteBatch);


	for (blib::AnimatableSprite* button : buttons.buttons)
		button->draw(spriteBatch);

	std::string debug = "Enemies: " + std::to_string(enemies.size()) + ", flowmaps: " + std::to_string(flowmaps.size());

	spriteBatch->draw(font, debug, blib::math::easyMatrix(glm::vec2(1, 129)), blib::Color::black);
	spriteBatch->draw(font, debug, blib::math::easyMatrix(glm::vec2(0, 128)));

	spriteBatch->draw(font, "Mouse: " + std::to_string(mousePos3d.x) + ", " + std::to_string(mousePos3d.y) + ", " + std::to_string(mousePos3d.z), blib::math::easyMatrix(glm::vec2(1, 141)), blib::Color::black);
	spriteBatch->draw(font, "Mouse: " + std::to_string(mousePos3d.x) + ", " + std::to_string(mousePos3d.y) + ", " + std::to_string(mousePos3d.z), blib::math::easyMatrix(glm::vec2(0, 140)));

	spriteBatch->draw(font, "Speed: " + std::to_string(speed), blib::math::easyMatrix(glm::vec2(1, 153)), blib::Color::black);
	spriteBatch->draw(font, "Speed: " + std::to_string(speed), blib::math::easyMatrix(glm::vec2(0, 152)));


	spriteBatch->draw(font48, "Gold: " + std::to_string(gold), blib::math::easyMatrix(glm::vec2(1920 - 10 - font48->textlen("Gold: " + std::to_string(gold)), 5)));

	spriteBatch->draw(whitePixel, blib::math::easyMatrix(glm::vec2(1920 - 75, 50), 0, glm::vec2(50, 50)));
	float threatFrac = threatLevel - (int)threatLevel;
	spriteBatch->draw(whitePixel, blib::math::easyMatrix(glm::vec2(1920 - 75, 50), 0, glm::vec2(50 * threatFrac, 50)), blib::Color::pinkishOrange);
	spriteBatch->draw(font48, std::to_string((int)threatLevel), blib::math::easyMatrix(glm::vec2(1920 - 60, 50)), blib::Color::black);
	spriteBatch->draw(font48, std::to_string(flags.size()) + " / " + std::to_string(maxFlagCount), blib::math::easyMatrix(glm::vec2(100, 300)));


	spriteBatch->end();

}




void Sieged::drawWorld(RenderPass renderPass)
{
	std::vector<blib::VertexP3T2N3> verts;
	verts.push_back(blib::VertexP3T2N3(glm::vec3(0, 0, 100), glm::vec2(0, 100 / 8.0f), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(100, 0, 0), glm::vec2(100 / 8.0f, 0), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(0, 0, 0), glm::vec2(0, 0), glm::vec3(0, 1, 0)));

	verts.push_back(blib::VertexP3T2N3(glm::vec3(100, 0, 100), glm::vec2(100 / 8.0f, 100 / 8.0f), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(100, 0, 0), glm::vec2(100 / 8.0f, 0), glm::vec3(0, 1, 0)));
	verts.push_back(blib::VertexP3T2N3(glm::vec3(0, 0, 100), glm::vec2(0, 100 / 8.0f), glm::vec3(0, 1, 0)));

	renderState.activeShader->setUniform(Uniforms::modelMatrix, glm::mat4());
	renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	renderState.activeShader->setUniform(Uniforms::buildFactor, 1.0f);
	renderState.activeShader->setUniform(Uniforms::shadowFac, 1.0f);
	renderState.activeTexture[0] = gridTexture;
	renderer->drawTriangles(verts, renderState);


	if (renderPass == RenderPass::Final)
		renderer->unproject(glm::vec2(mouseState.position), &mousePos3d, NULL, cameraMatrix, projectionMatrix);


	//renderState.activeShader->setUniform(Uniforms::shadowFac, 0.0f);


	for (auto e : enemies)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(e->position.x, 1.0f, e->position.y));
		mat = glm::scale(mat, glm::vec3(0.15f, 2, 0.15f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		//renderer->drawTriangles(cube, renderState);
		enemyModel->draw(renderState, renderer, -1);
	}

	for (auto e : soldiers)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(e->position.x, 1.0f, e->position.y));
		mat = glm::scale(mat, glm::vec3(0.15f, 3, 0.15f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		//renderer->drawTriangles(cube, renderState);
		enemyModel->draw(renderState, renderer, -1);
	}


	for (auto b : buildings)
	{
		if (b->buildingTemplate->type == BuildingTemplate::Wall)
			continue;

		glm::mat4 mat;
		//mat = glm::scale(mat, glm::vec3(2, 1, 2));
		mat = glm::translate(mat, glm::vec3(b->position.x + b->buildingTemplate->size.x / 2.0f, 0, b->position.y + b->buildingTemplate->size.y / 2.0f));
		mat = glm::rotate(mat, 180.0f, glm::vec3(0, 1, 0));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1.0f));
		renderState.activeShader->setUniform(Uniforms::buildFactor, 1 - glm::min(1.0f, b->buildTimeLeft / b->buildingTemplate->buildTime));
		renderState.activeShader->setUniform(Uniforms::location, glm::vec2(b->position));

		b->buildingTemplate->model->draw(renderState, renderer, -1);
	}

	for (const std::tuple<glm::mat4, Building*, blib::StaticModel*> &mm : wallCache)
	{
		renderState.activeShader->setUniform(Uniforms::modelMatrix, std::get<0>(mm));
		renderState.activeShader->setUniform(Uniforms::buildFactor, 1 - glm::min(1.0f, std::get<1>(mm)->buildTimeLeft / std::get<1>(mm)->buildingTemplate->buildTime));
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1.0f));
		renderState.activeShader->setUniform(Uniforms::location, glm::vec2(std::get<1>(mm)->position));
		std::get<2>(mm)->draw(renderState, renderer, -1);
	}
	renderState.activeShader->setUniform(Uniforms::buildFactor, 1.0f);



	renderState.activeShader->setUniform(Uniforms::buildFactor, 1.0f);
	renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1.0f));
	for (Flag* f : flags)
	{
		float height = 0;
		if (tiles[f->position.x][f->position.y]->building && tiles[f->position.x][f->position.y]->building->buildingTemplate->type == BuildingTemplate::Wall)
			height = 1.75;
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(f->position.x + 0.5f, height, f->position.y + 0.5f));
		mat = glm::rotate(mat, 90.0f, glm::vec3(0,1,0));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::location, glm::vec2(f->position));
		flagModel->draw(renderState, renderer, -1);
	}
	



	if (draggingBuilding)
	{
		if (fabs(mousePos3d.y) < 1)
		{
			glm::ivec2 pos(mousePos3d.x - draggingBuilding->size.x / 2, mousePos3d.z - draggingBuilding->size.y / 2);
			bool ok = true;
			for (int x = 0; x < draggingBuilding->size.x; x++)
				for (int y = 0; y < draggingBuilding->size.y; y++)
					if (tiles[pos.x + x][pos.y + y]->building)
						ok = false;

			glm::mat4 mat;
			mat = glm::translate(mat, glm::vec3((int)mousePos3d.x + (draggingBuilding->size.x % 2 == 0 ? 0 : 0.5f), 0, (int)mousePos3d.z + (draggingBuilding->size.y % 2 == 0 ? 0 : 0.5f)));
			mat = glm::rotate(mat, 180.0f, glm::vec3(0, 1, 0));
			renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
			if (ok)
				renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
			else
				renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 0, 0, 0.5f));
			draggingBuilding->model->draw(renderState, renderer, -1);
		}
	}

	if (!collisionWalls.empty() && renderPass == RenderPass::Final)
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
		renderState.depthTest = false;
		if (!mouseState.leftButton)
		{
			glm::mat4 mat;
			mat = glm::translate(mat, glm::vec3((int)mousePos3d.x + 0.5f, 0.05f, (int)mousePos3d.z + 0.5f));
			mat = glm::scale(mat, glm::vec3(1, 0.1f, 1));
			renderState.activeTexture[0] = gridTexture;
			renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
			renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
			renderer->drawTriangles(cube, renderState);
		}
		else
		{
			glm::vec4 diff = mousePos3d - mousePos3dBegin;
			glm::mat4 mat;

			if (abs(diff.x) > abs(diff.z))
			{
				diff.z = 0;
				diff.x = glm::round(diff.x);
			}
			else
			{
				diff.x = 0;
				diff.z = glm::round(diff.z);
			}

			glm::vec4 center = glm::vec4(glm::ivec4(mousePos3dBegin)) + diff * 0.5f + glm::vec4(0.5f, 0, 0.5f, 0);
			//if (abs(diff.x) > abs(diff.z))
			//	center.z += 0.5f;
			//else
			//	center.x += 0.5f;

			mat = glm::translate(mat, glm::vec3(center.x, 0.05f, center.z));
			mat = glm::scale(mat, glm::vec3(glm::max(1.0f, abs(diff.x) + 1), 0.1f, glm::max(1.0f, abs(diff.z) + 1)));
			renderState.activeTexture[0] = gridTexture;
			renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
			renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
			renderer->drawTriangles(cube, renderState);
		}

		renderState.depthTest = true;
	}
	else if (mode == BuildMode::Flag)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3((int)mousePos3d.x + 0.5f, 0, (int)mousePos3d.z + 0.5f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::location, glm::vec2(0,0));
		renderState.activeShader->setUniform(Uniforms::buildFactor, 1.0f);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 0.5f));
		flagModel->draw(renderState, renderer, -1);
	}
}





void Sieged::calcPaths()
{
	if (calculatingPaths)
	{
		calculatePathsAgain = true;
		return;
	}
	calculatePathsAgain = false;
	calculatingPaths = true;
	double beginTime = blib::util::Profiler::getAppTime();

	pathCalculateThread = new blib::BackgroundTask<std::map<Flowmap*, std::vector<std::vector<int>>>>(this, [=, this]()
	{
		double beginTime_ = blib::util::Profiler::getAppTime();

		std::map<Flowmap*, std::vector<std::vector<int>>> costs;
		for (Flowmap* flowMap : flowmaps)
		{
			std::vector<std::vector<float>> myCosts(100, std::vector<float>(100, 9999999));
			//std::set < glm::ivec2, std::function<bool(const glm::ivec2&, const glm::ivec2&)>>queue([](const glm::ivec2 &a, const glm::ivec2 &b) { return a.x == b.x ? a.y < b.y : a.x < b.x; });
			std::list<glm::ivec2> queue;

			if (flowMap->targetBuilding)
			{
				Building* targetBuilding = blib::linq::firstOrDefault<Building*>(buildings, [](Building* b){ return b->buildingTemplate->type == BuildingTemplate::TownHall; }, nullptr);
				if (targetBuilding == NULL)
					continue;

				for (int x = 0; x < targetBuilding->buildingTemplate->size.x; x++)
					for (int y = 0; y < targetBuilding->buildingTemplate->size.y; y++)
						queue.push_back(targetBuilding->position + glm::ivec2(x, y));
			}
			else
				queue.push_back(flowMap->targetPosition);

			for (auto p : queue)
				myCosts[p.x][p.y] = 0;
			int a = 0;

			while (!queue.empty())
			{
				a++;
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

						float costFac = 1;
						if (tiles[newPos.x][newPos.y]->building)
						{
							if (tiles[newPos.x][newPos.y]->building->buildTimeLeft > 0)
								costFac = 2;
							costFac = 10;
						}

						float newCost = myCosts[pos.x][pos.y] + glm::length(glm::vec2(offset)) * costFac;
						if (myCosts[newPos.x][newPos.y] < newCost)
							continue;

						if (!blib::linq::containsValue(queue, newPos))
							queue.push_front(newPos);
						myCosts[newPos.x][newPos.y] = newCost;
					}
				}
			}
			printf("%i iterations\n", a);


			std::vector<std::vector<int>> directions(100, std::vector<int>(100, 0));
			for (int x = 0; x < 100; x++)
			{
				for (int y = 0; y < 100; y++)
				{
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
							if (myCosts[xxx][yyy] < myCosts[x + m.x][y + m.y])
								m = glm::ivec2(xx, yy);
						}
					}
					if (m.x < 0)
						directions[x][y] |= Left;
					if (m.x > 0)
						directions[x][y] |= Right;
					if (m.y < 0)
						directions[x][y] |= Up;
					if (m.y > 0)
						directions[x][y] |= Down;
				}
			}

			costs[flowMap] = directions;
		}
		Log::out << "Finding paths: " << (blib::util::Profiler::getAppTime() - beginTime_) << " s " << Log::newline;
		pathCalculateThread = NULL;
		return costs;
	}, [=, this](const std::map<Flowmap*, std::vector<std::vector<int>>>& costs)
	{
		for (const std::pair<Flowmap*, std::vector<std::vector<int>>> &it : costs)
		{
			Flowmap* flowMap = it.first;
			const std::vector<std::vector<int>>& directions = it.second;
			it.first->flow = directions;
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
		calculatingPaths = false;
		if (calculatePathsAgain)
			calcPaths();
	});
}
void Sieged::calcWalls()
{
	wallCache.clear();
	static int mask[][4][3] =
	{
		{
			{ 2, 0, 2 },
			{ 1, 1, 1 },
			{ 2, 0, 2 },
			{ 2, 0, 0 }
		},
		{
			{ 2, 1, 2 },
			{ 0, 1, 0 },
			{ 2, 1, 2 },
			{ 2, 90, 0 }
		},
		{
			{ 2, 1, 2 },
			{ 0, 1, 0 },
			{ 2, 0, 2 },
			{ 1, 90, 0 },
		},
		{
			{ 2, 0, 2 },
			{ 0, 1, 0 },
			{ 2, 1, 2 },
			{ 1, -90, 0 }
		},
		{
			{ 2, 0, 2 },
			{ 1, 1, 0 },
			{ 2, 0, 2 },
			{ 1, 180, 0 },
		},
		{ //end4
			{ 2, 0, 2 },
			{ 0, 1, 1 },
			{ 2, 0, 2 },
			{ 1, 0, 0 }
		},
		{//corner 1
			{ 2, 0, 2 },
			{ 0, 1, 1 },
			{ 2, 1, 2 },
			{ 3, -90, 0 }
		},
		{//corner 2
			{ 2, 1, 2 },
			{ 0, 1, 1 },
			{ 2, 0, 2 },
			{ 3, 0, 0 }
		},
		{//corner 3
			{ 2, 1, 2 },
			{ 1, 1, 0 },
			{ 2, 0, 2 },
			{ 3, 90, 0 }
		},
		{//corner 4
			{ 2, 0, 2 },
			{ 1, 1, 0 },
			{ 2, 1, 2 },
			{ 3, 180, 0 }
		},
		{//T 1
			{ 2, 0, 2 },
			{ 1, 1, 1 },
			{ 2, 1, 2 },
			{ 4, 0, 0 }
		},
		{//T 2
			{ 2, 1, 2 },
			{ 1, 1, 1 },
			{ 2, 0, 2 },
			{ 4, 180, 0 }
		},
		{//T 3
			{ 2, 1, 2 },
			{ 1, 1, 0 },
			{ 2, 1, 2 },
			{ 4, -90, 0 }
		},
		{//T 4
			{ 2, 1, 2 },
			{ 0, 1, 1 },
			{ 2, 1, 2 },
			{ 4, 90, 0 }
		},
		{//+
			{ 2, 1, 2 },
			{ 1, 1, 1 },
			{ 2, 1, 2 },
			{ 5, 0, 0 }
		},
		{//.
			{ 2, 0, 2 },
			{ 0, 1, 0 },
			{ 2, 0, 2 },
			{ 0, 0, 0 }
		},

	};

	for (int x = 1; x < 99; x++)
	{
		for (int y = 1; y < 99; y++)
		{

			for (int i = 0; i < 16; i++)
			{
				bool match = true;
				for (int xx = 0; xx < 3; xx++)
					for (int yy = 0; yy < 3; yy++)
						if ((mask[i][yy][xx] == 0 && tiles[x - 1 + xx][y - 1 + yy]->isWall()) ||
							(mask[i][yy][xx] == 1 && !tiles[x - 1 + xx][y - 1 + yy]->isWall()))
							match = false;
				if (match)
				{
					glm::mat4 mat;
					//mat = glm::scale(mat, glm::vec3(2, 1, 2));
					mat = glm::translate(mat, glm::vec3(x + 0.5, 0, y + 0.5f));
					mat = glm::rotate(mat, (float)mask[i][3][1], glm::vec3(0, 1, 0));
					if (mask[i][3][0] == 3)
						mat = glm::rotate(mat, -90.0f, glm::vec3(0, 0, 1));
					if (mask[i][3][0] == 4)
						mat = glm::rotate(mat, 180.0f, glm::vec3(1, 0, 0));
					if (mask[i][3][0] == 5)
						mat = glm::rotate(mat, 90.0f, glm::vec3(0, 0, 1));


					wallCache.push_back(std::make_tuple(mat, tiles[x][y]->building, wallModels[mask[i][3][0]]));

					break;
				}
			}
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
	this->buildTimeLeft = buildingTemplate->buildTime;
	this->damage = 0;
}

BuildingTemplate::BuildingTemplate(const blib::json::Value &data, blib::TextureMap* textureMap, blib::StaticModel* model)
{
	this->type = (BuildingTemplate::Type)data["id"].asInt();
	this->size = glm::ivec2(data["size"][0].asInt(), data["size"][1].asInt());
	this->texInfo = textureMap->addTexture(data["beltthumb"]);
	this->model = model;
	this->buildTime = data["constructiontime"];

	this->rngWeight = -1;
	if (data.isMember("rng"))
		rngWeight = data["rng"];
	cost = data["cost"];
	hitpoints = data["hitpoints"];

	healthbarSize = 7000;
	if (data.isMember("healthbarsize"))
		healthbarSize = data["healthbarsize"];
}


Building* Character::updateMovement(float elapsedTime, TileMap &tiles)
{
	glm::ivec2 tile = glm::ivec2(position);
	if (tile.x < 0 || tile.y < 0)
		return NULL;
	int direction = flowmap->flow[tile.x][tile.y];

	glm::vec2 oldPos = position;
	glm::vec2 originalPos = oldPos;

	glm::vec2 tileCenter = glm::vec2(tile) + glm::vec2(0.5f, 0.5f);


	Building* attackBuilding = NULL;


	if ((direction & Left) != 0)
		position.x -= (float)elapsedTime * speed;
	if (tiles[(int)(position.x)][(int)(position.y)]->building)
	{
		position = oldPos;
		attackBuilding = tiles[(int)(position.x)][(int)(position.y)]->building;
	}
	oldPos = position;
	if ((direction & Right) != 0)
		position.x += (float)elapsedTime * speed;
	if (tiles[(int)(position.x)][(int)(position.y)]->building)
	{
		position = oldPos;
		attackBuilding = tiles[(int)(position.x)][(int)(position.y)]->building;
	}
	oldPos = position;

	if ((direction & Left) == 0 && (direction & Right) == 0)
		position.x += (float)elapsedTime * (tileCenter.x - position.x) * blib::math::randomFloat(0.1f, 0.75f);
	if (tiles[(int)(position.x)][(int)(position.y)]->building)
	{
		position = oldPos;
		attackBuilding = tiles[(int)(position.x)][(int)(position.y)]->building;
	}
	oldPos = position;


	if ((direction & Down) != 0)
		position.y += (float)elapsedTime * speed;
	if (tiles[(int)(position.x)][(int)(position.y)]->building)
	{
		position = oldPos;
		attackBuilding = tiles[(int)(position.x)][(int)(position.y)]->building;
	}
	oldPos = position;
	if ((direction & Up) != 0)
		position.y -= (float)elapsedTime * speed;
	if (tiles[(int)(position.x)][(int)(position.y)]->building)
	{
		position = oldPos;
		attackBuilding = tiles[(int)(position.x)][(int)(position.y)]->building;
	}
	oldPos = position;

	if ((direction & Down) == 0 && (direction & Up) == 0)
		position.y += (float)elapsedTime * (tileCenter.y - position.y) * blib::math::randomFloat(0.1f, 0.75f);
	if (tiles[(int)(position.x)][(int)(position.y)]->building)
	{
		position = oldPos;
		attackBuilding = tiles[(int)(position.x)][(int)(position.y)]->building;
	}
	oldPos = position;

	return attackBuilding;
}
