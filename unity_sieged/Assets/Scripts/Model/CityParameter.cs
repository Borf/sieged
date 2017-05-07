using UnityEngine;

public class CityParameter
{
    public static float UICityNoneFactor = 0.8f;

    public HouseDesignation HouseDesignation;
    public float TargetValue = 1/3f;
    public float ActualValue = 1/3f;
    private GameObject slider;

    public GameObject Slider
    {
        get { return slider; }
        set {
            slider = value;
        }
    }

    private float CityFactor
    {
        get
        {
            if (HouseDesignation == HouseDesignation.None)
            {
                return UICityNoneFactor;
            }
            return 1 - UICityNoneFactor;
        }
    }

    public float CityFactorTargetValue
    {
        get
        {
            return TargetValue * CityFactor;
        }
    }

    public float CityFactorActualValue
    {
        get
        {
            return ActualValue * CityFactor;
        }
    }

}