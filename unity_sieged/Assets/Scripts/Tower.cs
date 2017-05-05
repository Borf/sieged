using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Tower : MonoBehaviour {

    public float ShootDelay = 1;
    public float range = 15;
    public GameObject projectile;

	// Use this for initialization
	void Start () {
        StartCoroutine("shoot");
	}
	
	// Update is called once per frame
	void Update () {
		
	}


    IEnumerator shoot()
    {
        while(true)
        {
            GameObject[] enemies = GameObject.FindGameObjectsWithTag("Enemy");
            foreach(GameObject enemy in enemies)
            {
                if((enemy.transform.position - transform.position).magnitude < range)
                {
                    GameObject newProjectile = GameObject.Instantiate(projectile, transform.position + new Vector3(0,3,0), Quaternion.identity) as GameObject;

                    newProjectile.GetComponent<BasicProjectile>().Target = enemy;

                    yield return new WaitForSeconds(ShootDelay);
                }
            }
            yield return new WaitForSeconds(0.01f);
        }
    }

}
