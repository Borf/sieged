using UnityEngine;

public static class CameraHelper
{
    public static readonly Vector3 ScreenCenter = new Vector3(Screen.width / 2, Screen.height / 2);

    public static Point GetHitPositionMouse()
    {
        return GetHitPosition(Input.mousePosition);
    }

    public static Point GetHisPositionScreenCenter()
    {
        return GetHitPosition(ScreenCenter);
    }

    public static Point GetHitPosition(Vector3 screenPosition)
    {
        RaycastHit hit;
        Ray ray = Camera.main.GetComponent<Camera>().ScreenPointToRay(screenPosition);

        if (Physics.Raycast(ray, out hit))
        {
            Transform objectHit = hit.transform;
            return new Point(Mathf.FloorToInt(hit.point.x), Mathf.FloorToInt(hit.point.z));
        }

        return null;
    }

    public static Vector3 GetHit(Vector3 screenPosition)
    {
        RaycastHit hit;
        Ray ray = Camera.main.GetComponent<Camera>().ScreenPointToRay(screenPosition);

        if (Physics.Raycast(ray, out hit))
        {
            return hit.point;
        }

        return new Vector3(0,0,0);
    }
}