using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;

public class EnemyAI : MonoBehaviour {
    public Vector3 target;
    private NavMeshAgent agent;

    // Use this for initialization
    void Start () {
        agent = GetComponent<NavMeshAgent>();
	}
	
	// Update is called once per frame
	void Update () {
        //transform.LookAt(target);
        //transform.position = Vector3.MoveTowards(transform.position, target, Time.deltaTime * 2);
        agent.SetDestination(target);

    }
}
