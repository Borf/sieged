#include <blib/app.h>

#include <vector>
#include <map>
#include <blib/math/Polygon.h>
#include <blib/math.h>


namespace blib { class Texture; class Animation; class FBO; class Shader; class AnimatableSprite; class Font; }
class Tile;
typedef std::vector<std::vector<Tile*> > TileMap;

class BuildingTemplate
{
public:
	enum Type
	{
		TownHall,
		StoneMason,
		Farm,
		MarketPlace,
		ArcheryRange,
		WizardTower,
		Smithy,
		Tavern,
		WatchTower,
		AlchemyLabs,
	} type;
	glm::ivec2 size;

	BuildingTemplate(Type t, const glm::ivec2 &size) { this->type = t; this->size = size; }
};

class Building
{
public:
	glm::ivec2 position;
	BuildingTemplate* buildingTemplate;

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


	Enemy(glm::vec2 p) { this->position = p; }
};

class Sieged : public blib::App
{
	std::map<BuildingTemplate::Type, BuildingTemplate*> buildingTemplates;
	std::vector<Building*> buildings;
	std::vector<Enemy*> enemies;

	blib::Font* font;
	blib::Texture* tileTexture;
	blib::Texture* arrowsTexture;
	blib::Texture* enemyTexture;
	TileMap tiles;



	blib::MouseState prevMouseState;

public:
	Sieged();
	virtual void init() override;
	virtual void update(double elapsedTime) override;
	virtual void draw() override;
	void calcPaths();
};