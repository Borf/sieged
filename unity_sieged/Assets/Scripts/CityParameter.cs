using UnityEngine;

public class CityParameter
{
    public float TargetValue = 1/3f;
    public float ActualValue = 1/3f;
    public GameObject slider;
}

public enum CityParameterType
{
    Growth,
    Construction,
    Religion
}