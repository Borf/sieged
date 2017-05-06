using UnityEngine;

public class Tile
{
    public BuildingType BuildingType;
    public int NeighboringHouses;

    public bool HasBuilding { get { return BuildingType != BuildingType.None; } }

    public bool HasGeneratedBuilding { get { return BuildingType == BuildingType.House || BuildingType == BuildingType.Townhall; } }
    public GameObject Building;
}

public enum BuildingType
{
    None,
    Townhall,
    House,
    Wall,
    Tower
}

public enum HouseDesignation
{
    Growth,
    Construction,
    Sacrificion
}