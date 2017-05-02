using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BasicProjectile : MonoBehaviour {

    public GameObject Target;
    public float Speed = 5;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		if(Target != null)
        {
            transform.position = Vector3.MoveTowards(transform.position, Target.transform.position, Time.deltaTime * Speed);
            if((transform.position - Target.transform.position).magnitude < 0.1)
            {
                Target.GetComponent<EnemyAI>().Damage(10);
                GameObject.Destroy(gameObject);
            }
        }
	}
}
