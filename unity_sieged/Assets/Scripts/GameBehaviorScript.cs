using System;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class GameBehaviorScript : MonoBehaviour {

    public int Money = 100;
    public Text MoneyLabel;
    public Text PopulationLabel;
    public GameObject cursorObject;
    public GameObject city;

    public Text GrowthSliderText;
    public Text ConstructionSliderText;
    public Text ReligionSliderText;

    private CityBehaviorScript cityScript;

    private float GrowthValue;
    private float ConstructionValue;
    private float ReligionValue;

    public GameObject GrowthSlider;
    public GameObject ConstructionSlider;
    public GameObject ReligionSlider;

    MouseMode MouseMode;

	// Use this for initialization
	void Start () {
        this.MouseMode = MouseMode.Nothing;
        cityScript = city.GetComponent<CityBehaviorScript>();
    }

    // Update is called once per frame
    void Update () {
        Money++;
        MoneyLabel.text = "Money: " + Money.ToString();
        PopulationLabel.text = "Population: " + cityScript.Population.ToString();

        if (MouseMode != MouseMode.Nothing && !EventSystem.current.IsPointerOverGameObject())
        {
            RaycastHit hit;
            Ray ray = Camera.main.GetComponent<Camera>().ScreenPointToRay(Input.mousePosition);

            if (Physics.Raycast(ray, out hit))
            {
                Transform objectHit = hit.transform;
                Point hitPos = new Point(Mathf.FloorToInt(hit.point.x), Mathf.FloorToInt(hit.point.z));
                cursorObject.transform.position = new Vector3(hitPos.X + 0.5f, 0.5f, hitPos.Y + 0.5f);
                if(Input.GetMouseButton(0))
                {
                    if (MouseMode == MouseMode.Walls)
                    {
                        if (Money >= 10)
                        {
                            Money -= 10;
                            cityScript.SpawnWall(hitPos);
                        }
                    }
                    else if (MouseMode == MouseMode.Destroy)
                    {
                        cityScript.DestroyBuilding(hitPos);
                    }
                }

                if (Input.GetMouseButtonDown(0))
                {
                    if (MouseMode == MouseMode.Tower)
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

    public void GrowthSliderValueChanged(float newValue)
    {
        GrowthValue = newValue;
        var delta = GrowthValue + ConstructionValue + ReligionValue - 1;
        ConstructionValue -= delta * ConstructionValue / (ConstructionValue + ReligionValue);
        ReligionValue -= delta * ReligionValue / (ConstructionValue + ReligionValue);
        Debug.Log(GrowthValue + " - " + ConstructionValue + " - " + ReligionValue);
        UpdateSliderTexts();

        ConstructionSlider.GetComponent<Slider>().value = ConstructionValue;
        ReligionSlider.GetComponent<Slider>().value = ReligionValue;
    }

    public void ConstructionSliderValueChanged(float newValue)
    {
        ConstructionValue = newValue;
        var delta = GrowthValue + ConstructionValue + ReligionValue - 1;
        GrowthValue -= delta / 2;
        ReligionValue -= delta / 2;
        UpdateSliderTexts();

        GrowthSlider.GetComponent<Slider>().value = GrowthValue;
        ReligionSlider.GetComponent<Slider>().value = ReligionValue;
    }

    public void ReligionSliderValueChanged(float newValue)
    {
        ReligionValue = newValue;
        var delta = GrowthValue + ConstructionValue + ReligionValue - 1;
        GrowthValue -= delta / 2;
        ConstructionValue -= delta / 2;
        UpdateSliderTexts();

        GrowthSlider.GetComponent<Slider>().value = GrowthValue;
        ConstructionSlider.GetComponent<Slider>().value = ConstructionValue;
    }

    public void UpdateSliderTexts()
    {
        GrowthSliderText.text = Math.Round(GrowthValue * 100).ToString() + "%";
        ConstructionSliderText.text = Math.Round(ConstructionValue * 100).ToString() + "%";
        ReligionSliderText.text = Math.Round(ReligionValue * 100).ToString() + "%";

    }

    public void SetEditMode(string strmode)
    {
        MouseMode mode = (MouseMode)MouseMode.Parse(typeof(MouseMode), strmode); //ewwww
        if (MouseMode == mode)
        {
            MouseMode = MouseMode.Nothing;
            cursorObject.SetActive(false);
        }
        else
        {
            MouseMode = mode;
            cursorObject.SetActive(true);
        }
    }
}

public enum MouseMode
{
    Nothing,
    Destroy,
    Walls,
    Tower,
};