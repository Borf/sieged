using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraScript : MonoBehaviour {

    public float PanSpeed = 5;


    private Vector3 prevPosition;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if (Input.GetKey("mouse 2"))
        {
            Vector3 diff = prevPosition - Input.mousePosition;
            if (diff.magnitude < 100)
            {
                transform.Translate(Vector3.right * (float)Time.deltaTime * PanSpeed * diff.x, Space.World);
                transform.Translate(Vector3.forward * (float)Time.deltaTime * PanSpeed * diff.y, Space.World);
            }
        }

        transform.Translate(Vector3.forward * (float)Time.deltaTime * PanSpeed * 100 * Input.GetAxis("Mouse ScrollWheel"), Space.Self);

        prevPosition = Input.mousePosition;
    }
}
