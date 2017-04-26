using System.Collections;
using System.Linq;
using UnityEngine;

public class CityBehaviorScript : MonoBehaviour {

    private Tile[,] Grid;
    public GameObject TownhallTemplate;
    public GameObject[] Buildings;

	// Use this for initialization
	void Start () {
        Grid = new Tile[100, 100];
        var x = Grid.GetLength(0) / 2;
        var y = Grid.GetLength(1) / 2;

        StartCoroutine(spawnStuff());
    }

    private void SpawnBuilding(int left, int top, GameObject template)
    {
        var building = Instantiate(template, new Vector3(left, 0, top), Quaternion.identity, gameObject.transform);
        var buildingTemplate = template.GetComponent<BuildingTemplate>();

        foreach (var x in Enumerable.Range(left, buildingTemplate.Width))
        {
            foreach (var y in Enumerable.Range(top, buildingTemplate.Height))
            {
                Grid[x, y] = new Tile();
            }
        }
    }
	
	// Update is called once per frame
	void Update () {
		
	}


    public IEnumerator spawnStuff()
    {
        for(int i = 0; i < 1000; i+=5)
        {
            SpawnBuilding(i, 0, TownhallTemplate);
            yield return new WaitForSeconds(1);
        }

    }

}
