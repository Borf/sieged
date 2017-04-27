using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class CityBehaviorScript : MonoBehaviour {

    private Tile[,] Grid;
    private List<GameObject> Buildings;

    private List<Point> offsets = new List<Point> { new Point(0, 1), new Point(0, -1), new Point(1, 0), new Point(-1, 0) };

    public GameObject TownhallTemplate;
    public List<GameObject> BuildingTemplates;
    public List<GameObject> WallTemplates;
    public List<GameObject> TowerTemplates;


    // Use this for initialization
    void Start () {

        Grid = new Tile[100, 100];
        Buildings = new List<GameObject>();

        var x = Grid.GetLength(0) / 2;
        var y = Grid.GetLength(1) / 2;
        SpawnBuilding(x, y, TownhallTemplate);

        StartCoroutine(spawnStuff());
    }

    public void SpawnBuilding(int left, int top, GameObject template)
    {
        if (!canSpawn(left, top, template))
            return;

        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        var building = Instantiate(template, new Vector3(left + buildingTemplate.Width / 2.0f, 0, top + buildingTemplate.Height / 2.0f), Quaternion.identity, gameObject.transform);
        building.isStatic = true;

        Buildings.Add(building);

        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
            {
                Grid[x, y] = new Tile();
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
                if (Grid[x, y] != null)
                    return false;
            }
        }
        return true;
    }

    internal void changeToTower(Point pos)
    {
        if (pos.X < 0 || pos.X >= Grid.GetLength(0) ||
            pos.Y < 0 || pos.Y >= Grid.GetLength(1))
            return;
        //if (Grid[pos.X, pos.Y].Building.Template != wallTemplates.First()) //TODO: if is wall
        //    return;


        Grid[pos.X, pos.Y] = null;

        SpawnBuilding(pos.X, pos.Y, TowerTemplates.First());

        


    }

    internal bool isEmpty(Point pos)
    {
        if (pos.X < 0 || pos.X >= Grid.GetLength(0) ||
            pos.Y < 0 || pos.Y >= Grid.GetLength(1))
            return false;
        return Grid[pos.X, pos.Y] == null;
    }

    internal void SpawnWall(int x, int y)
    {
        SpawnBuilding(x, y, WallTemplates.First());
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

                SpawnBuilding(pos.X, pos.Y, BuildingTemplates.First());
            }

            yield return new WaitForSeconds(1);
        }
    }

    public List<Point> GetEmptyNeighbors()
    {
        var result = new List<Point>();

        for(int x = 0; x < Grid.GetLength(0); x++)
        {
            for (int y = 0; y < Grid.GetLength(1); y++)
            {
                var pos = new Point(x, y);
                if (Grid[x, y] == null)
                {
                    bool hasNeighbor = false;
                    foreach (var offset in offsets)
                    {
                        var newPos = pos + offset;

                        if (newPos.X < 0 || newPos.X >= Grid.GetLength(0) ||
                            newPos.Y < 0 || newPos.Y >= Grid.GetLength(1))
                            continue;

                        if (Grid[newPos.X, newPos.Y] != null)
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
