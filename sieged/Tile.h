
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