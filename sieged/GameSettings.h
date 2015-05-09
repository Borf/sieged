#pragma  once


class GameSettings
{
public:
	float stoneMasonFactor = 1;
	float conveyorBuildingsPerSecond = 0.5f;
	float conveyorSpeed = 50;
	int startGold = 1000;

	float threadLevelFactor = 1;
	float threadLevelExponent = 2;

	float archeryRangeFactor = 1;
	float archeryAttackDelay = 1; // seconds
	int archeryDamage = 1;
	int archerHealth = 5;



	float wallBuildSpeed = 1; //derived
	float archerStrength = 1; //derived
};