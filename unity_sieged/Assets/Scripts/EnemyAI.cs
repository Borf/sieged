using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EnemyAI : MonoBehaviour {

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        transform.LookAt(new Vector3(250, 0, 250));
        transform.position = Vector3.MoveTowards(transform.position, new Vector3(250, 0, 250), Time.deltaTime * 2);

    }
}
