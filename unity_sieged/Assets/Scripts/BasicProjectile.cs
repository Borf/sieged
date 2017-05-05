using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BasicProjectile : MonoBehaviour {

    public GameObject Target;
    private Vector3 TargetPosition;
    public float Speed = 5;
    public float damage = 0.1f;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if (Target)
            TargetPosition = Target.transform.position;

        transform.position = Vector3.MoveTowards(transform.position, TargetPosition, Time.deltaTime * Speed);
        transform.LookAt(TargetPosition);
        if ((transform.position - TargetPosition).magnitude < 0.1)
        {
            GameObject.Destroy(gameObject);
            if(Target)
                Target.GetComponent<EnemyAI>().Damage(damage);

        }

    }
}
