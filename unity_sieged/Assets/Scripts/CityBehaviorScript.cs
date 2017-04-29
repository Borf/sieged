using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class CityBehaviorScript : MonoBehaviour {

    private List<GameObject> Buildings;
    private int[,] Neighbors;
    public float delay = 0.5f;


    private List<Point> offsets = new List<Point> { new Point(0, 1), new Point(0, -1), new Point(1, 0), new Point(-1, 0) };

    public GameObject TownhallTemplate;
    public List<GameObject> BuildingTemplates;
    public List<GameObject> WallTemplates;
    public List<GameObject> TowerTemplates;

    public Grid Grid { get; set; }

    // Use this for initialization
    void Start () {

        Grid = new Grid(100, 100);

        Buildings = new List<GameObject>();

        SpawnBuilding(Grid.Width/2, Grid.Height/2, TownhallTemplate, Builder.Generated);

        StartCoroutine(spawnStuff());
    }

    public void SpawnBuilding(int left, int top, GameObject template, Builder builder)
    {
        if (!canSpawn(left, top, template))
            return;

        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        var building = Instantiate(template, new Vector3(left + buildingTemplate.Width / 2.0f, 0, top + buildingTemplate.Height / 2.0f), Quaternion.identity, gameObject.transform);
        building.isStatic = true;
        Buildings.Add(building);

        //buildings have random rotation ;)
        if (BuildingTemplates.Contains(template))
            building.transform.Rotate(new Vector3(0, UnityEngine.Random.Range(0, 360), 0));


        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
            {
                Grid[x, y].Builder = builder;
                Grid[x, y].Building = building;
            }
        }
    }

    private bool canSpawn(int left, int top, GameObject template)
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
        GameObject.Destroy(Grid[pos].Building);
        Grid[pos].Builder = Builder.None;
    }

    internal void changeToTower(Point pos)
    {
        if (Grid.IsOutOfBounds(pos))
            return;

        //if (Grid[pos.X, pos.Y].Building.Template != wallTemplates.First()) //TODO: if is wall
        //    return;

        Grid[pos].Builder = Builder.None;
        SpawnBuilding(pos.X, pos.Y, TowerTemplates.First(), Builder.Player);
    }

    internal void SpawnWall(int x, int y)
    {
        SpawnBuilding(x, y, WallTemplates.First(), Builder.Player);
    }

    // Update is called once per frame
    void Update () {
		
	}

    public IEnumerator spawnStuff()
    {
        while(true)
        {
            for (int i = 0; i < 1; i++)
            {
                var neighbors = GetEmptyNeighbors();
                var pos = neighbors[UnityEngine.Random.Range(0, neighbors.Count - 1)];

                SpawnBuilding(pos.X, pos.Y, BuildingTemplates.First(), Builder.Generated);
            }

            yield return new WaitForSeconds(delay);
        }
    }

    public List<Point> GetEmptyNeighbors()
    {
        var result = new List<Point>();

        for(int x = 0; x < Grid.Width; x++)
        {
            for (int y = 0; y < Grid.Height; y++)
            {
                var pos = new Point(x, y);
                if (!Grid[x, y].HasBuilding)
                {
                    bool hasNeighbor = false;
                    foreach (var offset in offsets)
                    {
                        var newPos = pos + offset;

                        if (Grid.IsOutOfBounds(newPos))
                            continue;

                        if (Grid[newPos.X, newPos.Y].HasBuilding)
                        {
                            hasNeighbor = true;
                            break;
                        }
                    }
                    if (hasNeighbor)
                        result.Add(pos);
                }
            }
        }
        return result;
    }
}
