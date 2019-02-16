#include <sstream>
#include "RandomAIModule.h"
#include "BroodwarPrinter.h"
#include "LogWriter.h"
#include <math.h>
using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

// LOGGING
LogWriter gLog( "RandomPpayer.log" );
BroodwarPrinter gPrinter;
int lowestAction = 1000, biggestAction = -1;

RandomAIModule::RandomAIModule() : m_frameCounter( 0 )
{
	// Sleep for 5 seconds so we can hook in to debug it
	//Sleep( 5000 );

    m_pRandom = new RandomController();
}

RandomAIModule::~RandomAIModule()
{
    delete m_pRandom;
}

void RandomAIModule::onStart()
{
	m_bGameOver = false;
	char buffer[512];
	gLog.setPrinter( &gPrinter );
	
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	Broodwar->enableFlag(Flag::CompleteMapInformation);
	// speed increase
	Broodwar->setLocalSpeed( s_localSpeed );

	//read map information into BWTA so terrain analysis can be done in another thread
	BWTA::readMap();

	m_enemyMaxHP = (* Broodwar->enemy()->getUnits().begin() )->getType().maxHitPoints();
	m_ownMaxHP = (* Broodwar->self()->getUnits().begin() )->getType().maxHitPoints();

	m_ownTotalShotsToDeath = (float)GetNumberOfShotsLeftToDeath(
		(float)(* Broodwar->self()->getUnits().begin() )->getType().maxHitPoints(), 
		GetPotentialDamage( (* Broodwar->enemy()->getUnits().begin()), (* Broodwar->self()->getUnits().begin()) ) );
	m_enemyTotalShotsToDeath = (float)GetNumberOfShotsLeftToDeath(
		(float)(* Broodwar->enemy()->getUnits().begin() )->getType().maxHitPoints(), 
		GetPotentialDamage( (* Broodwar->self()->getUnits().begin()), (* Broodwar->enemy()->getUnits().begin()) ) );

	m_ownMaxCooldown = (* Broodwar->self()->getUnits().begin() )->getType().groundWeapon().damageCooldown();
	m_enemyMaxCooldown = (* Broodwar->enemy()->getUnits().begin() )->getType().groundWeapon().damageCooldown();
}

void RandomAIModule::onEnd(bool isWinner)
{
	char buffer[512];

	if(m_frameCounter>10)
	{
		//Broodwar->printf( "lowest=[%d], biggest=[%d]", lowestAction, biggestAction );
		if (isWinner)
		{
			//log win to file
			sprintf( buffer, "WON" );
		} 
		else 
		{
			//log win to file
			sprintf( buffer, "LOST" );
		}
		m_pRandom->setGameOver( true );
		gLog.addLine( buffer );

		m_bGameOver = true;
	}
}

void RandomAIModule::onFrame()
{
	//Sleep( 50 );
    // check if the game is not over,
    // if we have an enemy 
    // or if we are still alive, otherwise we cannot do anything
	if( !m_bGameOver 
        && Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0
        && Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
	{
		int action = m_pRandom->step( );
		doAction( action );
	}

	++ m_frameCounter;

	if(m_frameCounter>10000)
	{
		Broodwar->printf( "Played too long [%d], leaving game...", m_frameCounter );
		Broodwar->leaveGame();
	}

	// debug output
	drawStats();
}

void RandomAIModule::drawStats()
{
	if( Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0  &&
		Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0 )
	{
		char buffer[512];
		Unit * me = (* Broodwar->self()->getUnits().begin() );
		Unit * enemy = * Broodwar->enemy()->getUnits().begin();
		
		sprintf( buffer, "%d,%d", me->getGroundWeaponCooldown(), GetNumberOfShotsLeftToDeath(me->getHitPoints(), GetPotentialDamage(enemy, me)) );
		Broodwar->drawTextMap(me->getPosition().x(), me->getPosition().y(), "\x07%s", buffer);
		
		sprintf( buffer, "%d,%d", enemy->getGroundWeaponCooldown(), GetNumberOfShotsLeftToDeath(enemy->getHitPoints(), GetPotentialDamage(me, enemy)) );
		Broodwar->drawTextMap(enemy->getPosition().x(), enemy->getPosition().y(), "\x06%s", buffer);
	}
}

void RandomAIModule::doAction( int action )
{
    // my and the enemy unit.
    Unit * me = (* Broodwar->self()->getUnits().begin() );
    Unit * enemy = (* Broodwar->enemy()->getUnits().begin() );

    BWAPI::Position myPos = me->getPosition();
    BWAPI::Position enemyPos = enemy->getPosition();

    // run from the enemy:
    BWAPI::Position dir = myPos - enemyPos;

    float length = dir.getLength() / 100.0f; // we shouldn't normalize because then the values get too small for movement

    // move away from the enemy
	switch( action )
	{
	case WITHDRAW:
		{
			Position moveTo( myPos.x() + ((float)dir.x()) / length, myPos.y() + ((float)dir.y()) / length );
			moveTo.makeValid();
			me->move( moveTo );
			if( m_lastAction != action )
			{
				Broodwar->printf( "withdraw" );
			}
		}
		break;
	case ATTACK:
		{
			if(me->getOrder()!=Orders::AttackUnit)
				me->attackUnit(enemy);
			if( m_lastAction != action )
			{
				Broodwar->printf( "attack" );
			}
		}
		break;
	}

	m_lastAction = action;
}

int RandomAIModule::GetNumberOfShotsLeftToDeath(float hp, float damage)
{
	return (int)(ceil(hp/damage));
}

float RandomAIModule::GetPotentialDamage(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit)
{
	UnitType type = pUnit->getType();
	UnitType enemyType = opponentUnit->getType();
	float hisArmor = (float)enemyType.armor();

	float myAttack = (float)type.groundWeapon().damageAmount();

	float multiplier = 1.0;
	if(enemyType.size()==UnitSizeTypes::getUnitSizeType("Medium") && type.groundWeapon().damageType()==DamageTypes::getDamageType("Concussive"))
		multiplier = 0.5;
	if(enemyType.size()==UnitSizeTypes::getUnitSizeType("Large") && type.groundWeapon().damageType()==DamageTypes::getDamageType("Concussive"))
		multiplier = 0.25;
	if(enemyType.size()==UnitSizeTypes::getUnitSizeType("Small") && type.groundWeapon().damageType()==DamageTypes::getDamageType("Explosive"))
		multiplier = 0.5;
	if(enemyType.size()==UnitSizeTypes::getUnitSizeType("Medium") && type.groundWeapon().damageType()==DamageTypes::getDamageType("Explosive"))
		multiplier = 0.75;

	return (myAttack - hisArmor)*multiplier;
}