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

    private HashSet<Point> buildPositions = new HashSet<Point>();
    private List<Point> offsets = new List<Point> { new Point(0, 1), new Point(0, -1), new Point(1, 0), new Point(-1, 0) };

    public Grid Grid { get; set; }

    // Use this for initialization
    void Start()
    {

        Grid = new Grid(100, 100);
        SpawnBuilding(Grid.Width / 2, Grid.Height / 2, TownhallTemplate, BuildingType.Townhall);

        StartCoroutine(spawnStuff());
    }

    // Update is called once per frame
    void Update()
    {

    }

    public void SpawnBuilding(int left, int top, GameObject template, BuildingType builder)
    {
        if (!CanSpawn(left, top, template))
            return;

        if (builder == BuildingType.House)
            Population++;

        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        var building = Instantiate(template, new Vector3(left + buildingTemplate.Width / 2.0f, 0, top + buildingTemplate.Height / 2.0f), Quaternion.identity, gameObject.transform);
        building.isStatic = true;

        //buildings have random rotation ;)
        if (BuildingTemplates.Contains(template))
            building.transform.Rotate(new Vector3(0, UnityEngine.Random.Range(0, 360), 0));

        //borfcode: is this more efficient?
        List<Point> newPoints = new List<Point>();
        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
            {
                Grid.UpdateTile(new Point(x, y), builder, building);
                newPoints.Add(new Point(x, y));
            }
        }

        HashSet<Point> newNeighbours = new HashSet<Point>();
        foreach (Point p in newPoints)
            foreach (Point offset in offsets)
                if (!Grid[p+offset].HasBuilding)
                    newNeighbours.Add(p + offset);
        buildPositions.RemoveWhere(p => newPoints.Contains(p));
        buildPositions.UnionWith(newNeighbours);
    }

    private bool CanSpawn(int left, int top, GameObject template)
    {
        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
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
        GameObject.Destroy(building);
    }

    internal void changeToTower(Point pos)
    {
        if (Grid.IsOutOfBounds(pos))
            return;

        //if (Grid[pos.X, pos.Y].Building.Template != wallTemplates.First()) //TODO: if is wall
        //    return;

        Grid.UpdateTile(pos, BuildingType.None, null);
        SpawnBuilding(pos.X, pos.Y, TowerTemplates.First(), BuildingType.Tower);
    }

    internal void SpawnWall(int x, int y)
    {
        SpawnBuilding(x, y, WallTemplates.First(), BuildingType.Wall);
    }

    public IEnumerator spawnStuff()
    {
        while (true)
        {
            for (int i = 0; i < 50; i++)
            {
                /*var neighbors = Grid.GetEmptyNeighbors();

                if (!neighbors.Any())
                    break;

                var pos = neighbors[UnityEngine.Random.Range(0, neighbors.Count - 1)];*/

                if (!buildPositions.Any())
                    break;

                var pos = buildPositions.ElementAt(UnityEngine.Random.Range(0, buildPositions.Count - 1));
                SpawnBuilding(pos.X, pos.Y, BuildingTemplates.First(), BuildingType.House);
            }

            yield return new WaitForSeconds(Delay);
        }
    }
}