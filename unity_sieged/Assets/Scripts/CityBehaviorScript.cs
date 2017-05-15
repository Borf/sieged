using Assets.Scripts.Lib;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class CityBehaviorScript : MonoBehaviour
{
    // Set via Unity
    public float SpawnDelay = 0.5f;
    public float ConvertHouseDelay = 1.0f;
    public GameObject TownhallTemplate;
    public List<GameObject> BuildingTemplates;
    public List<GameObject> WallTemplates;
    public List<GameObject> TowerTemplates;
    public int Population = 0;

    //[System.Serializable]
    public class customClass
    {
        public int customInt;
        public string customString;
    }

    public List<customClass> bla;

    // Private consts
    private List<Point> offsets = new List<Point> { new Point(0, 1), new Point(0, -1), new Point(1, 0), new Point(-1, 0) };
    private List<Point> diagonalOffsets = new List<Point> { new Point(1, 1), new Point(-1, 1), new Point(1, -1), new Point(-1, -1) };

    // Private state variables
    private HashSet<Point> buildPositionsHouses = new HashSet<Point>();
    private HashSet<Point> buildPositionsWalls = new HashSet<Point>();
    private Dictionary<BuildingType, List<GameObject>> Buildings;
    private Dictionary<HouseDesignation, List<GameObject>> Houses;

    // Public state variables
    public Grid Grid { get; set; }
    public CityParameterHandler ParameterHandler { get; set; }

    // Use this for initialization
    void Start()
    {
        Buildings = new Dictionary<BuildingType, List<GameObject>>();
        foreach (var buildingType in Helper.GetEnumValues<BuildingType>())
        {
            Buildings[buildingType] = new List<GameObject>();
        }

        Houses = new Dictionary<HouseDesignation, List<GameObject>>();
        foreach (var houseDesignation in Helper.GetEnumValues<HouseDesignation>())
        {
            Houses[houseDesignation] = new List<GameObject>();
        }

        Grid = new Grid(100, 100);
        SpawnBuilding(new Point(Grid.Width / 2, Grid.Height / 2), TownhallTemplate, BuildingType.Townhall);

        StartCoroutine(spawnStuff());
        StartCoroutine(convertHouseDesignations());
    }

    // Update is called once per frame
    void Update()
    {

    }

    public List<Point> GetAllPointsWithOffset(Point pos)
    {
        return offsets.Select(o => o + pos).ToList();
    }

    public bool SpawnBuilding(Point pos, GameObject templates, BuildingType buildingType)
    {
        return SpawnBuilding(pos, new List<GameObject>() { templates }, buildingType);
    }

    public bool SpawnBuilding(Point pos, IEnumerable<GameObject> templates, BuildingType buildingType)
    {
        // Determine what to build
        GameObject template = null;
        HouseDesignation houseDesignation = HouseDesignation.None;
        if (buildingType == BuildingType.House)
        {
            houseDesignation = DetermineNextHouseDesignation();
            template = templates.FirstOrDefault(t => t.name == "House." + houseDesignation.ToString());
        }
        else
        {
            template = templates.First();
        }

        if (!CanSpawn(pos, template))
            return false;


        if (buildingType == BuildingType.Tower)
            Debug.Log("Building " + template.name);

        // Create building object
        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        var building = Instantiate(template, new Vector3(pos.X + buildingTemplate.Width / 2.0f, 0, pos.Y + buildingTemplate.Height / 2.0f), Quaternion.identity, gameObject.transform);
        building.isStatic = true;

        Buildings[buildingType].Add(building);

        // Handling of different building types
        if (buildingType == BuildingType.House)
        {
            Houses[houseDesignation].Add(building);
            Population++;
        }

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
                    // skip, nothing to update
                }
                else
                {
                    buildPositionsWalls.Add(newNeighbor);
                }
            }
        }

        return true;
    }

    public HouseDesignation DetermineNextHouseDesignation()
    {
        if (Houses[HouseDesignation.None].Count <= 10)  // first x amount of houses should always be regular houses
        {
            return HouseDesignation.None;
        }
        else
        {
            var empty = Houses.Where(h => !h.Value.Any()).OrderBy(h => ParameterHandler[h.Key].CityFactorTargetValue);
            if (empty.Any())
            {
                return empty.First().Key;
            }
            else
            {
                return Houses.OrderByDescending(h => ParameterHandler[h.Key].CityFactorTargetValue / h.Value.Count)
                            .First().Key;
            }
        }
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

    internal void changeToTower(Point pos, TowerType selectedTower)
    {
        if (Grid.IsOutOfBounds(pos))
            return;

        //if (Grid[pos.X, pos.Y].Building.Template != wallTemplates.First()) //TODO: if is wall
        //    return;

        Grid.UpdateTile(pos, BuildingType.None, null);
        SpawnBuilding(pos, TowerTemplates[(int)selectedTower], BuildingType.Tower);
    }

    internal void SpawnWall(Point pos)
    {
        if (!Grid.IsEmpty(pos))
            DestroyBuilding(pos);
        SpawnBuilding(pos, WallTemplates.First(), BuildingType.Wall);
    }

    public IEnumerator convertHouseDesignations()
    {
        while (true)
        {
            //var nextHouseDesignation = DetermineNextHouseDesignation();
            //SpawnBuilding(Point pos, IEnumerable < GameObject > templates, BuildingType buildingType)

            yield return new WaitForSeconds(ConvertHouseDelay);
        }
    }

    private GameObject GetRandomHouse()
    {
        var houses = Buildings[BuildingType.House];
        return houses[UnityEngine.Random.Range(0, houses.Count)];
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

                SpawnBuilding(pos, BuildingTemplates, BuildingType.House);

            }

            yield return new WaitForSeconds(SpawnDelay);
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