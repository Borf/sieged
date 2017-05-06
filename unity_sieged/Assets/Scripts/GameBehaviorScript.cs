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

    /* public Text GrowthSliderText;
     public Text ConstructionSliderText;
     public Text ReligionSliderText;*/

    private CityBehaviorScript cityScript;

    private float GrowthValue { get { return Parameters[CityParameterType.Growth].TargetValue; } }
    private float ConstructionValue { get { return Parameters[CityParameterType.Construction].TargetValue; } }
    private float ReligionValue { get { return Parameters[CityParameterType.Religion].TargetValue; } }

    private float ActualGrowthValue { get { return Parameters[CityParameterType.Growth].ActualValue; } }
    private float ActualConstructionValue { get { return Parameters[CityParameterType.Construction].ActualValue; } }
    private float ActualReligionValue { get { return Parameters[CityParameterType.Religion].ActualValue; } }

    private Dictionary<CityParameterType, CityParameter> Parameters;

    private MouseMode MouseMode;

	// Use this for initialization
	void Start () {
        this.MouseMode = MouseMode.Nothing;
        cityScript = city.GetComponent<CityBehaviorScript>();

        // Init city parameters
        Parameters = new Dictionary<CityParameterType, CityParameter>();
        Parameters[CityParameterType.Construction] = new CityParameter() { slider = ConstructionSlider };
        Parameters[CityParameterType.Growth] = new CityParameter() { slider = GrowthSlider };
        Parameters[CityParameterType.Religion] = new CityParameter() { slider = ReligionSlider };

        StartCoroutine(UpdateActualValues());
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


    private IEnumerator UpdateActualValues()
    {
        foreach (var parameter in Parameters.Values)
            parameter.slider.GetComponent<SliderBehavior>().ActualValue = parameter.ActualValue;
        while (true)
        {
            foreach(var parameter in Parameters.Values)
            {
                if (parameter.ActualValue != parameter.TargetValue)
                {
                    parameter.ActualValue += 0.001f * Mathf.Sign(parameter.TargetValue - parameter.ActualValue);
                    parameter.ActualValue += (parameter.TargetValue - parameter.ActualValue) / 100.0f;
                    parameter.slider.GetComponent<SliderBehavior>().ActualValue = parameter.ActualValue;
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
        SliderChanged(newValue, Helper.ParseEnum<CityParameterType>(changedParameter));
    }

    public void SliderChanged(float newValue, CityParameterType changedParameter)
    {
        Parameters[changedParameter].TargetValue = newValue;
        var delta = Parameters.Sum(p => p.Value.TargetValue) - 1;
        var division = delta + 1 - newValue;
        //first calculate all new parameters
        foreach (var parameter in Parameters)
        {
            if(parameter.Key != changedParameter)
            {
                if (division > 0)
                    parameter.Value.TargetValue -= delta * parameter.Value.TargetValue / division;
                else
                    parameter.Value.TargetValue -= delta / (Parameters.Count - 1);
            }
        }
        //then send them to the UI
        foreach(var parameter in Parameters)
        {
            parameter.Value.slider.GetComponent<SliderBehavior>().Value = parameter.Value.TargetValue;
        }
    }


    public void GrowthSliderValueChanged(float newValue)
    {
        SliderChanged(newValue, CityParameterType.Growth);
    }

    public void ConstructionSliderValueChanged(float newValue)
    {
        SliderChanged(newValue, CityParameterType.Construction);
    }

    public void ReligionSliderValueChanged(float newValue)
    {
        SliderChanged(newValue, CityParameterType.Religion);
    }

    public void SetEditMode(string mode)
    {
        SetEditMode(Helper.ParseEnum<MouseMode>(mode));
    }

    public void SetEditMode(MouseMode mode)
    {
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