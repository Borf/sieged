using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

public class TowerBase : MonoBehaviour
{
    protected float RetryFindTargetDelay = 0.01f;
    protected GameObject Target;

    public float ShootDelay = 1;

    public float Range;

    public void FindTarget()
    {
        GameObject[] enemies = GameObject.FindGameObjectsWithTag("Enemy");
        foreach (GameObject enemy in enemies)
        {
            if ((enemy.transform.position - transform.position).sqrMagnitude < Range * Range)
            {
                Target = enemy;
                return;
            }
        }
    }
}