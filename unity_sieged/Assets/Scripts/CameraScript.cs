using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraScript : MonoBehaviour {

    public float PanSpeed = 4;
    public float ScrollSpeed = 1000;
    
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
        else
        {
            var scrollWheel = Input.GetAxis("Mouse ScrollWheel");

            if (scrollWheel != 0)
            {
                var screenCenterFieldHit = CameraHelper.GetHit(CameraHelper.ScreenCenter);
                var distanceToField = (transform.position - screenCenterFieldHit).magnitude;

                var move = Mathf.Min((float)Time.deltaTime * ScrollSpeed * scrollWheel, transform.position.y - 5);
                transform.Translate(Vector3.forward * move, Space.Self);

                if (scrollWheel > 0)
                {
                    var newDistanceToField = (transform.position - screenCenterFieldHit).magnitude;
                    var scrollRatio = (distanceToField - newDistanceToField) / distanceToField;

                    var mousePos = CameraHelper.GetHitPositionMouse();
                    var centerPos = CameraHelper.GetHisPositionScreenCenter();
                    var fieldDistanceMouseToCenter = mousePos - centerPos;

                    transform.Translate(Vector3.right * fieldDistanceMouseToCenter.X * scrollRatio, Space.World);
                    transform.Translate(Vector3.forward * fieldDistanceMouseToCenter.Y * scrollRatio, Space.World);
                }
            }
        }

        prevPosition = Input.mousePosition;
    }
}
