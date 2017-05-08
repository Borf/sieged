using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EnemySpawner : MonoBehaviour
{
    public float SpawnDelay = 1;
    public GameObject SpawnMonster;

    public List<Mob> Mobs;

    // Use this for initialization
    void Start() {
        StartCoroutine("Spawn");
    }

    // Update is called once per frame
    void Update() {

    }

    private GameObject ChooseMob()
    {
        if (Random.value < 0.8)
            return Mobs[0].GameObject;
        else
            return Mobs[1].GameObject;
    }

    public IEnumerator Spawn()
    {
        while(true)
        {
            float angle = Random.Range(0, Mathf.PI * 2);
            Vector3 position = this.transform.position + new Vector3(Mathf.Cos(angle), 0, Mathf.Sin(angle)) * 50.0f;

            GameObject newEnemy = Instantiate(ChooseMob(), position, Quaternion.identity, gameObject.transform) as GameObject;
            newEnemy.GetComponent<EnemyAI>().target = this.transform.position;

            yield return new WaitForSeconds(SpawnDelay);
        }
    }

}
