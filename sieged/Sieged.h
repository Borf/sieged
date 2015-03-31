#include <blib/app.h>

#include <vector>
#include <map>
#include <blib/math/Polygon.h>
#include <blib/math.h>

namespace blib { class Texture; class Animation; class FBO; class Shader; class AnimatableSprite;  }

class Zombie
{
public:
	glm::vec2 position;
	float direction;
	bool alive;
	int hp;
	enum class State
	{
		IDLE,
		CHASE,
		ROTATING,
		WANDER,
	} state;

	float nextSound;

	float timer;

	Zombie(const glm::vec2& position, float direction) : position(position), direction(direction)
	{
		state = State::IDLE;
		timer = 0;
		alive = true;
		hp = 2;
		nextSound = blib::math::randomFloat(0, 7);
	}
};


class Pickup
{
public:
	glm::vec2 pos;
	enum class Type
	{
		None,
		Pistol,
		Axe,
		MachineGun,
		Meat,
		Box,
	} type;
	Pickup(const glm::vec2 &pos, Type type) : pos(pos), type(type) {}
};

class State
{
public:
	glm::vec2 playerPosition;
	float playerRotation;

	enum class Weapon
	{
		None,
		Pistol,
		Axe,
		MachineGun,
		Meat
	} weapon;

	std::vector<Pickup> pickups;
	std::vector<Zombie> zombies;
};




class ZombieDraw : public blib::App
{
public:
	std::vector<blib::Texture*> mapTextures;
	blib::Texture* checkpointTexture;
	blib::Texture* lineTexture;
	blib::Texture* buttonTexture;
	blib::Texture* playButtonTexture;
	blib::Texture* undoButtonTexture;
	blib::Texture* zombieSoundTexture;
	blib::Texture* bobTexture;
	blib::Texture* bulletTexture;
	blib::Texture* panButtonTexture;

	std::map<Pickup::Type, blib::Texture*> pickupTextures;

	std::vector<blib::AnimatableSprite*> effects;
	blib::Animation* playerAnimation;
	blib::Animation* zombieWalkAnimation;
	blib::Animation* zombieIdleAnimation;
	blib::Animation* zombieDeadAnimation;
	blib::FBO* visionFbo;
	blib::Shader* combineShader;
	enum ShaderAttributes
	{
		ProjectionMatrix,
		Matrix,
		s_texture,
		s_visionTexture,
		zombieFactor,
	};

	std::vector<glm::vec2> checkpoints;
	std::vector<blib::math::Polygon> visionObjects;
	std::vector<blib::math::Rectangle> visionAabb;

	std::vector<blib::math::Polygon> wallObjects;
	std::vector<blib::math::Rectangle> wallAabb;

	std::vector<glm::vec2> drawnLine;
	float lineOffset;

	std::vector<std::pair<blib::math::Line, float>> gunLines;


	std::pair<int, int> dragNode;	//editor
	blib::math::Polygon newPolygon; //editor


	glm::mat4 cameraMat;
	glm::vec2 cameraPos;
	glm::vec2 cameraTarget;
	glm::vec2 cameraOffset;
	float zoom;

	bool drawing;

	blib::MouseState lastMouseState;
	blib::KeyState lastKeyState;
	float walkBlendFactor;
	bool walking;
	float walkIndex;
	float gunTimer;

	State currentState;
	State lastState;

	bool hideZombies = true;
	bool showBob = true;

	void calculateAabb();
	bool collidesWalls(const blib::math::Line &line);


	ZombieDraw();
	virtual void init() override;
	virtual void update(double elapsedTime) override;
	virtual void draw() override;


	bool drawingReachedCheckpoint();
	void saveMap();
};