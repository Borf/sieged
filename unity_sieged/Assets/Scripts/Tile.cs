using UnityEngine;

public class Tile
{
    public BuildingType BuildingType;
    public int NeighboringHouses;

    public bool HasBuilding { get { return BuildingType != BuildingType.None; } }
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