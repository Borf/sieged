using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TarbuchetTower : TowerBase
{
    public GameObject Projectile;

	// Use this for initialization
	void Start () {
        StartCoroutine("Shoot");
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    IEnumerator Shoot()
    {
        while(true)
        {
            while(Target == null)
            {
                yield return new WaitForSeconds(RetryFindTargetDelay);
                FindTarget();
            }

            GameObject newProjectile = GameObject.Instantiate(Projectile, transform.position + new Vector3(0,3,0), Quaternion.identity) as GameObject;
            newProjectile.GetComponent<ParabolaProjectile>().TargetPosition = Target.transform.position;

            yield return new WaitForSeconds(ShootDelay);
        }
    }
}