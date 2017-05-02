using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.AI;

public class EnemyAI : MonoBehaviour {
    public Vector3 target;
    private NavMeshAgent agent;
    public float MaxHealth = 100;
    float health;
    HealthBar healthbar;

    // Use this for initialization
    void Start () {
        agent = GetComponent<NavMeshAgent>();
        health = MaxHealth;
        healthbar = transform.FindChild("HealthBar").GetComponent<HealthBar>();
	}
	
	// Update is called once per frame
	void Update () {
        //transform.LookAt(target);
        //transform.position = Vector3.MoveTowards(transform.position, target, Time.deltaTime * 2);
        agent.SetDestination(target);

    }

    internal void Damage(float damage)
    {
        health -= damage;
        healthbar.Health = health / MaxHealth;
        if (health <= 0)
            GameObject.Destroy(gameObject);
    }
}
