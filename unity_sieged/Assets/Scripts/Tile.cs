using UnityEngine;

public class Tile
{
    public Builder Builder;
    public int Neighbors;

    public bool HasBuilding { get { return Builder != Builder.None; } }
    public GameObject Building;
}

public enum Builder
{
    None,
    Player,
    Generated
}