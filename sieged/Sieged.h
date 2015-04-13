#include <blib/app.h>

#include <vector>
#include <map>
#include <blib/math/Polygon.h>
#include <blib/math.h>
#include <blib/RenderState.h>
#include <blib/BackgroundTask.h>
#include <blib/SkelAnimatedModel.h>

#include "BuildingTemplate.h"
#include "FlowMap.h"

namespace blib {
	class Texture; class Animation; class FBO; class Shader; class AnimatableSprite; class Font; class Shader; class StaticModel;
namespace json { class Value; }
}
class Tile;
typedef std::vector<std::vector<Tile*> > TileMap;


class Building;
class Flag;
class Enemy;
class Soldier;
class Archer;








class Sieged : public blib::App
{
	std::map<BuildingTemplate::Type, BuildingTemplate*> buildingTemplates;
	std::vector<Building*> buildings;
	std::vector<Enemy*> enemies;
	std::vector<std::pair<BuildingTemplate*, float> > conveyorBuildings;
	std::vector<blib::math::Polygon> collisionWalls;
	std::vector<Flag*> flags;


	std::vector<Flag*> flagsToErase;

	std::vector<Soldier*> soldiers;
	std::vector<Archer*> archers;

	TileMap tiles;
	Flowmap flowMap;

	std::vector<Flowmap*> flowmaps;


	std::vector<std::tuple<glm::mat4, Building*, blib::StaticModel*> > wallCache;
	blib::StaticModel* enemyModel;
	blib::StaticModel* dudeModel;

	blib::SkelAnimatedModel* protobot;
	blib::SkelAnimatedModel::State* protoBotState;


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
	void damageSoldier(Soldier* attackTarget, int damage);
	void damageEnemy(Enemy* enemy, int damage);
};