#include <blib/app.h>

#include <vector>
#include <map>
#include <blib/math/Polygon.h>
#include <blib/math.h>
#include <blib/RenderState.h>


namespace blib { class Texture; class Animation; class FBO; class Shader; class AnimatableSprite; class Font; class Shader; class StaticModel; 
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
		toBase = 0;
	}

	bool isWall() { if (!building) return false; return building->buildingTemplate->type == BuildingTemplate::Wall; }

	enum Direction
	{
		Left = 1,Right = 2,Up = 4,Down = 8
	};
	int toBase;
};

class Enemy
{
public:
	glm::vec2 position;
	float speed;

	Enemy(glm::vec2 p) { this->position = p; this->speed = blib::math::randomFloat(0.25f, 1.5f); }
};

class Sieged : public blib::App
{
	std::map<BuildingTemplate::Type, BuildingTemplate*> buildingTemplates;
	std::vector<Building*> buildings;
	std::vector<Enemy*> enemies;
	std::vector<std::pair<BuildingTemplate*, float> > conveyorBuildings;
	std::vector<blib::math::Polygon> collisionWalls;

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

	blib::StaticModel* wallModels[6];

	BuildingTemplate* draggingBuilding;
	int conveyorDragIndex;

	TileMap tiles;
	struct
	{
		blib::AnimatableSprite* wall;
		blib::AnimatableSprite* market;
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


	
	float stoneMasonFactor = 1;
	float wallBuildSpeed = 1;
	float lightDirection = 0;


	int gold;






	enum class BuildMode
	{
		Normal,
		Wall,
		Destroy
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