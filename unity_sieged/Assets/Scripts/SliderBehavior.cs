using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class SliderBehavior : MonoBehaviour {

    Slider slider;
    Text textValue;
    RectTransform actualValueTransform;

	// Use this for initialization
	void Start () {
        slider = transform.FindChild("Slider").GetComponent<Slider>();
        textValue = transform.FindChild("Value").GetComponent<Text>();
        actualValueTransform = transform.Find("Slider/Handle Slide Area/ActualHandle").GetComponent<RectTransform>();
	}
	
	// Update is called once per frame
	void Update () {
	}

    public float Value
    {
        set {
            if(Mathf.Abs(value - slider.value) > 1e-4f)
                slider.value = value;
            textValue.text = Mathf.Round(value * 100).ToString() + "%";
        }
    }
    public float ActualValue
    {
        set
        {
            float sliderHeight = slider.gameObject.GetComponent<RectTransform>().rect.height - actualValueTransform.rect.height;


            actualValueTransform.localPosition = new Vector3(0, (value - 0.5f) * sliderHeight, 0);
        }
    }

}
