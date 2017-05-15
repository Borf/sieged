using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

public class PhotonBeamTower : TowerBase
{
    public GameObject Projectile;

    // Use this for initialization
    void Start()
    {
        StartCoroutine("Shoot");
    }

    // Update is called once per frame
    void Update()
    {

    }

    IEnumerator Shoot()
    {
        while (true)
        {
            while (Target == null)
            {
                yield return new WaitForSeconds(RetryFindTargetDelay);
                FindTarget();
            }

            GameObject newProjectile = GameObject.Instantiate(Projectile, transform.position + new Vector3(0, 3, 0), Quaternion.identity) as GameObject;
            newProjectile.transform.localScale = new Vector3(1, 1, 20);
            newProjectile.transform.Rotate(0, 0, 45, Space.World);

            yield return new WaitForSeconds(ShootDelay);
        }
    }
}