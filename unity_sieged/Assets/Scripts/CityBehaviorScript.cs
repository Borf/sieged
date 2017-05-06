using Assets.Scripts.Lib;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class CityBehaviorScript : MonoBehaviour
{
    public float Delay = 0.5f;
    public GameObject TownhallTemplate;
    public List<GameObject> BuildingTemplates;
    public List<GameObject> WallTemplates;
    public List<GameObject> TowerTemplates;
    public int Population = 0;

    private HashSet<Point> buildPositionsHouses = new HashSet<Point>();
    private HashSet<Point> buildPositionsWalls = new HashSet<Point>();
    private Dictionary<BuildingType, List<GameObject>> Buildings;

    private List<Point> offsets = new List<Point> { new Point(0, 1), new Point(0, -1), new Point(1, 0), new Point(-1, 0) };
    private List<Point> diagonalOffsets = new List<Point> { new Point(1, 1), new Point(-1, 1), new Point(1, -1), new Point(-1, -1) };

    public Grid Grid { get; set; }

    // Use this for initialization
    void Start()
    {
        Buildings = new Dictionary<BuildingType, List<GameObject>>();
        foreach (var buildingType in Enum.GetValues(typeof(BuildingType)).Cast<BuildingType>())
        {
            Buildings[buildingType] = new List<GameObject>();
        }

        Grid = new Grid(100, 100);
        SpawnBuilding(new Point(Grid.Width / 2, Grid.Height / 2), TownhallTemplate, BuildingType.Townhall);

        StartCoroutine(spawnStuff());
    }

    // Update is called once per frame
    void Update()
    {

    }

    public List<Point> GetAllPointsWithOffset(Point pos)
    {
        return offsets.Select(o => o + pos).ToList();
    }

    public bool SpawnBuilding(Point pos, GameObject template, BuildingType buildingType)
    {
        if (!CanSpawn(pos, template))
            return false;

        // Create building object
        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        var building = Instantiate(template, new Vector3(pos.X + buildingTemplate.Width / 2.0f, 0, pos.Y + buildingTemplate.Height / 2.0f), Quaternion.identity, gameObject.transform);
        building.isStatic = true;

        Buildings[buildingType].Add(building);

        // Handling of different building types
        if (buildingType == BuildingType.House)
            Population++;

        // Random rotation
        if (BuildingTemplates.Contains(template))
            building.transform.Rotate(new Vector3(0, UnityEngine.Random.Range(0, 360), 0));

        // Place building
        List<Point> newPoints = new List<Point>();
        foreach (var x in Enumerable.Range(pos.X, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(pos.Y, buildingTemplate.Height))
            {
                var newPoint = new Point(x, y);
                Grid.UpdateTile(newPoint, buildingType, building);
                newPoints.Add(newPoint);
            }
        }


        // Determine new buildable positions
        HashSet<Point> newNeighbours = new HashSet<Point>();
        foreach (Point p in newPoints)
            foreach (Point offset in offsets)
                if (!newPoints.Contains(p + offset) && !Grid[p + offset].HasBuilding)
                    newNeighbours.Add(p + offset);

        buildPositionsWalls.RemoveWhere(newPoints.Contains);  // Remove current built building
        buildPositionsHouses.RemoveWhere(newPoints.Contains);  // Remove current built building

        if (buildingType == BuildingType.House || buildingType == BuildingType.Townhall)
        {
            buildPositionsHouses.UnionWith(newNeighbours);
            buildPositionsWalls.RemoveWhere(newNeighbours.Contains);
        }
        else if (buildingType == BuildingType.Wall || buildingType == BuildingType.Tower)
        {
            foreach (var newNeighbor in newNeighbours)
            {
                if (buildPositionsHouses.Contains(newNeighbor))
                {
                    // skip
                }
                else
                {
                    buildPositionsWalls.Add(newNeighbor);
                }
            }
        }

        return true;
    }

    private bool CanSpawn(Point pos, GameObject template)
    {
        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        foreach (var x in Enumerable.Range(pos.X, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(pos.Y, buildingTemplate.Height))
            {
                if (Grid[x, y].HasBuilding)
                    return false;
            }
        }
        return true;
    }

    internal void DestroyBuilding(Point pos)
    {
        if (Grid.IsOutOfBounds(pos))
            return;
        if (Grid.IsEmpty(pos))
            return;

        GameObject building = Grid[pos].Building;

        BuildingTemplate template = building.GetComponent<BuildingTemplate>();

        var buildingType = Grid[pos].BuildingType;
        if (buildingType == BuildingType.House) // // Update population
            Population--;

        if (buildingType == BuildingType.Townhall) // Don't allow destruction of town hall
            return;

        for (int x = 0; x < template.Width; x++)
            for (int y = 0; y < template.Height; y++)
                Grid.UpdateTile(pos + new Point(x, y), BuildingType.None, null);

        Buildings[buildingType].Remove(building);
        GameObject.Destroy(building);
    }

    internal void changeToTower(Point pos)
    {
        if (Grid.IsOutOfBounds(pos))
            return;

        //if (Grid[pos.X, pos.Y].Building.Template != wallTemplates.First()) //TODO: if is wall
        //    return;

        Grid.UpdateTile(pos, BuildingType.None, null);
        SpawnBuilding(pos, TowerTemplates.First(), BuildingType.Tower);
    }

    internal void SpawnWall(Point pos)
    {
        if (!Grid.IsEmpty(pos))
            DestroyBuilding(pos);
        SpawnBuilding(pos, WallTemplates.First(), BuildingType.Wall);
    }

    public IEnumerator spawnStuff()
    {
        while (true)
        {
            for (int i = 0; i < 20; i++)
            {
                /*var neighbors = Grid.GetEmptyNeighbors();

                if (!neighbors.Any())
                    break;

                var pos = neighbors[UnityEngine.Random.Range(0, neighbors.Count - 1)];*/

                Point pos = null;

                if (buildPositionsHouses.Any())
                {
                    pos = buildPositionsHouses.GetRandomElement();
                }
                else if (buildPositionsWalls.Any())
                {
                    pos = buildPositionsWalls.GetRandomElement();
                }

                if (pos == null)
                    break;

                SpawnBuilding(pos, BuildingTemplates.First(), BuildingType.House);

            }

            yield return new WaitForSeconds(Delay);
        }
    }

    private bool TrySpawnNeighbor(Point pos)
    {
        foreach (var newPos in offsets.Select(o => o + pos))
        {
            if (SpawnBuilding(newPos, BuildingTemplates.First(), BuildingType.House))
                return true;
        }

        foreach (var newPos in diagonalOffsets.Select(o => o + pos))
        {
            if (SpawnBuilding(newPos, BuildingTemplates.First(), BuildingType.House))
                return true;
        }
        return false;
    }
}