#include <blib/app.h>

#include <vector>
#include <map>
#include <blib/math/Polygon.h>
#include <blib/math.h>
#include <blib/RenderState.h>
#include <blib/BackgroundTask.h>

namespace blib {
	class Texture; class Animation; class FBO; class Shader; class AnimatableSprite; class Font; class Shader; class StaticModel;
namespace json { class Value; }
}
class Tile;
typedef std::vector<std::vector<Tile*> > TileMap;

class BuildingTemplate
{
public:
	enum Type
	{
		Wall = 0,
		Gate,
		TownHall,
		StoneMason,
		Bank,
		MineralMine,
		MarketPlace,
		Recycler,
		ArcheryRange,
		Barracks,
		BattleArena,
		ImposingTauntingStatue,
		TeslaTower,
		Smithy,
		Tavern,
		WatchTower,
		AlchemyLabs,
		Workshop,
		Refinery
	} type;
	glm::ivec2 size;
	blib::TextureMap::TexInfo* texInfo;
	blib::StaticModel* model;

	int cost;
	int rngWeight;
	float buildTime;
	int hitpoints;
	float healthbarSize;

	BuildingTemplate(const blib::json::Value &data, blib::TextureMap* textureMap, blib::StaticModel* model);
};

class Building
{
public:
	glm::ivec2 position;
	BuildingTemplate* buildingTemplate;
	float buildTimeLeft;
	int damage;

	Building(const glm::ivec2 position, BuildingTemplate* buildingTemplate, TileMap& tilemap);
};

class Tile
{
public:
	enum FloorType
	{
		Grass
	} floor;
	Building* building;

	Tile()
	{
		building = NULL;
	}

	bool isWall() { if (!building) return false; return building->buildingTemplate->type == BuildingTemplate::Wall; }
};

class Enemy
{
public:
	glm::vec2 position;
	float speed;

	float timeLeftForAttack;
	int health;

	Enemy(glm::vec2 p) { this->position = p; this->speed = blib::math::randomFloat(0.25f, 1.5f); timeLeftForAttack = 0; health = 5; }
};


enum Direction
{
	Left = 1, Right = 2, Up = 4, Down = 8
};
class Flowmap
{
public:
	std::vector<std::vector<int> > flow;
	Building* targetBuilding;
	glm::ivec2 targetPosition;
	
	Flowmap()
	{
		flow.resize(100, std::vector<int>(100, 0));
		targetBuilding = NULL;
	}

private:
	Flowmap(const Flowmap& other) { throw "argh";  }
};



class Flag
{
public:
	Flowmap flowmap;
	glm::ivec2 position;

	Flag(const glm::ivec2 &p)
	{
		position = p;
	}

private:
	Flag(const Flag& other) { throw "argh"; }
};



class Sieged : public blib::App
{
	std::map<BuildingTemplate::Type, BuildingTemplate*> buildingTemplates;
	std::vector<Building*> buildings;
	std::vector<Enemy*> enemies;
	std::vector<std::pair<BuildingTemplate*, float> > conveyorBuildings;
	std::vector<blib::math::Polygon> collisionWalls;
	std::vector<Flag*> flags;
	TileMap tiles;
	Flowmap flowMap;

	std::list<Flowmap*> flowmaps;


	std::vector<std::tuple<glm::mat4, Building*, blib::StaticModel*> > wallCache;
	blib::StaticModel* enemyModel;

	blib::Font* font;
	blib::Font* font48;
	blib::Texture* gridTexture;
	blib::Texture* whitePixel;
	blib::Texture* tileTexture;
	blib::Texture* arrowsTexture;
	blib::Texture* enemyTexture;
	blib::Texture* conveyorTexture;
	blib::Texture* notEnoughGoldTexture;
	blib::TextureMap* conveyorBuildingTextureMap;

	std::vector<blib::AnimatableSprite*> effects;



	blib::FBO* shadowMap;
	blib::StaticModel* flagModel;
	blib::StaticModel* wallModels[6];

	BuildingTemplate* draggingBuilding;
	int conveyorDragIndex;


	union
	{
		blib::AnimatableSprite* buttons[5];
		struct  
		{
			blib::AnimatableSprite* wall;
			blib::AnimatableSprite* market;
			blib::AnimatableSprite* flag;
			blib::AnimatableSprite* soldiers;
			blib::AnimatableSprite* archers;
		};
	} buttons;

	glm::vec3 cameraCenter;
	float cameraDistance;
	float cameraRotation;
	float cameraAngle;

	const float conveyorSpeed = 50;
	float conveyorOffset;
	int rngTotalWeight;
	float conveyorBuildingsPerSecond;
	float lastConveyorBuilding = 0;
	float goldTimeLeft = 0;
	bool calculatingPaths = false;
	bool calculatePathsAgain = false;

	bool gamePlaying = false;

	float threatLevel;
	float speed = 1;

	float nextEnemySpawn = 0;


	
	float stoneMasonFactor = 1;
	float wallBuildSpeed = 1;
	float lightDirection = 0;

	int maxFlagCount = 0;


	int gold;



	blib::BackgroundTask<std::map<Flowmap*, std::vector<std::vector<int>>>>* pathCalculateThread;


	enum class BuildMode
	{
		Normal,
		Wall,
		Destroy,
		Flag,
	} mode;

	blib::MouseState prevMouseState;
	glm::vec4 mousePos3d;
	glm::vec4 mousePos3dBegin;

	blib::RenderState renderState;
	blib::Shader* shadowmapShader;
	blib::Shader* backgroundShader;
	blib::Shader* characterShader;

	enum class Uniforms
	{
		ProjectionMatrix,
		CameraMatrix,
		modelMatrix,
		colorMult,
		s_texture,
		s_shadowmap,
		buildFactor,
		location,
		shadowFac,

		shadowProjectionMatrix,
		shadowCameraMatrix,
		lightDirection,
	};
	glm::mat4 cameraMatrix;
	glm::mat4 projectionMatrix;
public:
	Sieged();
	virtual void init() override;
	virtual void update(double elapsedTime) override;
	virtual void draw() override;
	

	enum class RenderPass
	{
		ShadowMap,
		Final,
	};

	void drawWorld(RenderPass renderPass);

	void calcPaths();
	void calcWalls();
};