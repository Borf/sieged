#include "ZombieDraw.h"

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

ZombieDraw::ZombieDraw()
{
	appSetup.renderer = blib::AppSetup::GlRenderer;
	appSetup.title = "Zombie Draw";
	appSetup.window.setWidth(1024);
	appSetup.window.setHeight(768);

	appSetup.vsync = false;
	appSetup.joystickDriver = blib::AppSetup::DirectInput;
    
    appSetup.threaded = false;

}

void ZombieDraw::init()
{
	for (int i = 0; i < 8; i++)
		mapTextures.push_back(resourceManager->getResource<blib::Texture>("assets/textures/0" + std::to_string(i+1) + "WT.jpg"));
	checkpointTexture = resourceManager->getResource<blib::Texture>("assets/textures/checkpoint.png");
	playerAnimation = new blib::Animation("assets/textures/player.png.json", resourceManager);
	playerAnimation->setState("idle");
	zombieWalkAnimation = new blib::Animation("assets/textures/zombie.png.json", resourceManager);
	zombieWalkAnimation->setState("walk");
	zombieIdleAnimation = new blib::Animation("assets/textures/zombie.png.json", resourceManager);
	zombieIdleAnimation->setState("idle");
	zombieDeadAnimation = new blib::Animation("assets/textures/zombie.png.json", resourceManager);
	zombieDeadAnimation->setState("dead");
	lineTexture = resourceManager->getResource<blib::Texture>("assets/textures/line.png");
	lineTexture->setTextureRepeat(true);
	buttonTexture = resourceManager->getResource<blib::Texture>("assets/textures/button_back.png");
	playButtonTexture = resourceManager->getResource<blib::Texture>("assets/textures/play.png");
	undoButtonTexture = resourceManager->getResource<blib::Texture>("assets/textures/undo.png");
	bobTexture = resourceManager->getResource<blib::Texture>("assets/textures/bob.png");
	pickupTextures[Pickup::Type::Axe] = resourceManager->getResource<blib::Texture>("assets/textures/pickup_gun2.png");
	pickupTextures[Pickup::Type::Pistol] = resourceManager->getResource<blib::Texture>("assets/textures/pickup_gun.png");
	pickupTextures[Pickup::Type::MachineGun] = resourceManager->getResource<blib::Texture>("assets/textures/pickup_ak47.png");
	bulletTexture = resourceManager->getResource<blib::Texture>("assets/textures/bullet.png");
	zombieSoundTexture = resourceManager->getResource<blib::Texture>("assets/textures/zombiesound.png");
	panButtonTexture = resourceManager->getResource<blib::Texture>("assets/textures/pan_arrows.png");


	visionFbo = resourceManager->getResource<blib::FBO>();
	visionFbo->setSize(1024*2, 768*2);
	visionFbo->stencil = true;
	visionFbo->depth = false;
	visionFbo->textureCount = 1;
	combineShader = resourceManager->getResource<blib::Shader>("CombineShader");
	combineShader->bindAttributeLocation("a_position", 0);
	combineShader->bindAttributeLocation("a_texture", 1);
	combineShader->bindAttributeLocation("a_color", 2);
	combineShader->setUniformName(ProjectionMatrix, "projectionmatrix", blib::Shader::Mat4);
	combineShader->setUniformName(Matrix, "matrix", blib::Shader::Mat4);
	combineShader->setUniformName(s_texture, "s_texture", blib::Shader::Int);
	combineShader->setUniformName(s_visionTexture, "s_visionTexture", blib::Shader::Int);
	combineShader->setUniformName(zombieFactor, "zombieFactor", blib::Shader::Float);
	combineShader->finishUniformSetup();

	combineShader->setUniform(s_texture, 0);
	combineShader->setUniform(s_visionTexture, 1);
	combineShader->setUniform(ProjectionMatrix, glm::ortho(0.0f, (float)window->getWidth(), (float)window->getHeight(), 0.0f, -1000.0f, 1.0f));
	combineShader->setUniform(Matrix, glm::mat4());

	cameraTarget = cameraPos = currentState.playerPosition = glm::vec2(800, 500);
	cameraOffset = glm::vec2(0, 0);
	currentState.playerRotation = 0;
	currentState.weapon = State::Weapon::None;


	currentState.pickups.push_back(Pickup(glm::vec2(900, 500), Pickup::Type::Pistol));
	currentState.pickups.push_back(Pickup(glm::vec2(1100, 500), Pickup::Type::Axe));
	currentState.pickups.push_back(Pickup(glm::vec2(1100, 1000), Pickup::Type::MachineGun));


	checkpoints.push_back(glm::vec2(800, 500));
	checkpoints.push_back(glm::vec2(2200, 500));
	checkpoints.push_back(glm::vec2(2050, 1300));
	checkpoints.push_back(glm::vec2(2050, 1300));
	checkpoints.push_back(glm::vec2(480, 1580));
	checkpoints.push_back(glm::vec2(765, 2205));
	checkpoints.push_back(glm::vec2(1140, 1800));
	checkpoints.push_back(glm::vec2(2750, 1650));
	checkpoints.push_back(glm::vec2(4215, 1225));
	checkpoints.push_back(glm::vec2(3700, 2400));
	checkpoints.push_back(glm::vec2(2250, 2335));
	drawnLine.push_back(checkpoints[0]);

	lineOffset = 0;
	drawing = false;
	walking = false;
	zoom = 0.45f;
	gunTimer = 0;
	walkBlendFactor = 1;

	blib::json::Value mapData = blib::util::FileSystem::getJson("assets/map.txt");
	for (auto obj : mapData)
	{
		blib::math::Polygon p;
		for (auto v : obj)
			p.push_back(glm::vec2(v[0].asFloat(), v[1].asFloat()));
		visionObjects.push_back(p);
	}

	blib::json::Value zombies = blib::util::FileSystem::getJson("assets/zombies.txt");
	for (auto z : zombies)
	{
		currentState.zombies.push_back(Zombie(glm::vec2(z["pos"][0], z["pos"][1]), z["rot"]));
	}


	ClipperLib::Polygons polygons;
	ClipperLib::Polygons out;
	for (auto o : visionObjects)
		polygons.push_back(o.toClipperPolygon());
	ClipperLib::OffsetPolygons(polygons, out, 20000, ClipperLib::jtSquare);
	for (auto p : out)
		wallObjects.push_back(p);


	calculateAabb();
	lastState = currentState;

}

void ZombieDraw::update(double elapsedTime)
{
	glm::vec2 pointPos(glm::inverse(cameraMat) * glm::vec4(mouseState.position, 0, 1));


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
	if (keyState.isPressed(blib::Key::PLUS))
		zoom *= (float)(1 + elapsedTime);
	if (keyState.isPressed(blib::Key::MINUS))
		zoom *= (float)(1 - elapsedTime);

	if (keyState.isPressed(blib::Key::SPACE))
		Log::out << pointPos.x << ", " << pointPos.y << Log::newline;

	if (mouseState.leftButton && !lastMouseState.leftButton)
	{
		showBob = false;
		if (!walking)
		{
			if (glm::distance(pointPos, drawnLine[0]) < 60 && !walking && drawingReachedCheckpoint())
			{
				walking = true;
				cameraOffset = glm::vec2(0, 0);
				walkIndex = 0;
			}
			else if (glm::distance(pointPos, drawnLine[0]) < 60 && !walking && drawnLine.size() > 1)
				drawnLine.resize(1);
			else if (glm::distance(pointPos, drawnLine[0]) < 60 && !walking)
			{
				drawing = true;
				if (drawnLine.empty())
					drawnLine.push_back(pointPos);
			}
			else if (mouseState.position.x < 128 || mouseState.position.x > 1024 - 128 || mouseState.position.y < 128 || mouseState.position.y > 768 - 128)
			{
				if (mouseState.position.x < 128)
					cameraOffset.x--;
				if (mouseState.position.x > 1024 - 128)
					cameraOffset.x++;
				if (mouseState.position.y < 128)
					cameraOffset.y--;
				if (mouseState.position.y > 768 - 128)
					cameraOffset.y++;
			}
			else
			{
				drawing = true;
				if (drawnLine.empty())
					drawnLine.push_back(pointPos);
			}
		}
		cameraOffset = glm::clamp(glm::vec2(-1, -1), glm::vec2(1, 1), cameraOffset);
	}
	else if (mouseState.leftButton && lastMouseState.leftButton)
	{
		if (drawing && glm::distance(pointPos, drawnLine[drawnLine.size() - 1]) > 10 && (glm::distance(pointPos, drawnLine[drawnLine.size() - 1]) < 40 || drawnLine.size() > 1) && !drawingReachedCheckpoint())
		{
			blib::math::Line l(drawnLine[drawnLine.size() - 1], pointPos);
			if (!collidesWalls(l))
				drawnLine.push_back(pointPos);
		}
	}
	else if (!mouseState.leftButton && lastMouseState.leftButton)
	{
		drawing = false;

	}

	for (Zombie& z : currentState.zombies)
	{
		if (!z.alive)
			continue;
		z.nextSound -= (float)elapsedTime;
		if (z.nextSound <= 0)
		{
			blib::AnimatableSprite* e = new blib::AnimatableSprite(zombieSoundTexture, blib::math::Rectangle(z.position - (zombieSoundTexture->center/2.0f), zombieSoundTexture->originalWidth/2, zombieSoundTexture->originalHeight/2));
			e->alphaTo(0, 2);
			e->resizeTo(glm::vec2(2,2),2);
			e->rotation = blib::math::randomFloat(0, 360);

			effects.push_back(e);
			z.nextSound = blib::math::randomFloat(6, 12) * glm::distance(z.position, currentState.playerPosition) / 750.0f;
		}
	}


	walkBlendFactor = glm::clamp(walkBlendFactor + (walking ? (float)-elapsedTime : (float)elapsedTime), 0.0f, 1.0f);

	if (walking)
	{
		playerAnimation->setState("walk");
		walkIndex += (float)(elapsedTime * 150);
		bool inBounds = false;
		float a = 0;
		for (int i = 0; i < (int)drawnLine.size() - 1 && a < walkIndex; i++)
		{
			float d = glm::distance(drawnLine[i], drawnLine[i + 1]);
			if (a + d > walkIndex)
			{
				float f = walkIndex - a;
				float factor = f / d;
				glm::vec2 prevPos = currentState.playerPosition;
				currentState.playerPosition = glm::mix(drawnLine[i], drawnLine[i + 1], factor);
				inBounds = true;
				break;
			}
			else
				a += d;
		}
		if (!inBounds)
		{
			glm::vec2 firstPoint = drawnLine[0];
			glm::vec2 lastPoint = drawnLine[drawnLine.size() - 1];
			currentState.playerPosition = blib::linq::firstOrDefault(checkpoints, [&lastPoint, &firstPoint](const glm::vec2& c) { return glm::distance(c, lastPoint) < 60 && glm::distance(c, firstPoint) > 60; }, glm::vec2(0, 0));
			walking = false;
			drawnLine.clear();
			drawnLine.push_back(currentState.playerPosition);
			drawing = false;
			lastState = currentState;
		}
		cameraTarget = currentState.playerPosition;
		if (mouseState.leftButton)
		{
			float targetAngle = glm::degrees(atan2(currentState.playerPosition.y - pointPos.y, currentState.playerPosition.x - pointPos.x));
			float diff = targetAngle - currentState.playerRotation;
			while (diff < -180)
				diff += 360;
			while (diff > 180)
				diff -= 360;
			if (fabs(diff) < 700 * elapsedTime)
				currentState.playerRotation = targetAngle;
			else if (diff < 0)
				currentState.playerRotation -= (float)(700 * elapsedTime);
			else if (diff > 0)
				currentState.playerRotation += (float)(700 * elapsedTime);

			gunTimer -= elapsedTime;
			if (!lastMouseState.leftButton || gunTimer < 0)
			{
				if (currentState.weapon == State::Weapon::MachineGun)
					gunTimer = 0.1f;
				else
					gunTimer = 1;
				blib::math::Line gunLine(currentState.playerPosition, currentState.playerPosition + 1200.0f * glm::normalize(pointPos - currentState.playerPosition));
				for (const blib::math::Polygon& o : visionObjects)
				{
					std::vector<std::pair<glm::vec2, blib::math::Line>> collisions;
					if (o.intersects(gunLine, &collisions))
						for (auto c : collisions)
							if (glm::distance(c.first, gunLine.p1) < gunLine.length())
								gunLine.p2 = c.first;
					for (Zombie& z : currentState.zombies)
						if (z.alive && glm::distance(gunLine.project(z.position), z.position) < 30)
							gunLine.p2 = gunLine.project(z.position);
				}
				gunLines.push_back(std::pair<blib::math::Line, float>(gunLine,1.0f));

				for (Zombie& z : currentState.zombies)
				{
					glm::vec2 projection = gunLine.project(z.position);
					if (glm::distance(projection, z.position) < 30 && z.alive)
					{
						z.hp--;
						if (z.hp <= 0)
						{
							z.direction = glm::degrees(atan2(z.position.y - currentState.playerPosition.y, z.position.x - currentState.playerPosition.x));
							z.alive = false;
						}
					}
				}


			}




		}


		for (const Zombie& z : currentState.zombies)
		{
			if(z.alive && glm::distance(z.position, currentState.playerPosition) < 25)
			{	//die!
				currentState = lastState;
				drawing = true;
				walking = false;
				drawnLine.clear();
				drawnLine.push_back(currentState.playerPosition);
				cameraTarget = currentState.playerPosition;
				break;
			}
		}

		for (size_t i = 0; i < currentState.pickups.size(); i++)
		{
			if (glm::distance(currentState.pickups[i].pos, currentState.playerPosition) < 50)
			{
				currentState.weapon = (State::Weapon)currentState.pickups[i].type;

				blib::AnimatableSprite* e = new blib::AnimatableSprite(pickupTextures[currentState.pickups[i].type], currentState.pickups[i].pos);
				e->resizeTo(glm::vec2(100, 100), 0.2f);
				e->alphaTo(0, 0.2f);
				effects.push_back(e);
				currentState.pickups.erase(currentState.pickups.begin() + i);
				break;
			}
		}
		for (Zombie& z : currentState.zombies)
		{
			if (!z.alive)
				continue;

			glm::vec2 oldPos = z.position;


			if (z.state == Zombie::State::ROTATING)
				z.direction += 100 * (float)elapsedTime;
			if (z.state == Zombie::State::WANDER)
				z.position += 100 * (float)elapsedTime * blib::util::fromAngle(glm::radians(z.direction));

			z.timer -= (float)elapsedTime;
			if (z.timer <= 0)
			{
				if (z.state == Zombie::State::IDLE)
				{
					z.state = rand() % 2 == 0 ? Zombie::State::ROTATING : Zombie::State::WANDER;
					z.timer = blib::math::randomFloat(0.25f, 1.0f);
					if (z.state == Zombie::State::WANDER)
						z.timer *= blib::math::randomFloat(1, 3);
				}
				else if (z.state == Zombie::State::ROTATING || z.state == Zombie::State::WANDER)
				{
					z.state = Zombie::State::IDLE;
					z.timer = blib::math::randomFloat(1.0f, 2.0f);
				}
			}

			if (z.state == Zombie::State::IDLE || z.state == Zombie::State::ROTATING || z.state == Zombie::State::WANDER)
			{
				float diff = z.direction - glm::degrees(atan2(z.position.y - currentState.playerPosition.y, z.position.x - currentState.playerPosition.x));
				if (diff < 180)
					diff += 360;
				if (diff > 180)
					diff -= 360;
				if (fabs(diff) < 45 && glm::distance(currentState.playerPosition, z.position) < 1000)
				{
					if (!collidesWalls(blib::math::Line(currentState.playerPosition, z.position)))
						z.state = Zombie::State::CHASE;
				}
			}

			if (z.state == Zombie::State::CHASE)
			{
				z.direction = glm::degrees(atan2(currentState.playerPosition.y - z.position.y, currentState.playerPosition.x - z.position.x));
				z.position += 100 * (float)elapsedTime * blib::util::fromAngle(glm::radians(z.direction));
			}



			blib::math::Line ray(oldPos, z.position);
			std::vector<std::pair<glm::vec2, blib::math::Line> > collisions;
			glm::vec2 point;
			blib::math::Line hitLine;
			bool collided = true;
			int iterations = 0;
			while (collided && iterations < 4)
			{
				iterations++;
				collided = false;
				for (size_t i = 0; i < wallObjects.size(); i++)
				{
					if (!wallAabb[i].intersect(ray))
						continue;
					blib::math::Polygon& o = wallObjects[i];
					if (o.intersects(ray, &collisions))
					{
						for (size_t ii = 0; ii < collisions.size(); ii++)
						{
							glm::vec2 newPos = collisions[ii].second.project(z.position);
							z.position = newPos + 1.0001f * collisions[ii].second.normal();
							ray.p2 = z.position;
							collided = true;
							z.state = Zombie::State::ROTATING;
							z.timer = blib::math::randomFloat(0.5, 1);
						}
						break;
					}
				}
			}

		}

		zombieIdleAnimation->update(elapsedTime);
		zombieWalkAnimation->update(elapsedTime);
		zombieDeadAnimation->update(elapsedTime);
	}
	else // not walking
		playerAnimation->setState("idle");

	//PC hover mouse
	{
		float targetAngle = glm::degrees(atan2(currentState.playerPosition.y - pointPos.y, currentState.playerPosition.x - pointPos.x));
		float diff = targetAngle - currentState.playerRotation;
		while (diff < -180)
			diff += 360;
		while (diff > 180)
			diff -= 360;
		if (fabs(diff) < 700 * elapsedTime)
			currentState.playerRotation = targetAngle;
		else if (diff < 0)
			currentState.playerRotation -= (float)(700 * elapsedTime);
		else if (diff > 0)
			currentState.playerRotation += (float)(700 * elapsedTime);
	}



	for (int i = 0; i < (int)gunLines.size(); i++)
	{
		gunLines[i].second -= (float)elapsedTime * 5000.0f / gunLines[i].first.length();
		if (gunLines[i].second < 0)
		{
			gunLines.erase(gunLines.begin() + i);
			i--;
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


	float fac = 0.95f;
	cameraPos = fac * cameraPos + (1 - fac) * (cameraTarget + glm::vec2(1024,768) * cameraOffset);

	if (cameraPos.x < 1024 / zoom / 2)
		cameraPos.x = 1024 / zoom / 2;
	if (cameraPos.y < 768 / zoom / 2)
		cameraPos.y = 768 / zoom / 2;

	playerAnimation->update(elapsedTime);

#pragma region editor
	if (mouseState.rightButton && !lastMouseState.rightButton)
	{
		if (newPolygon.empty())
		{
			dragNode.first = -1;
			for (size_t i = 0; i < visionObjects.size(); i++)
				for (size_t ii = 0; ii < visionObjects[i].size(); ii++)
					if (glm::distance(visionObjects[i][ii], pointPos) < 5)
						dragNode = std::pair<int, int>(i, ii);
		}

		if (dragNode.first == -1)
			newPolygon.push_back(pointPos);
		
	}
	else if (mouseState.rightButton && lastMouseState.rightButton)
	{
		if (dragNode.first != -1)
		{
			visionObjects[dragNode.first][dragNode.second] = pointPos;
			visionObjects[dragNode.first].resetLines();
			calculateAabb();
			saveMap();
		}
	}
	if ((keyState.isPressed(blib::Key::A) || keyState.isPressed(blib::Key::S)) && keyState.isPressed(blib::Key::CONTROL) && !newPolygon.empty())
	{
		ClipperLib::Clipper clipper;
		clipper.AddPolygons(blib::linq::select<ClipperLib::Polygons>(visionObjects, [](const blib::math::Polygon& p) { return p.toClipperPolygon(); }), ClipperLib::ptSubject);
		clipper.AddPolygon(newPolygon.toClipperPolygon(), ClipperLib::ptClip);
		ClipperLib::Polygons result;
		if (keyState.isPressed(blib::Key::A))
			clipper.Execute(ClipperLib::ctUnion, result);
		else //s
			clipper.Execute(ClipperLib::ctDifference, result);

		visionObjects.clear();
		for (auto p : result)
			visionObjects.push_back(p);

		calculateAabb();
		saveMap();
		newPolygon.clear();
	}
	if (keyState.isPressed(blib::Key::C) && keyState.isPressed(blib::Key::CONTROL) && !newPolygon.empty())
		newPolygon.clear();
	if (keyState.isPressed(blib::Key::X) && keyState.isPressed(blib::Key::CONTROL) && newPolygon.empty())
	{
		for (size_t i = 0; i < visionObjects.size(); i++)
		{
			for (size_t ii = 0; ii < visionObjects[i].size(); ii++)
			{
				if (glm::distance(visionObjects[i][ii], pointPos) < 5)
				{
					Log::out << "Removing vert" << Log::newline;
					visionObjects[i].erase(visionObjects[i].begin() + ii);
					visionObjects[i].resetLines();
				}
			}
		}
		for (size_t i = 0; i < visionObjects.size(); i++)
			if (visionObjects[i].size() <= 2)
				visionObjects.erase(visionObjects.begin() + i);
		dragNode.first = -1;
		calculateAabb();
		saveMap();
	}
	if (keyState.isPressed(blib::Key::Z) && !lastKeyState.isPressed(blib::Key::Z) && keyState.isPressed(blib::Key::CONTROL))
	{
		blib::json::Value zombies;
		for (auto z : currentState.zombies)
		{
			blib::json::Value jz;
			jz["pos"].push_back(z.position.x);
			jz["pos"].push_back(z.position.y);
			jz["rot"] = z.direction;
			zombies.push_back(jz);
		}
#ifdef BLIB_WIN
		zombies.prettyPrint(std::ofstream("zombies.txt"));
#endif
	}
	if (keyState.isPressed(blib::Key::Z) && !lastKeyState.isPressed(blib::Key::Z) && keyState.isPressed(blib::Key::SHIFT))
	{
		for (size_t i = 0; i < currentState.zombies.size(); i++)
			if (glm::distance(currentState.zombies[i].position, pointPos) < 32)
				currentState.zombies.erase(currentState.zombies.begin() + i);
	}
	else if (keyState.isPressed(blib::Key::Z) && !lastKeyState.isPressed(blib::Key::Z))
		currentState.zombies.push_back(Zombie(pointPos, (float)(rand() % 360)));
	else if (keyState.isPressed(blib::Key::F1) && !lastKeyState.isPressed(blib::Key::F1))
		hideZombies = !hideZombies;
#pragma endregion editor

	lineOffset += (float)elapsedTime;
	while (lineOffset > 1)
		lineOffset--;

	lastKeyState = keyState;
	lastMouseState = mouseState;
}


void ZombieDraw::draw()
{
	renderer->clear(glm::vec4(0, 0, 0, 1), blib::Renderer::Color);

	cameraMat = glm::mat4();
	cameraMat = glm::translate(cameraMat, glm::vec3(1024/2, 768/2, 0));
	cameraMat = glm::scale(cameraMat, glm::vec3(zoom, zoom, 1.0f));
	cameraMat = glm::translate(cameraMat, glm::vec3(-cameraPos, 0));

#pragma region Vision
	blib::RenderState state = lineBatch->renderState;
	state.activeShader->setUniform(blib::LineBatch::Uniforms::matrix, cameraMat);
	state.activeVbo = NULL;
	state.activeFbo = visionFbo;
	state.stencilTestEnabled = true;
	renderer->clear(glm::vec4(0.5f, 0.5f, 0.5f, 1), blib::Renderer::Color | blib::Renderer::Stencil, state);

	glm::vec2 lightPoint(currentState.playerPosition);

	std::vector<blib::VertexP2C4> verts;
	for (const blib::math::Polygon& o : visionObjects)
	{
		for (size_t i = 0; i < o.size(); i++)
		{
			const glm::vec2& v1 = o[i];
			const glm::vec2& v2 = o[(i + 1) % o.size()];

			blib::math::Line l(v1, v2);
			if (l.side(lightPoint))
				continue;


			const glm::vec2& v3 = v1 + 50.0f * (v1 - lightPoint);
			const glm::vec2& v4 = v2 + 50.0f * (v2 - lightPoint);;

			glm::vec4 c(0.25f, 0.25f, 0.25f, 1.0f);
			verts.push_back(blib::VertexP2C4(v1, c));
			verts.push_back(blib::VertexP2C4(v2, c));
			verts.push_back(blib::VertexP2C4(v3, c));

			verts.push_back(blib::VertexP2C4(v3, c));
			verts.push_back(blib::VertexP2C4(v4, c));
			verts.push_back(blib::VertexP2C4(v2, c));
		}
	}


	renderer->clear(glm::vec4(1, 1, 1, 0.5f), blib::Renderer::Stencil, state);
	state.stencilWrite = true;
	renderer->drawTriangles(verts, state);
	state.stencilWrite = false;
	state.blendEnabled = false;


	//if (walkBlendFactor > 0)
	//else
	{
		glm::vec4 c = glm::mix(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), walkBlendFactor);
		std::vector<blib::VertexP2C4> verts2{
			blib::VertexP2C4(cameraPos + glm::vec2(-1024, -768) / zoom, c),
			blib::VertexP2C4(cameraPos + glm::vec2(1024, -768) / zoom, c),
			blib::VertexP2C4(cameraPos + glm::vec2(-1024, 768) / zoom, c),

			blib::VertexP2C4(cameraPos + glm::vec2(1024, 768) / zoom, c),
			blib::VertexP2C4(cameraPos + glm::vec2(1024, -768) / zoom, c),
			blib::VertexP2C4(cameraPos + glm::vec2(-1024, 768) / zoom, c),

		};
		renderer->drawTriangles(verts2, state);
	}

	{
		glm::vec4 c(1.0f, 1.0f, 1.0f, 1.0f);
		std::vector<blib::VertexP2C4> verts2{
			blib::VertexP2C4(currentState.playerPosition, c),
			blib::VertexP2C4(currentState.playerPosition + 3000.0f * blib::util::fromAngle(glm::radians(180 + currentState.playerRotation - 45)), c),
			blib::VertexP2C4(currentState.playerPosition + 3000.0f * blib::util::fromAngle(glm::radians(180 + currentState.playerRotation + 45)), c),
		};
		renderer->drawTriangles(verts2, state);
	}



#pragma endregion



	blib::Shader* s = spriteBatch->renderState.activeShader;
	spriteBatch->renderState.activeShader = spriteBatch->shader = combineShader;
	spriteBatch->renderState.activeTexture[1] = visionFbo;


	spriteBatch->begin(cameraMat);

	for (int i = 0; i < 8; i++)
	{
		int x = i % 4;
		int y = i / 4;
		spriteBatch->draw(mapTextures[i], blib::math::easyMatrix(glm::vec2(x*1476.5f, y * 1476.5f), 0, 0.5f));
	}


	for (auto checkpoint : checkpoints)
		spriteBatch->draw(checkpointTexture, blib::math::easyMatrix(checkpoint), checkpointTexture->center);

	for (auto pickup : currentState.pickups)
	{
		spriteBatch->draw(pickupTextures[pickup.type], blib::math::easyMatrix(pickup.pos,0,2), pickupTextures[pickup.type]->center);
	}


	playerAnimation->draw(*spriteBatch, blib::math::easyMatrix(currentState.playerPosition, currentState.playerRotation + 90));

	spriteBatch->end();
	if(hideZombies)
		spriteBatch->shader->setUniform(ShaderAttributes::zombieFactor, 1.0f);
	spriteBatch->begin(cameraMat);
	for (auto z : currentState.zombies)
	{
		if (!z.alive){}
		else if (z.state == Zombie::State::WANDER || z.state == Zombie::State::CHASE)
			zombieWalkAnimation->draw(*spriteBatch, blib::math::easyMatrix(z.position, z.direction + 90));
		else
			zombieIdleAnimation->draw(*spriteBatch, blib::math::easyMatrix(z.position, z.direction + 90));
	}
	spriteBatch->end();
	spriteBatch->shader->setUniform(ShaderAttributes::zombieFactor, 0.0f);
	spriteBatch->begin(cameraMat);

	for (auto z : currentState.zombies)
	{
		if (!z.alive)
			zombieDeadAnimation->draw(*spriteBatch, blib::math::easyMatrix(z.position, z.direction + 90));
	}

	spriteBatch->end();





	glm::vec4 lineColor = blib::Color::white;
	if (walking)
		lineColor.a = 0.25f;

	float index = (float)-lineOffset;
	std::vector<blib::VertexP2T2C4> lineVerts;
	for (int i = 0; i < (int)drawnLine.size() - 1; i++)
	{
		glm::vec2 diff = drawnLine[i + 1] - drawnLine[i];
		glm::vec2 diff2 = diff;
		if (i < (int)drawnLine.size() - 2)
			diff2 = drawnLine[i + 2] - drawnLine[i + 1];
		float len = glm::length(diff);

		glm::vec2 normal = glm::normalize(glm::vec2(-diff.y, diff.x));
		glm::vec2 normal2 = glm::normalize(glm::vec2(-diff2.y, diff2.x));

		float index2 = index + len / 128.0f;

		lineVerts.push_back(blib::VertexP2T2C4(drawnLine[i] + 4.0f * normal, glm::vec2(index, 1), lineColor));
		lineVerts.push_back(blib::VertexP2T2C4(drawnLine[i + 1] + 4.0f * normal2, glm::vec2(index2, 1), lineColor));
		lineVerts.push_back(blib::VertexP2T2C4(drawnLine[i + 1] - 4.0f * normal2, glm::vec2(index2, 0), lineColor));

		lineVerts.push_back(blib::VertexP2T2C4(drawnLine[i] + 4.0f * normal, glm::vec2(index, 1), lineColor));
		lineVerts.push_back(blib::VertexP2T2C4(drawnLine[i] - 4.0f * normal, glm::vec2(index, 0), lineColor));
		lineVerts.push_back(blib::VertexP2T2C4(drawnLine[i + 1] - 4.0f * normal2, glm::vec2(index2, 0), lineColor));

		index = index2;
	}
	spriteBatch->renderState.activeShader = spriteBatch->shader = s;
	spriteBatch->renderState.activeShader->setUniform(blib::SpriteBatch::ShaderAttributes::Matrix, cameraMat);
	spriteBatch->renderState.activeTexture[1] = NULL;
	spriteBatch->renderState.activeTexture[0] = lineTexture;
	blib::VBO* tmpVbo = spriteBatch->renderState.activeVbo;
	spriteBatch->renderState.activeVbo = NULL;
	renderer->drawTriangles(lineVerts, spriteBatch->renderState);
	spriteBatch->renderState.activeVbo = tmpVbo;

	spriteBatch->begin(cameraMat);

	for (auto e : effects)
		e->draw(spriteBatch);

	lineBatch->begin(cameraMat);
#ifdef _DEBUG
	lineBatch->draw(newPolygon);
	for (const blib::math::Polygon &p : visionObjects)
	{
		lineBatch->draw(p, glm::vec4(1, 0, 1, 1));
		for (const glm::vec2 v : p)
		{
			lineBatch->draw(blib::math::Rectangle(v-glm::vec2(5,5), 10,10), glm::vec4(1,1,1,1));
		}
	}
	for (const blib::math::Polygon &p : wallObjects)
		lineBatch->draw(p, glm::vec4(1, 0, 0, 1));
#endif

	for (const std::pair<blib::math::Line, float>& p : gunLines)
	{
		glm::vec2 point = glm::mix(p.first.p1, p.first.p2, 1 - p.second);
		lineBatch->draw(p.first.p1, point, glm::vec4(1, 1, 1, p.second));
		spriteBatch->draw(bulletTexture, blib::math::easyMatrix(point, glm::degrees(p.first.angle()), 0.5f), bulletTexture->center);

	}
	lineBatch->end();


	if (showBob)
		spriteBatch->draw(bobTexture, blib::math::easyMatrix(currentState.playerPosition - glm::vec2(150, 128), 0, 0.5f));

	if (drawingReachedCheckpoint() && !walking)
		spriteBatch->draw((int)(time * 3) % 2 == 0 ? playButtonTexture : buttonTexture, blib::math::easyMatrix(currentState.playerPosition, 0, 0.5f), playButtonTexture->center);
	else if (!walking && drawnLine.size() > 1)
		spriteBatch->draw((int)(time * 3) % 2 == 0 ? undoButtonTexture : buttonTexture, blib::math::easyMatrix(currentState.playerPosition, 0, 0.5f), playButtonTexture->center);
	spriteBatch->end();


	if (!walking && !drawing)
	{
		spriteBatch->begin();

		if (cameraOffset.y > -1)
		{
			if (cameraOffset.x > -1)
				spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(0, 0), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.25f, 0.5f, 0.25f, 0.5f));
			spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(512 - panButtonTexture->originalWidth / 4 / 2 / 2, 0), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.5f, 0.5f, 0.25f, 0.5f));
			if (cameraOffset.x < 1)
				spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(1024 - panButtonTexture->originalWidth / 4 / 2, 0), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.75f, 0.5f, 0.25f, 0.5f));
		}

		if (cameraOffset.x > -1)
			spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(0, 384 - panButtonTexture->originalHeight / 2 / 2 / 2), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.5f, 0.25f, 0.5f));
		if (cameraOffset.x < 1)
			spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(1024 - panButtonTexture->originalWidth / 4 / 2, 384 - panButtonTexture->originalHeight / 2 / 2 / 2), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.0f, 0.0f, 0.25f, 0.5f));


		if (cameraOffset.y < 1)
		{
			if (cameraOffset.x > -1)
				spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(0, 768 - panButtonTexture->originalHeight / 2 / 2), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.75f, 0.0f, 0.25f, 0.5f));
			spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(512 - panButtonTexture->originalWidth / 4 / 2 / 2, 768 - panButtonTexture->originalHeight / 2 / 2), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.5f, 0.0f, 0.25f, 0.5f));
			if (cameraOffset.x < 1)
				spriteBatch->draw(panButtonTexture, blib::math::easyMatrix(glm::vec2(1024 - panButtonTexture->originalWidth / 4 / 2, 768 - panButtonTexture->originalHeight / 2 / 2), 0, 0.5f), glm::vec2(0, 0), blib::math::Rectangle(0.25f, 0.0f, 0.25f, 0.5f));
		}
		spriteBatch->end();
	}
}

bool ZombieDraw::drawingReachedCheckpoint()
{
	glm::vec2 firstPoint = drawnLine[0];
	glm::vec2 lastPoint = drawnLine[drawnLine.size() - 1];
	return blib::linq::any(checkpoints, [&lastPoint, &firstPoint](const glm::vec2& c) { return glm::distance(c, lastPoint) < 60 && glm::distance(c, firstPoint) > 60; });
}

void ZombieDraw::saveMap()
{
	blib::json::Value mapData;
	for (auto p : visionObjects)
	{
		blib::json::Value o;
		for (auto n : p)
		{
			blib::json::Value v;
			v.push_back(n.x);
			v.push_back(n.y);
			o.push_back(v);
		}
		mapData.push_back(o);
	}
#ifdef BLIB_WIN
	mapData.prettyPrint(std::ofstream("map.txt"));
#endif
}

void ZombieDraw::calculateAabb()
{
	visionAabb.clear();
	for (const blib::math::Polygon& polygon : visionObjects)
		visionAabb.push_back(polygon.getBoundingBox());
	wallAabb.clear();
	for (const blib::math::Polygon& polygon : wallObjects)
		wallAabb.push_back(polygon.getBoundingBox());
}

bool ZombieDraw::collidesWalls(const blib::math::Line &line)
{
	for (size_t i = 0; i < wallAabb.size(); i++)
		if (wallAabb[i].intersect(line))
			if (wallObjects[i].intersects(line))
				return true;
	return false;
}




