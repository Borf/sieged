using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class GameBehaviorScript : MonoBehaviour {

    public int Money = 100;
    public Text MoneyLabel;
    public GameObject cursorObject;
    public GameObject city;

    private CityBehaviorScript cityScript;
    private Camera camera;

    MouseMode MouseMode;


	// Use this for initialization
	void Start () {
        this.camera = GameObject.Find("Camera").GetComponent<Camera>();
        this.MouseMode = MouseMode.Nothing;
        cityScript = city.GetComponent<CityBehaviorScript>();
    }
	
	// Update is called once per frame
	void Update () {
        Money++;
        MoneyLabel.text = "Munney: " + Money.ToString();


        if (MouseMode != MouseMode.Nothing)
        {
            RaycastHit hit;
            Ray ray = this.camera.ScreenPointToRay(Input.mousePosition);

            if (Physics.Raycast(ray, out hit))
            {
                Transform objectHit = hit.transform;
                Point hitPos = new Point(Mathf.FloorToInt(hit.point.x), Mathf.FloorToInt(hit.point.z));
                cursorObject.transform.position = new Vector3(hitPos.X + 0.5f, 0.5f, hitPos.Y + 0.5f);
                if(Input.GetMouseButton(0))
                {

                    if (MouseMode == MouseMode.Walls)
                    {
                        if (cityScript.isEmpty(hitPos) && Money >= 10)
                        {
                            Money -= 10;
                            cityScript.SpawnWall(hitPos.X, hitPos.Y);
                        }
                    }
                    else if(MouseMode == MouseMode.Tower)
                    {
                        if (Money >= 25)
                        {
                            cityScript.changeToTower(hitPos);
                            Money -= 25;
                        }
                    }
                }
            }

        }

    }


    public void ClickWalls()
    {
        MouseMode = MouseMode.Walls;
        cursorObject.SetActive(true);
    }
    public void ClickTower()
    {
        MouseMode = MouseMode.Tower;
        cursorObject.SetActive(true);
    }


}

public enum MouseMode
{
    Nothing,
    Walls,
    Tower,
};