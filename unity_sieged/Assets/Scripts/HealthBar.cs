using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class HealthBar : MonoBehaviour {

    public float Health = .5f;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        Vector3 target = Camera.main.transform.position;
        target.x = this.transform.position.x;

        this.transform.FindChild("Canvas").gameObject.SetActive(Health < 1);

        this.transform.LookAt(target);
        this.transform.FindChild("Canvas").FindChild("Green").GetComponent<RectTransform>().SetSizeWithCurrentAnchors(RectTransform.Axis.Horizontal, Health);
    }
}
