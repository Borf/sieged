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
	float archeryAttackDelay = 5; // seconds
	int archeryDamage = 1;
	int archerHealth = 5;
	int archerTrainingCost = 10;

	float battleArenaFactor = 1;
	float knightAttackDelay = 1; // seconds
	int knightDamage = 1;
	int knightHealth = 5;
	float knightSpottingRange = 5;
	int knightTrainingCost = 10;

	float enemyAttackDelay = 1; // seconds
	int enemyDamage = 1;
	int enemyHealth = 5;
	int goldPerSecondPerMineralMine = 5;
	float goldInterest = 0.005f;
	
	int flagsPerBarracks = 2;


	float wallBuildSpeed = 1; //derived
	float archerStrength = 1; //derived
	float knightStrength = 1; //derived
};