﻿using System;
using System.Collections.Generic;
using UnityEngine;

public enum AttackType
{
    Melee,
    Ranged,
}

public class Wave
{
    public int StartAfterSecondsInGame;
    public List<MobSpawn> MobSpawns;
}

public class MobSpawn
{
    public int StartAfterSecondsWithinWave;
    public int SpawnWindowInSeconds;
    public int Amount;
    public Mob Mob;

    // Random single Location: SpawnSpread = 0, SpawnDirection = -1
    // Full spread: SpawnSpread 360, SpawnDirection = x
    public int SpawnSpread;
    public int SpawnDirection;
}

[Serializable]
public class Mob
{
    public string Name;
    public bool IsBoss;
    public GameObject GameObject;
    public List<Attack> Attacks;
}

[Serializable]
public class Attack
{
    public AttackType Type;
    public float Range;
    public bool Splash;
    public int Damage;
}