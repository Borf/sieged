using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

public class CityBehaviorScript : MonoBehaviour {

    private List<GameObject> Buildings;

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

    // Update is called once per frame
    void Update()
    {

    }

    public void SpawnBuilding(int left, int top, GameObject template, Builder builder)
    {
        if (!CanSpawn(left, top, template))
            return;

        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        var building = Instantiate(template, new Vector3(left + buildingTemplate.Width / 2.0f, 0, top + buildingTemplate.Height / 2.0f), Quaternion.identity, gameObject.transform);
        building.isStatic = true;

        Buildings.Add(building);

        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
            {
                Grid.UpdateTile(new Point(x, y), builder);
            }
        }
    }

    private bool CanSpawn(int left, int top, GameObject template)
    {
        var buildingTemplate = template.GetComponent<BuildingTemplate>();
        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
            {
                if (Grid.Tiles[x, y].HasBuilding)
                    return false;
            }
        }
        return true;
    }

    internal void changeToTower(Point pos)
    {
        if (Grid.IsOutOfBounds(pos))
            return;

        //if (Grid[pos.X, pos.Y].Building.Template != wallTemplates.First()) //TODO: if is wall
        //    return;

        Grid.UpdateTile(pos, Builder.None);
        SpawnBuilding(pos.X, pos.Y, TowerTemplates.First(), Builder.Player);
    }

    internal void SpawnWall(int x, int y)
    {
        SpawnBuilding(x, y, WallTemplates.First(), Builder.Player);
    }

    public IEnumerator spawnStuff()
    {
        while(true)
        {
            for (int i = 0; i < 50; i++)
            {
                var neighbors = Grid.GetEmptyNeighbors();

                if (!neighbors.Any())
                    break;

                var pos = neighbors[UnityEngine.Random.Range(0, neighbors.Count - 1)];
                SpawnBuilding(pos.X, pos.Y, BuildingTemplates.First(), Builder.Generated);
            }

            yield return new WaitForSeconds(0.001f);
        }
    }
}