#include "Sieged.h"

#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <set>

#include "Archer.h"
#include "Knight.h"
#include "Enemy.h"
#include "Building.h"
#include "BuildingTemplate.h"
#include "Tile.h"
#include "Flag.h"

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
	cloudTexture = resourceManager->getResource<blib::Texture>("assets/textures/cloud.png");
	conveyorTexture = resourceManager->getResource<blib::Texture>("assets/textures/conveyor.png");
	conveyorTexture->setTextureRepeat(true);
	gridTexture->setTextureRepeat(true);
	font = resourceManager->getResource<blib::Font>("tahoma");
	font48 = resourceManager->getResource<blib::Font>("main48");
	conveyorBuildingTextureMap = resourceManager->getResource<blib::TextureMap>();

	notEnoughGoldTexture = resourceManager->getResource<blib::Texture>("assets/textures/notenoughgold.png");



	gold = gameSettings.startGold;

	
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

	flagModel = new blib::StaticModel("assets/models/flag.fbx.json", resourceManager, renderer);
	flagModel->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/flag.png");

	protobot = new blib::SkelAnimatedModel("assets/models/protobot.dae.mesh.json", "assets/models/protobot.dae.skel.json", resourceManager, renderer);
	protobot->loadAnimation("assets/models/protobot.dae..anim.json");
	protobot->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/protobot.png");

	protoBotState = protobot->getNewState();

	knightModel = new blib::SkelAnimatedModel("assets/models/testgastje.dae.mesh.json", "assets/models/testgastje.dae.skel.json", resourceManager, renderer);
	knightModel->loadAnimation("assets/models/testgastje.dae.idle.anim.json");
	knightModel->loadAnimation("assets/models/testgastje.dae.walk.anim.json");
	knightModel->loadAnimation("assets/models/testgastje.dae.attack.anim.json");
	//soldierModel->meshes[0]->material.texture = resourceManager->getResource<blib::Texture>("assets/models/protobot.png");

	knightState = knightModel->getNewState();



	buttons.wall = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnWall.png"), blib::math::Rectangle(glm::vec2(16, 200), 48, 48));
	buttons.market = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnMarket.png"), blib::math::Rectangle(glm::vec2(16, 248), 48, 48));
	buttons.flag = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnFlag.png"), blib::math::Rectangle(glm::vec2(16, 296), 48, 48));
	buttons.knights = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnSoldiers.png"), blib::math::Rectangle(glm::vec2(16, 344), 48, 48));
	buttons.archers = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnArchers.png"), blib::math::Rectangle(glm::vec2(16, 392), 48, 48));

	buttons.magic.powerSurge = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnPowerSurge.png"), blib::math::Rectangle(glm::vec2(16, 440), 48, 48));
	buttons.magic.lightningBolt = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnLightningBolt.png"), blib::math::Rectangle(glm::vec2(16, 488), 48, 48));
	buttons.magic.thunderstorm = new blib::AnimatableSprite(resourceManager->getResource<blib::Texture>("assets/textures/hud/btnThunderStorm.png"), blib::math::Rectangle(glm::vec2(16, 536), 48, 48));

	buttons.wall->color = glm::vec4(1, 1, 1, 0);




	tiles.resize(100, std::vector<Tile*>(100, nullptr));
	for (int x = 0; x < 100; x++)
		for (int y = 0; y < 100; y++)
			tiles[x][y] = new Tile();


	flowMap.srcBuilding = (Building*)1;
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
	backgroundShader->addVertexShader("getmatrix.static");
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

	characterShader = resourceManager->getResource<blib::Shader>("simple");
	characterShader->addVertexShader("getmatrix.animate");
	characterShader->bindAttributeLocation("a_position", 0);
	characterShader->bindAttributeLocation("a_texcoord", 1);
	characterShader->bindAttributeLocation("a_normal", 2);
	characterShader->bindAttributeLocation("a_boneIds", 3);
	characterShader->bindAttributeLocation("a_boneWeights", 4);
	characterShader->setUniformName(Uniforms::ProjectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	characterShader->setUniformName(Uniforms::CameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	characterShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	characterShader->setUniformName(Uniforms::colorMult, "colorMult", blib::Shader::UniformType::Vec4);
	characterShader->setUniformName(Uniforms::s_texture, "s_texture", blib::Shader::UniformType::Int);
	characterShader->setUniformName(Uniforms::s_shadowmap, "s_shadowmap", blib::Shader::UniformType::Int);
	characterShader->setUniformName(Uniforms::buildFactor, "buildFactor", blib::Shader::UniformType::Float);
	characterShader->setUniformName(Uniforms::location, "location", blib::Shader::UniformType::Vec2);
	characterShader->setUniformName(Uniforms::shadowProjectionMatrix, "shadowProjectionMatrix", blib::Shader::UniformType::Mat4);
	characterShader->setUniformName(Uniforms::shadowCameraMatrix, "shadowCameraMatrix", blib::Shader::UniformType::Mat4);
	characterShader->setUniformName(Uniforms::lightDirection, "lightDirection", blib::Shader::UniformType::Vec3);
	characterShader->setUniformName(Uniforms::shadowFac, "shadowFac", blib::Shader::UniformType::Float);
	characterShader->setUniformArray(Uniforms::boneMatrices, "boneMatrices", 50, blib::Shader::Mat4);

	characterShader->finishUniformSetup();
	characterShader->setUniform(Uniforms::s_texture, 0);
	characterShader->setUniform(Uniforms::s_shadowmap, 1);


	shadowmapBackgroundShader = resourceManager->getResource<blib::Shader>("shadowmap");
	shadowmapBackgroundShader->addVertexShader("getmatrix.static");
	shadowmapBackgroundShader->bindAttributeLocation("a_position", 0);
	shadowmapBackgroundShader->bindAttributeLocation("a_texcoord", 1);
	shadowmapBackgroundShader->setUniformName(Uniforms::ProjectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	shadowmapBackgroundShader->setUniformName(Uniforms::CameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	shadowmapBackgroundShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	shadowmapBackgroundShader->setUniformName(Uniforms::buildFactor, "buildFactor", blib::Shader::UniformType::Float);
	shadowmapBackgroundShader->setUniformName(Uniforms::location, "location", blib::Shader::UniformType::Vec2);
	shadowmapBackgroundShader->finishUniformSetup();

	shadowmapCharacterShader = resourceManager->getResource<blib::Shader>("shadowmap");
	shadowmapCharacterShader->addVertexShader("getmatrix.animate");
	shadowmapCharacterShader->bindAttributeLocation("a_position", 0);
	shadowmapCharacterShader->bindAttributeLocation("a_texcoord", 1);
	shadowmapCharacterShader->bindAttributeLocation("a_boneIds", 3);
	shadowmapCharacterShader->bindAttributeLocation("a_boneWeights", 4);
	shadowmapCharacterShader->setUniformName(Uniforms::ProjectionMatrix, "projectionMatrix", blib::Shader::UniformType::Mat4);
	shadowmapCharacterShader->setUniformName(Uniforms::CameraMatrix, "cameraMatrix", blib::Shader::UniformType::Mat4);
	shadowmapCharacterShader->setUniformName(Uniforms::modelMatrix, "modelMatrix", blib::Shader::UniformType::Mat4);
	shadowmapCharacterShader->setUniformName(Uniforms::buildFactor, "buildFactor", blib::Shader::UniformType::Float);
	shadowmapCharacterShader->setUniformName(Uniforms::location, "location", blib::Shader::UniformType::Vec2);
	shadowmapCharacterShader->setUniformArray(Uniforms::boneMatrices, "boneMatrices", 50, blib::Shader::Mat4);
	shadowmapCharacterShader->finishUniformSetup();



	renderState.depthTest = true;
	renderState.blendEnabled = true;
	renderState.srcBlendColor = blib::RenderState::SRC_ALPHA;
	renderState.srcBlendAlpha = blib::RenderState::SRC_ALPHA;
	renderState.dstBlendColor = blib::RenderState::ONE_MINUS_SRC_ALPHA;
	renderState.dstBlendAlpha = blib::RenderState::ONE_MINUS_SRC_ALPHA;
	renderState.activeShader = backgroundShader;



	conveyorOffset = 0;
	lastConveyorBuilding = 1 / gameSettings.conveyorBuildingsPerSecond;
	draggingBuilding = NULL;
	conveyorDragIndex = -1;
	conveyorBuildings.push_back(std::pair<BuildingTemplate*, float>(buildingTemplates[BuildingTemplate::TownHall], 1920.0f));
	conveyorBuildings.push_back(std::pair<BuildingTemplate*, float>(buildingTemplates[BuildingTemplate::Barracks], 1920.0f+64));


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
	if (keyState.isPressed(blib::Key::_8))
		speed = 50;


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


	




	conveyorOffset += (float)elapsedTime * gameSettings.conveyorSpeed;
	while (conveyorOffset > 128)
		conveyorOffset -= 128;

	for (size_t i = 0; i < conveyorBuildings.size(); i++)
	{
		conveyorBuildings[i].second = glm::max(conveyorBuildings[i].second - (float)elapsedTime * gameSettings.conveyorSpeed, 64.0f * i);
	}

	if (mouseState.rightButton && !prevMouseState.leftButton)
		beginMouseState = mouseState;

	if (mouseState.leftButton && !prevMouseState.leftButton)
	{
		beginMouseState = mouseState;
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
			if (buttons.knights->contains(glm::vec2(mouseState.position)))
			{
				if (gold < gameSettings.knightTrainingCost)
				{
					blib::AnimatableSprite* e = new blib::AnimatableSprite(notEnoughGoldTexture, glm::vec2(mouseState.position) - notEnoughGoldTexture->center);
					e->resizeTo(glm::vec2(2, 2), 1);
					e->alphaTo(0, 1);
					effects.push_back(e);
				}
				else
				{
					gold -= gameSettings.knightTrainingCost;
					mode = BuildMode::Normal;
					Building* barracks = blib::linq::firstOrDefault<Building*>(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::Barracks; });
					assert(barracks);
					Knight* knight = new Knight(glm::vec2(barracks->position) + glm::vec2(1.5, barracks->buildingTemplate->size.y + 0.1f), gameSettings);
					knight->modelState = knightModel->getNewState();
					knight->modelState->playAnimation("idle");
					knight->modelState->update(0.01f);
					knight->flowmap = NULL;
					knights.push_back(knight);

					std::vector<Flag*> knightFlags = blib::linq::where(flags, [](Flag* f) { return f->knightFlag; });
					if (!knightFlags.empty())
					{
						std::sort(knightFlags.begin(), knightFlags.end(), [](Flag* a, Flag* b) { return a->knights.size() < b->knights.size();  });
						Flag* flag = knightFlags[0];

						knight->flowmap = &flag->flowmap;
						knight->flag = flag;
						flag->knights.push_back(knight);
					}
				}
			}
			if (buttons.archers->contains(glm::vec2(mouseState.position)))
			{
				if (gold < gameSettings.archerTrainingCost)
				{
					blib::AnimatableSprite* e = new blib::AnimatableSprite(notEnoughGoldTexture, glm::vec2(mouseState.position) - notEnoughGoldTexture->center);
					e->resizeTo(glm::vec2(2, 2), 1);
					e->alphaTo(0, 1);
					effects.push_back(e);
				}
				else
				{
					gold -= gameSettings.archerTrainingCost;
					mode = BuildMode::Normal;
					Building* barracks = blib::linq::firstOrDefault<Building*>(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::Barracks; });
					assert(barracks);
					Archer* archer = new Archer(glm::vec2(barracks->position) + glm::vec2(1.5, barracks->buildingTemplate->size.y + 0.1f), gameSettings);
					archer->modelState = knightModel->getNewState();
					archer->modelState->playAnimation("idle");
					archer->modelState->update(0.01f);
					archer->flowmap = NULL;
					archers.push_back(archer);

					std::vector<Flag*> archerFlags = blib::linq::where(flags, [](Flag* f) { return !f->knightFlag; });
					if (!archerFlags.empty())
					{
						std::sort(archerFlags.begin(), archerFlags.end(), [](Flag* a, Flag* b) { return a->archers.size() < b->archers.size();  });
						Flag* flag = flags[0];

						archer->flowmap = &flag->flowmap;
						archer->flag = flag;
						flag->archers.push_back(archer);
					}
				}
			}
			if (buttons.magic.thunderstorm->contains(glm::vec2(mouseState.position)))
			{
				if (mode == BuildMode::MagicThunderstorm)
					mode = BuildMode::Normal;
				else
					mode = BuildMode::MagicThunderstorm;
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
				if (ok)
				{
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

				for (auto f : flags)
					f->knightFlag = !(tiles[f->position.x][f->position.y]->building && tiles[f->position.x][f->position.y]->building->buildingTemplate->type == BuildingTemplate::Wall);


			}
		}
		else if (mode == BuildMode::Flag)
		{
			Flag* f = blib::linq::firstOrDefault<Flag*>(flags, [this](Flag* f) { return f->position == glm::ivec2(mousePos3d.x, mousePos3d.z); }, NULL);
			if (f)
			{
				blib::linq::removewhere(flags, [this](Flag* f) { return f->position == glm::ivec2(mousePos3d.x, mousePos3d.z); });
				blib::linq::removewhere(flowmaps, [f](Flowmap* fm) { return fm == &f->flowmap; });


				std::vector<Knight*> solds = f->knights;
				flagsToErase.push_back(f);

				for (auto s : solds)
				{
					std::sort(flags.begin(), flags.end(), [](Flag* a, Flag* b) { return a->knights.size() < b->knights.size();  });
					flags[0]->knights.push_back(s);
					s->flowmap = &flags[0]->flowmap;
					s->flag = flags[0];
				}


			}
			else
				if (!buttons.flag->contains(glm::vec2(mouseState.position)))
				{
					f = new Flag(glm::ivec2(mousePos3d.x, mousePos3d.z));
					f->knightFlag = !(tiles[f->position.x][f->position.y]->building && tiles[f->position.x][f->position.y]->building->buildingTemplate->type == BuildingTemplate::Wall);
					flowmaps.push_back(&f->flowmap);
					flags.push_back(f);
					calcPaths();
				}
		}
		else if (mode == BuildMode::MagicThunderstorm && !buttons.magic.thunderstorm->contains(glm::vec2(mouseState.position)))
		{
			mode = BuildMode::Normal;
			
			thunderStormPosition = glm::vec3(mousePos3d);
			thunderStormTime = 2;
		}
	}
	if (mouseState.rightButton && !prevMouseState.rightButton && glm::distance(glm::vec2(beginMouseState.position), glm::vec2(mouseState.position)) < 4)
		mode = BuildMode::Normal;

	
	if (blib::linq::contains(buildings, [](Building* b){ return b->buildingTemplate->type == BuildingTemplate::TownHall && b->buildTimeLeft == 0;  }))
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
			lastConveyorBuilding = 1 / gameSettings.conveyorBuildingsPerSecond;
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
			gold += (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::MineralMine; })) * gameSettings.goldPerSecondPerMineralMine;
			gold = glm::max(gold+1, (int)(gold * (1 + (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::Bank; })) * gameSettings.goldInterest)));
		}


		nextEnemySpawn -= (float)elapsedTime;
		if (nextEnemySpawn < 0)
		{
			
			int monstersThisThreatLevel = pow(gameSettings.threadLevelFactor * (int)(threatLevel+1), gameSettings.threadLevelExponent);

			nextEnemySpawn = 60.0f / monstersThisThreatLevel;

			Log::out << "Thread level: " << threatLevel << ", Monsters: " << monstersThisThreatLevel << ", nextEnemySpawn: " << nextEnemySpawn << Log::newline;

			while (true)
			{
				glm::vec2 pos = glm::vec2(50,50) + 49.0f * blib::util::fromAngle(blib::math::randomFloat(0, 2 * blib::math::pif));
				if (tiles[(int)(pos.x)][(int)(pos.y)]->building)
					continue;
				enemies.push_back(new Enemy(pos, &flowMap, gameSettings));
				break;
			}
		}

		

		//knight AI
		for (Knight* s : knights)
		{
			s->modelState->update((float)elapsedTime);
			if (!s->flag)
				continue;
			s->movementDirection = s->directionFromFlowMap();
			s->movementTarget = glm::vec2(s->flag->position) + glm::vec2(0.5f, 0.5f);
			for (auto ee : knights)
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
			{
				Building* b = tiles[(int)(s->position.x)][(int)(s->position.y)]->building;

				blib::math::Rectangle buildRect(glm::vec2(b->position), b->buildingTemplate->size.x, b->buildingTemplate->size.y);
				glm::vec2 projection = buildRect.projectClosest(s->position);
				s->movementDirection = glm::normalize(projection - s->position);
			}


			Enemy* attackTarget = NULL;
			if (!enemies.empty())
			{
				Enemy* e = blib::linq::min<float, Enemy*>(enemies, [s](Enemy* e) { return glm::distance(e->position, s->position); }, [](Enemy* e) { return e; });
				if (e)
				{
					if (glm::distance(e->position, s->position) < 5 && glm::distance(e->position, s->position) > 0.001f) // spotting range
					{
						s->movementDirection = glm::normalize(e->position - s->position);
						s->movementTarget = e->position; //TODO: make him stop in front of him
					}
					if (glm::distance(e->position, s->position) < 0.5f) //attack range
						attackTarget = e;
				}
			}

			s->timeLeftForAttack -= (float)elapsedTime;
			if (s->timeLeftForAttack <= 0)
				s->timeLeftForAttack = 0;

			if (s->lastAttackedEntity)
			{
				Enemy* asEnemy = dynamic_cast<Enemy*>(s->lastAttackedEntity);
				if (asEnemy)
					if (glm::distance(asEnemy->position, s->position) < 0.5f)
						attackTarget = asEnemy;
			}

			if (attackTarget && s->timeLeftForAttack <= 0)
			{
				s->timeLeftForAttack = gameSettings.knightAttackDelay * gameSettings.knightStrength; // attack delay
				damage(attackTarget, gameSettings.knightDamage * gameSettings.knightStrength);
			}

			glm::vec2 oldPos = s->position;
			s->move(tiles, (float)elapsedTime);
			float moved = glm::distance(oldPos, s->position);
			if (moved < 0.1f * elapsedTime * s->speed)
			{
				s->modelState->playAnimation("idle");
				s->modelState->stopAnimation("walk");
			}
			else
			{
				s->modelState->playAnimation("walk");
				s->modelState->stopAnimation("idle");
			}
		}

		//archers AI
		for (Archer* a : archers)
		{
			a->modelState->update((float)elapsedTime);
			if (!a->flag)
				continue;
			glm::vec2 oldPos = a->position;

			if (!a->atFlag)
			{
				if (a->flowmap->flow[(int)a->position.x][(int)a->position.y] != 0)
					a->movementDirection = a->directionFromFlowMap();
				a->movementTarget = glm::vec2(a->flag->position) + glm::vec2(0.5f, 0.5f);
				a->move(tiles, (float)elapsedTime, true);

				if (glm::distance(a->movementTarget, a->position) < 0.75f)
				{
					a->atFlag = true;
					a->calculateWallPosition(tiles);
				}
			}
			else //at flag
			{
				if (glm::distance(a->movementTarget, a->position) > 0.1f)
				{
					a->position += glm::normalize(a->movementTarget - a->position) * a->speed * (float)elapsedTime;
				}

				Enemy* attackTarget = NULL;
				if (!enemies.empty())
				{
					Enemy* e = blib::linq::min<float, Enemy*>(enemies, [a](Enemy* e) { return glm::distance(e->position, a->position); }, [](Enemy* e) { return e; });
					if (e)
					{
						if (glm::distance(e->position, a->position) < 15 && glm::distance(e->position, a->position) > 0.001f) // spotting range
							attackTarget = e;
					}
				}

				a->timeLeftForAttack -= (float)elapsedTime;
				if (a->timeLeftForAttack <= 0)
					a->timeLeftForAttack = 0;

				if (a->lastAttackedEntity)
				{
					Enemy* asEnemy = dynamic_cast<Enemy*>(a->lastAttackedEntity);
					if (asEnemy)
						if (glm::distance(asEnemy->position, a->position) < 15.0f)//attackrange
							attackTarget = asEnemy;
				}

				if (attackTarget && a->timeLeftForAttack <= 0)
				{
					a->modelState->stopAnimation("idle");
					a->modelState->playAnimation("attack", 0.0f, true);
					a->timeLeftForAttack = gameSettings.archeryAttackDelay / gameSettings.archerStrength; // attack delay
					damage(attackTarget, gameSettings.archeryDamage * gameSettings.archerStrength);
				}
			}


			float moved = glm::distance(oldPos, a->position);
			if (moved < 0.1f * elapsedTime * a->speed)
			{
				a->modelState->stopAnimation("walk");
				if (a->modelState->animations.size() == 0)
					a->modelState->playAnimation("idle");
			}
			else
			{
				a->modelState->stopAnimation("idle");
				if (a->modelState->animations.size() == 0)
					a->modelState->playAnimation("walk");
			}
		}


		//enemy AI
		for (auto e : enemies)
		{
			e->movementDirection = e->directionFromFlowMap();

			for (auto ee : enemies) // todo: use all characters here
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
			{
				Building* b = tiles[(int)(e->position.x)][(int)(e->position.y)]->building;

				blib::math::Rectangle buildRect(glm::vec2(b->position), b->buildingTemplate->size.x, b->buildingTemplate->size.y);
				glm::vec2 projection = buildRect.projectClosest(e->position);
				if (glm::length(projection - e->position) > 0.001f)
					e->movementDirection = glm::normalize(projection - e->position);
			}

			Damagable* attackTarget = NULL;
			if (!knights.empty())
			{
				Knight* s = blib::linq::min<float, Knight*>(knights, [e](Knight* s) { return glm::distance(e->position, s->position); }, [](Knight* s) { return s; });
				if (s)
				{
					float spotRange = gameSettings.knightSpottingRange;
					if (glm::distance(s->position, glm::vec2(s->flag->position) + glm::vec2(0.5f, 0.5f)) < 1)
						spotRange *= 2;

					if (glm::distance(e->position, s->position) < spotRange && glm::distance(e->position, s->position) >  0.001f) // spotting range
						e->movementDirection = glm::normalize(s->position - e->position);
					if (glm::distance(e->position, s->position) < 0.25f) //attack range
						attackTarget = s;
				}
			}

			if (!attackTarget)
			{
				for (Building* b : buildings)
				{
					blib::math::Rectangle buildRect(glm::vec2(b->position), b->buildingTemplate->size.x, b->buildingTemplate->size.y);
					glm::vec2 projection = buildRect.projectClosest(e->position);
					if (glm::length(projection - e->position) < 0.25f) // attack range
						attackTarget = b;
				}
			}
			
			e->timeLeftForAttack -= (float)elapsedTime;
			if (e->timeLeftForAttack <= 0)
				e->timeLeftForAttack = 0;

			if (e->lastAttackedEntity)
			{
				PlayerCharacter* asPlayerCharacter = dynamic_cast<PlayerCharacter*>(e->lastAttackedEntity);
				if (asPlayerCharacter)
					if (glm::distance(asPlayerCharacter->position, e->position) < 0.5f)
						attackTarget = asPlayerCharacter;
			}

			if (attackTarget && e->timeLeftForAttack <= 0)
			{
				e->timeLeftForAttack = gameSettings.enemyAttackDelay; // attack delay
				e->lastAttackedEntity = attackTarget;
				damage(attackTarget, gameSettings.enemyDamage);
			}

			e->move(tiles, (float)elapsedTime);
		}

	}

	for (auto b : buildings)
	{
		if (b->buildTimeLeft > 0 && b->buildingTemplate->type == BuildingTemplate::Wall)
		{
			if (b->buildTimeLeft == b->buildingTemplate->buildTime)
				if (gold >= b->buildingTemplate->cost)
					gold -= b->buildingTemplate->cost;
				else
					continue;
			b->buildTimeLeft -= (float)elapsedTime * gameSettings.wallBuildSpeed;
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
			if (b->buildTimeLeft == b->buildingTemplate->buildTime)
				if (gold >= b->buildingTemplate->cost)
					gold -= b->buildingTemplate->cost;
				else
					continue;


			b->buildTimeLeft -= (float)elapsedTime;
			if (b->buildTimeLeft < 0)
			{
				calcPaths();
				b->buildTimeLeft = 0;
				//done with building
				if (b->buildingTemplate->type == BuildingTemplate::StoneMason)
				{
					buttons.wall->alphaTo(1.0f, 1);
					gameSettings.wallBuildSpeed = 1 + (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::StoneMason; }) - 1) * gameSettings.stoneMasonFactor;
				}
				if (b->buildingTemplate->type == BuildingTemplate::TownHall)
				{
					gamePlaying = true;
				}
				if (b->buildingTemplate->type == BuildingTemplate::Barracks)
				{
					maxFlagCount = blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::Barracks; }) * gameSettings.flagsPerBarracks;
				}
				if (b->buildingTemplate->type == BuildingTemplate::ArcheryRange)
				{
					gameSettings.archerStrength = gameSettings.archeryRangeFactor * (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::ArcheryRange; }) - 1);
				}
				if (b->buildingTemplate->type == BuildingTemplate::BattleArena)
				{
					gameSettings.archerStrength = gameSettings.battleArenaFactor * (blib::linq::count(buildings, [](Building* b) { return b->buildingTemplate->type == BuildingTemplate::BattleArena; }) - 1);
				}
			}
			break;
		}
	}

	if (thunderStormTime > 0)
	{
		thunderStormTime -= elapsedTime;
		thunderStormClouds.push_back(glm::vec4(thunderStormPosition + glm::vec3(blib::math::randomFloat(-4, 4), 7, blib::math::randomFloat(-4, 4)), 0));
	}

	for (int i = 0; i < (int)thunderStormClouds.size(); i++)
	{
		thunderStormClouds[i].w += elapsedTime;
		if (thunderStormClouds[i].w > 2)
		{
			thunderStormClouds.erase(thunderStormClouds.begin() + i);
			i--;
		}
	}


	lightDirection += (float)elapsedTime * 0.001f;

	for (blib::AnimatableSprite* button : buttons.buttons)
		button->update((float)elapsedTime);
	protoBotState->update((float)elapsedTime);
	knightState->update((float)elapsedTime);

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
	renderState.activeShader = shadowmapBackgroundShader;
	shadowmapBackgroundShader->setUniform(Uniforms::CameraMatrix, shadowCameraMatrix);
	shadowmapBackgroundShader->setUniform(Uniforms::ProjectionMatrix, shadowProjectionMatrix);

	shadowmapCharacterShader->setUniform(Uniforms::CameraMatrix, shadowCameraMatrix);
	shadowmapCharacterShader->setUniform(Uniforms::ProjectionMatrix, shadowProjectionMatrix);

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
	backgroundShader->setUniform(Uniforms::shadowCameraMatrix, shadowCameraMatrix);
	backgroundShader->setUniform(Uniforms::shadowProjectionMatrix, shadowProjectionMatrix);
	backgroundShader->setUniform(Uniforms::lightDirection, lightAngle);

	backgroundShader->setUniform(Uniforms::CameraMatrix, cameraMatrix);
	backgroundShader->setUniform(Uniforms::ProjectionMatrix, projectionMatrix);

	characterShader->setUniform(Uniforms::shadowCameraMatrix, shadowCameraMatrix);
	characterShader->setUniform(Uniforms::shadowProjectionMatrix, shadowProjectionMatrix);
	characterShader->setUniform(Uniforms::lightDirection, lightAngle);

	characterShader->setUniform(Uniforms::CameraMatrix, cameraMatrix);
	characterShader->setUniform(Uniforms::ProjectionMatrix, projectionMatrix);
	drawWorld(RenderPass::Final);



	spriteBatch->begin();


	for (glm::vec4& c : thunderStormClouds)
	{
		glm::vec3 p = glm::project(glm::vec3(c), cameraMatrix, projectionMatrix, glm::uvec4(0, 0, window->getWidth(), window->getHeight()));
		if (p.z < 0 || p.z > 1)
			continue;
		if (c.w < 1)
			spriteBatch->draw(cloudTexture, blib::math::easyMatrix(glm::vec2(p)), cloudTexture->center, glm::vec4(1,1,1,c.w));
		else
			spriteBatch->draw(cloudTexture, blib::math::easyMatrix(glm::vec2(p)), cloudTexture->center, glm::vec4(1, 1, 1, 2-c.w));
	}



	for (auto b : buildings)
		b->drawHealthBar(this);
	for (auto k : knights)
		k->drawHealthBar(this);
	for (auto a : archers)
		a->drawHealthBar(this);
	for (auto e : enemies)
		e->drawHealthBar(this);



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


	spriteBatch->draw(font48, "Gold: " + std::to_string(gold), blib::math::easyMatrix(glm::vec2(1920 - 10 - font48->textlen("Gold: " + std::to_string(gold)) - 1, 5)), blib::Color::black);
	spriteBatch->draw(font48, "Gold: " + std::to_string(gold), blib::math::easyMatrix(glm::vec2(1920 - 10 - font48->textlen("Gold: " + std::to_string(gold)) + 1, 5)), blib::Color::black);
	spriteBatch->draw(font48, "Gold: " + std::to_string(gold), blib::math::easyMatrix(glm::vec2(1920 - 10 - font48->textlen("Gold: " + std::to_string(gold)), 4)), blib::Color::black);
	spriteBatch->draw(font48, "Gold: " + std::to_string(gold), blib::math::easyMatrix(glm::vec2(1920 - 10 - font48->textlen("Gold: " + std::to_string(gold)), 6)), blib::Color::black);
	spriteBatch->draw(font48, "Gold: " + std::to_string(gold), blib::math::easyMatrix(glm::vec2(1920 - 10 - font48->textlen("Gold: " + std::to_string(gold)), 5)));

	spriteBatch->draw(whitePixel, blib::math::easyMatrix(glm::vec2(1920 - 75, 50), 0, glm::vec2(50, 50)));
	float threatFrac = threatLevel - (int)threatLevel;
	spriteBatch->draw(whitePixel, blib::math::easyMatrix(glm::vec2(1920 - 75, 50), 0, glm::vec2(50 * threatFrac, 50)), blib::Color::pinkishOrange);
	spriteBatch->draw(font48, std::to_string((int)threatLevel), blib::math::easyMatrix(glm::vec2(1920 - 60, 50)), blib::Color::black);
	spriteBatch->draw(font48, std::to_string(flags.size()) + " / " + std::to_string(maxFlagCount), blib::math::easyMatrix(glm::vec2(100, 300)));



	if (mode == BuildMode::MagicThunderstorm)
	{
		spriteBatch->draw(buttons.magic.thunderstorm->texture, blib::math::easyMatrix(glm::vec2(mouseState.position)), buttons.magic.thunderstorm->texture->center);
	}


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
	renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1.25f, 1.25f, 1.25f, 1));
	renderState.activeShader->setUniform(Uniforms::buildFactor, 1.0f);
	renderState.activeShader->setUniform(Uniforms::shadowFac, 1.0f);
	renderState.activeTexture[0] = gridTexture;
	renderer->drawTriangles(verts, renderState);


	if (renderPass == RenderPass::Final)
		renderer->unproject(glm::vec2(mouseState.position), &mousePos3d, NULL, cameraMatrix, projectionMatrix);

	renderState.activeShader = renderPass == RenderPass::Final ? characterShader : shadowmapCharacterShader;

	renderState.activeShader->setUniform(Uniforms::modelMatrix, glm::mat4());
	renderState.activeShader->setUniform(Uniforms::buildFactor, 1.0f);
	renderState.activeShader->setUniform(Uniforms::shadowFac, 1.0f);

	for (auto e : enemies)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(e->position.x, 0.0f, e->position.y));
		//mat = glm::rotate(mat, -90.0f, glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, -glm::degrees(atan2(e->movementDirection.y, e->movementDirection.x)) + 90, glm::vec3(0, 1, 0));
		mat = glm::scale(mat, glm::vec3(0.05f, 0.05f, 0.05f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		protoBotState->draw(renderState, renderer, -1, (int)Uniforms::boneMatrices);
	}

	for (auto a : knights)
	{
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(a->position.x, 0.0f, a->position.y));
		//mat = glm::rotate(mat, -90.0f, glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, -glm::degrees(atan2(a->movementDirection.y, a->movementDirection.x)) + 90.0f, glm::vec3(0, 1, 0));
		mat = glm::scale(mat, glm::vec3(0.15f, 0.15f, 0.15f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		//renderer->drawTriangles(cube, renderState);
		a->modelState->draw(renderState, renderer, -1, (int)Uniforms::boneMatrices);
	}

	for (auto archer : archers)
	{
		bool onWall = tiles[(int)archer->position.x][(int)archer->position.y]->building && tiles[(int)archer->position.x][(int)archer->position.y]->building->buildingTemplate->type == BuildingTemplate::Wall;
		glm::mat4 mat;
		mat = glm::translate(mat, glm::vec3(archer->position.x, onWall ? 1.75f : 0.0f, archer->position.y));
		//mat = glm::rotate(mat, -90.0f, glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, -glm::degrees(atan2(archer->movementDirection.y, archer->movementDirection.x)) + 90.0f, glm::vec3(0, 1, 0));
		mat = glm::scale(mat, glm::vec3(0.15f, 0.15f, 0.15f));
		renderState.activeShader->setUniform(Uniforms::modelMatrix, mat);
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(2.0f, 1.0f, 1.0f, 1));
		//renderer->drawTriangles(cube, renderState);
		archer->modelState->draw(renderState, renderer, -1, (int)Uniforms::boneMatrices);
	}

	renderState.activeShader = renderPass == RenderPass::Final ? backgroundShader : shadowmapBackgroundShader;
	renderState.activeShader->setUniform(Uniforms::shadowFac, 1.0f);


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
			if (pos.x >= 0 && pos.y >= 0) // TODO: add upper bound check
			{
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
	}

	if (!collisionWalls.empty() && renderPass == RenderPass::Final)
	{
		std::vector<blib::VertexP3N3C4> lineVerts;
		for (blib::math::Polygon& e : collisionWalls)
		{
			for (int i = 0; i < (int)e.size(); i++)
			{
				int ii = (i + 1) % e.size();
				lineVerts.push_back(blib::VertexP3N3C4(glm::vec3(e[i].x, 0.1f, e[i].y), glm::vec3(0, 1, 0), glm::vec4(0, 0, 1, 1)));
				lineVerts.push_back(blib::VertexP3N3C4(glm::vec3(e[ii].x, 0.1f, e[ii].y), glm::vec3(0, 1, 0), glm::vec4(0, 0, 1, 1)));
			}
		}
		renderState.activeTexture[0] = whitePixel;
		renderState.activeShader->setUniform(Uniforms::modelMatrix, glm::mat4());
		renderState.activeShader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 9, 1));
		renderer->drawLines(lineVerts, 10.0f, renderState);
	}


	if (mode == BuildMode::Wall && renderPass == RenderPass::Final)
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
			blib::RenderState::CullFaces oldCull = renderState.cullFaces;
			renderState.cullFaces = blib::RenderState::CullFaces::NONE;
			renderer->drawTriangles(cube, renderState);
			renderState.cullFaces = oldCull;
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
			blib::RenderState::CullFaces oldCull = renderState.cullFaces;
			renderState.cullFaces = blib::RenderState::CullFaces::NONE;
			renderer->drawTriangles(cube, renderState);
			renderState.cullFaces = oldCull;
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
		std::vector<std::vector<float>> myCosts(100, std::vector<float>(100, 9999999));
		for (size_t ie = 0; ie < flowmaps.size(); ie++) //use for here instead of foreach, because the main thread can change the array, causing iterators to go invalid
		{
			Flowmap* flowMap = flowmaps[ie];

			for (int y = 0; y < 100; y++)
				for (int x = 0; x < 100; x++)
					myCosts[y][x] = 9999999;

			//std::set < glm::ivec2, std::function<bool(const glm::ivec2&, const glm::ivec2&)>>queue([](const glm::ivec2 &a, const glm::ivec2 &b) { return a.x == b.x ? a.y < b.y : a.x < b.x; });
			std::list<glm::ivec2> queue;

			if (flowMap->srcBuilding)
			{
				Building* targetBuilding = blib::linq::firstOrDefault<Building*>(buildings, [](Building* b){ return b->buildingTemplate->type == BuildingTemplate::TownHall; }, nullptr);
				if (targetBuilding == NULL)
					continue;

				for (int x = 0; x < targetBuilding->buildingTemplate->size.x; x++)
					for (int y = 0; y < targetBuilding->buildingTemplate->size.y; y++)
						queue.push_back(targetBuilding->position + glm::ivec2(x, y));
			}
			else
				queue.push_back(flowMap->srcPosition);

			for (auto p : queue)
				myCosts[p.x][p.y] = 0;
			int a = 0;

			while (!queue.empty())
			{
				a++;
				glm::ivec2 pos = queue.back();
				queue.pop_back();

				if (!flowMap->srcBuilding && tiles[pos.x][pos.y]->building && tiles[pos.x][pos.y]->building->buildingTemplate->type == BuildingTemplate::Barracks && pos.x == tiles[pos.x][pos.y]->building->position.x+1 && tiles[pos.x][pos.y]->building->position.y + tiles[pos.x][pos.y]->building->buildingTemplate->size.y == pos.y)
				{
					queue.clear();
					break;
				}
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
							
							
							if (!flowMap->srcBuilding) // flowmap to a flag
								costFac = 100;
							else
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


			costs[flowMap] = std::vector<std::vector<int>>(100, std::vector<int>(100, 0));
			std::vector<std::vector<int>> &directions = costs[flowMap];
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


		if (!flagsToErase.empty())
		{
			for (auto f : flagsToErase)
				delete f;
			flagsToErase.clear();
		}




		Log::out << "Path calculations: " << (blib::util::Profiler::getAppTime() - beginTime) << " s " << Log::newline;
		ClipperLib::Clipper clipper;
		ClipperLib::Polygons subject;
		ClipperLib::Polygons result;

		for (auto b : buildings)
		{
			subject.push_back(blib::math::Polygon({
				glm::vec2(b->position.x - 0.05f, b->position.y - 0.05f),
				glm::vec2(b->position.x + 0.05f + b->buildingTemplate->size.x, b->position.y - 0.05f),
				glm::vec2(b->position.x + 0.05f + b->buildingTemplate->size.x, b->position.y + 0.05f + b->buildingTemplate->size.y),
				glm::vec2(b->position.x - 0.05f, b->position.y + 0.05f + b->buildingTemplate->size.y),

			}).toClipperPolygon());

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



void Sieged::damage(Damagable* target, int damage)
{
	target->health -= damage;

	if (!target->isAlive())
	{
		for (auto s : knights)
			if (s->lastAttackedEntity == target)
				s->lastAttackedEntity = NULL;
		for (auto s : archers)
			if (s->lastAttackedEntity == target)
				s->lastAttackedEntity = NULL;
		for (auto s : enemies)
			if (s->lastAttackedEntity == target)
				s->lastAttackedEntity = NULL;


		PlayerCharacter* asPc = dynamic_cast<PlayerCharacter*>(target);
		if (asPc)
			if (asPc->flag)
			{
				blib::linq::removewhere(asPc->flag->knights, [asPc](Knight* s) { return s == asPc; });
				blib::linq::removewhere(asPc->flag->archers, [asPc](Archer* a) { return a == asPc; });
			}


		Knight* asKnight = dynamic_cast<Knight*>(target);
		if (asKnight)
			blib::linq::deletewhere(knights, [asKnight](Knight* k) { return k == asKnight; });
		else
		{
			Archer* asArcher = dynamic_cast<Archer*>(target);
			if (asArcher)
				blib::linq::deletewhere(archers, [asArcher](Archer* a) { return a == asArcher; });
			else
			{
				Enemy* asEnemy = dynamic_cast<Enemy*>(target);
				if (asEnemy)
					blib::linq::deletewhere(enemies, [asEnemy](Enemy* e) { return e == asEnemy; });
				else
				{
					Building* asBuilding = dynamic_cast<Building*>(target);
					if (asBuilding)
					{
						for (int x = 0; x < asBuilding->buildingTemplate->size.x; x++)
							for (int y = 0; y < asBuilding->buildingTemplate->size.y; y++)
								tiles[asBuilding->position.x + x][asBuilding->position.y + y]->building = NULL;
						if (asBuilding->buildingTemplate->type == BuildingTemplate::Wall)
							calcWalls();
						blib::linq::deletewhere(buildings, [asBuilding](Building* b) { return b == asBuilding; });
					}
				}
			}
		}
	}
}









