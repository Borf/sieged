﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EnemySpawner : MonoBehaviour {
    public float SpawnTime = 2;
    public GameObject SpawnMonster;

	// Use this for initialization
	void Start () {
        StartCoroutine("Spawn");
	}
	
	// Update is called once per frame
	void Update () {
		
	}


    public IEnumerator Spawn()
    {
        while(true)
        {
            float angle = Random.Range(0, Mathf.PI * 2);

            Vector3 position = new Vector3(250,0,250) + new Vector3(Mathf.Cos(angle), 0, Mathf.Sin(angle)) * 50.0f;

            GameObject newEnemy = Instantiate(SpawnMonster, position, Quaternion.identity) as GameObject;

            


            yield return new WaitForSeconds(SpawnTime);
        }
    }

}