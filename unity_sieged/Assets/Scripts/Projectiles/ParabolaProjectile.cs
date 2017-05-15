using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ParabolaProjectile : MonoBehaviour
{
    public Vector3 TargetPosition;
    public float Speed = 5;
    public float damage = 0.1f;
    public float Gravity = 2;
    private Vector3 Direction;

    private float TicksRequired;

    // Use this for initialization
    void Start()
    {
        //GameObject newProjectile = GameObject.Instantiate(this.gameObject, TargetPosition, Quaternion.identity) as GameObject;
        //newProjectile.GetComponent<ParabolaProjectile>().TargetPosition = TargetPosition;

        var diff = TargetPosition - transform.position - new Vector3(0.0f, 0.1f, 0.0f);


        if (diff.sqrMagnitude > 0)
        {
            var magnitudeXZ = Mathf.Sqrt(diff.x * diff.x + diff.z * diff.z);

            TicksRequired = magnitudeXZ;
            Direction = diff / magnitudeXZ;
            Direction.y = Mathf.Min(Gravity * TicksRequired / (2 * Speed), 1);
        }
        else
        {
            Direction = new Vector3(0, 0, 0);
            TicksRequired = 0;
        }
    }

    private Vector3 NormalizeOverXandZ(Vector3 vector)
    {
        var magnitude = Mathf.Sqrt(vector.x * vector.x + vector.z + vector.z);
        return new Vector3(vector.x / magnitude, vector.y / magnitude, vector.z / magnitude);
    }

    // Update is called once per frame
    void Update()
    {
        transform.position = Vector3.MoveTowards(transform.position, transform.position + 10 * Direction, Time.deltaTime * Speed);

        Direction.y = Direction.y - Time.deltaTime * Gravity;

        transform.LookAt(TargetPosition);

        if (transform.position.y < 0.1)
        {
            GameObject.Destroy(gameObject);
        }
    }
}