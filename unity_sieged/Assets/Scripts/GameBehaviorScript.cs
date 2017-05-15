using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class GameBehaviorScript : MonoBehaviour {

    public int Money = 100;
    public Text MoneyLabel;
    public Text PopulationLabel;
    public GameObject cursorObject;
    public GameObject city;

    public GameObject GrowthSlider;
    public GameObject ConstructionSlider;
    public GameObject ReligionSlider;

    private CityBehaviorScript cityScript;

    private float GrowthValue { get { return ParameterHandler[HouseDesignation.Growth].TargetValue; } }
    private float ConstructionValue { get { return ParameterHandler[HouseDesignation.Construction].TargetValue; } }
    private float ReligionValue { get { return ParameterHandler[HouseDesignation.Religion].TargetValue; } }

    private float ActualGrowthValue { get { return ParameterHandler[HouseDesignation.Growth].ActualValue; } }
    private float ActualConstructionValue { get { return ParameterHandler[HouseDesignation.Construction].ActualValue; } }
    private float ActualReligionValue { get { return ParameterHandler[HouseDesignation.Religion].ActualValue; } }

    private TowerType SelectedTower;

    public CityParameterHandler ParameterHandler;

    private MouseMode MouseMode;

    void Awake ()
    {
        // Init city parameters
        ParameterHandler = new CityParameterHandler();
        ParameterHandler[HouseDesignation.None] = new CityParameter() { HouseDesignation = HouseDesignation.None, ActualValue = 1, TargetValue = 1 };
        ParameterHandler[HouseDesignation.Religion] = new CityParameter() { HouseDesignation = HouseDesignation.Religion, Slider = ReligionSlider };
        ParameterHandler[HouseDesignation.Construction] = new CityParameter() { HouseDesignation = HouseDesignation.Construction, Slider = ConstructionSlider };
        ParameterHandler[HouseDesignation.Growth] = new CityParameter() { HouseDesignation = HouseDesignation.Growth, Slider = GrowthSlider };
        ParameterHandler[HouseDesignation.Religion] = new CityParameter() { HouseDesignation = HouseDesignation.Religion, Slider = ReligionSlider };

        cityScript = city.GetComponent<CityBehaviorScript>();
        cityScript.ParameterHandler = ParameterHandler;
    }
	// Use this for initialization
	void Start () {
        this.MouseMode = MouseMode.Nothing;

        StartCoroutine(UpdateActualValues());
    }

    // Update is called once per frame
    void Update () {
        Money++;
        MoneyLabel.text = "Money: " + Money.ToString();
        PopulationLabel.text = "Population: " + cityScript.Population.ToString();

        if (MouseMode != MouseMode.Nothing && !EventSystem.current.IsPointerOverGameObject())
        {
            var hitPos = CameraHelper.GetHitPositionMouse();
            if (hitPos != null)
            {
                cursorObject.transform.position = new Vector3(hitPos.X + 0.5f, 0.5f, hitPos.Y + 0.5f);
                HandleMouseOnField(hitPos);
            }
        }
    }

    private void HandleMouseOnField(Point pos)
    {
        if (Input.GetMouseButton(0))
        {
            if (MouseMode == MouseMode.Walls)
            {
                if (Money >= 10)
                {
                    Money -= 10;
                    cityScript.SpawnWall(pos);
                }
            }
            else if (MouseMode == MouseMode.Destroy)
            {
                cityScript.DestroyBuilding(pos);
            }
        }

        if (Input.GetMouseButtonDown(0))
        {
            if (MouseMode == MouseMode.Tower)
            {
                if (Money >= 25)
                {
                    cityScript.changeToTower(pos, SelectedTower);
                    Money -= 25;
                }
            }
        }
    }

    private IEnumerator UpdateActualValues()
    {
        foreach (var kvp in ParameterHandler.GetSliderParameters())
        {
            var p = kvp.Value;
            p.Slider.GetComponent<SliderBehavior>().ActualValue = p.ActualValue;
        }

        while (true)
        {
            foreach(var kvp in ParameterHandler.GetSliderParameters())
            {
                var p = kvp.Value;
                if (p.ActualValue != p.TargetValue)
                {
                    p.ActualValue += 0.0005f * Mathf.Sign(p.TargetValue - p.ActualValue);
                    p.ActualValue += (p.TargetValue - p.ActualValue) / 2000.0f;
                    p.Slider.GetComponent<SliderBehavior>().ActualValue = p.ActualValue;
                }
            }

            yield return new WaitForSeconds(0.01f);
        }
    }

    /**
     * user Interface methods
     */

    public void SliderChanged(float newValue, string changedParameter)
    {
        SliderChanged(newValue, Helper.ParseEnum<HouseDesignation>(changedParameter));
    }

    public void SliderChanged(float newValue, HouseDesignation changedParameter)
    {
        ParameterHandler[changedParameter].TargetValue = newValue;
        var sliderParameters = ParameterHandler.GetSliderParameters();

        var delta = sliderParameters.Sum(p => p.Value.TargetValue) - 1;
        var division = delta + 1 - newValue;

        //first calculate all new parameters
        foreach (var parameter in sliderParameters)
        {
            if(parameter.Key != changedParameter)
            {
                if (division > 0)
                    parameter.Value.TargetValue -= delta * parameter.Value.TargetValue / division;
                else
                    parameter.Value.TargetValue -= delta / (ParameterHandler.Count - 1);
            }
        }
        //then send them to the UI
        foreach(var parameter in sliderParameters)
        {
            parameter.Value.Slider.GetComponent<SliderBehavior>().Value = parameter.Value.TargetValue;
        }
    }

    public void GrowthSliderValueChanged(float newValue)
    {
        SliderChanged(newValue, HouseDesignation.Growth);
    }

    public void ConstructionSliderValueChanged(float newValue)
    {
        SliderChanged(newValue, HouseDesignation.Construction);
    }

    public void ReligionSliderValueChanged(float newValue)
    {
        SliderChanged(newValue, HouseDesignation.Religion);
    }

    public void SetEditMode(string parameter)
    {
        var mode = "";

        if (parameter.Contains("_"))
        {
            mode = parameter.Split('_')[0];
            SelectedTower = (TowerType)int.Parse(parameter.Split('_')[1]);
        }
        else
        {
            mode = parameter;
        }

        SetEditMode(Helper.ParseEnum<MouseMode>(mode.ToString()));
    }

    public void SetEditMode(MouseMode mode)
    {
        /*if (MouseMode == mode)
        {
            MouseMode = MouseMode.Nothing;
            cursorObject.SetActive(false);
        }
        else*/
        {
            MouseMode = mode;
            cursorObject.SetActive(true);
        }
    }
}