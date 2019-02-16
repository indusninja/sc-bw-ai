#include <sstream>
#include "OneVOneGAModule.h"
#include "BroodwarPrinter.h"
#include "LogWriter.h"
#include <math.h>
using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

// LOGGING
LogWriter gLog( "GA.log" );
TableWriter gTable( "GA_1Vs1.csv" );
BroodwarPrinter gPrinter;
int lowestAction = 1000, biggestAction = -1;

OneVOneGAModule::OneVOneGAModule()
:   DataProvider( 2, STATE_DATA_SIZE ),
	//m_bIsWaitingForAttack( false ),
	//m_bAttackAborted( false ),
	//m_bWeaponRangeHack( false ),
	m_frameCounter( 0 )
{
	// Sleep for 5 seconds so we can hook in to debug it
	//Sleep( 5000 );

    m_pGA = new GAController(this, 10, "GA_Population.csv");

    m_pStateData = new float[STATE_DATA_SIZE];
}

OneVOneGAModule::~OneVOneGAModule()
{
    delete [] m_pStateData;
    delete m_pGA;
}

void OneVOneGAModule::onStart()
{
	m_bGameOver = false;
	char buffer[512];
	gLog.setPrinter( &gPrinter );

	if( s_bWriteTable )
	{
		gTable.setColumnHeaders( "ffffffff", "MyShots", "MyCoolDown", "MyDistance", "MySpeed", 
			"EnemyShots", "EnemyCooldown", "EnemyDistance", "EnemySpeed"); 
	}
	
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

void OneVOneGAModule::onEnd(bool isWinner)
{
	char buffer[512];

	if(m_frameCounter>10)
	{
		//Broodwar->printf( "lowest=[%d], biggest=[%d]", lowestAction, biggestAction );
		if (isWinner)
		{
			//log win to file
			sprintf( buffer, "WON;%f", getFitness() );
			m_pGA->setGameOver( true, 1, true );
		} 
		else 
		{
			//log win to file
			sprintf( buffer, "LOST;%f", getFitness() );
			m_pGA->setGameOver( true, -1, false );
		}
		gLog.addLine( buffer );

		m_bGameOver = true;
	}
}

void OneVOneGAModule::onFrame()
{
	//Sleep( 50 );
    // check if the game is not over,
    // if we have an enemy 
    // or if we are still alive, otherwise we cannot do anything
	if( !m_bGameOver 
        && Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0
        && Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
	{
		/*if( !s_bWaitForAttack || !m_bIsWaitingForAttack )
		{
			int action = m_pGA->step( );
			//perform action
			doAction( action );
		}
		if( m_bIsWaitingForAttack )
		{
			Unit * me = (* Broodwar->self()->getUnits().begin() );
			Unit * enemy = * Broodwar->enemy()->getUnits().begin();

			if( !m_bWeaponRangeHack && 
				unitInWeaponRange( me, enemy ) )
			{
				//me->attackUnit( enemy );
				me->attackMove( enemy->getPosition() );
				//Broodwar->printf( "Enemy in range! ATTACK NOW!!" );
				m_bWeaponRangeHack = true;
				m_bIsWaitingForAttack = true;
			}

			if( me->isStartingAttack() )
			{
				m_bIsWaitingForAttack = false;
				m_bWeaponRangeHack = false;
			}	
		}*/
		int action = m_pGA->step( );
		if(action<lowestAction)
			lowestAction = action;
		if(action>biggestAction)
			biggestAction = action;
		doAction( action );
	} 
    else 
    {
        static int counter = 0;
     	Broodwar->printf( "... now we do nothing anymore[%d] frame:[%d]", counter ++ , m_frameCounter );
    }

	++ m_frameCounter;

	if(m_frameCounter>100000)
	{
		Broodwar->printf( "Played too long [%d], leaving game...", m_frameCounter );
		Broodwar->leaveGame();
	}

	// debug output
	drawStats();
}

void OneVOneGAModule::drawStats()
{
	if( Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0  &&
		Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0 )
	{
		char buffer[512];
		Unit * me = (* Broodwar->self()->getUnits().begin() );
		Unit * enemy = * Broodwar->enemy()->getUnits().begin();
		
		/*sprintf( buffer, "%d%c", me->getHitPoints(), unitInWeaponRange( me, enemy ) ? '+' : '-' );
		Broodwar->drawTextMap(me->getPosition().x(), me->getPosition().y(), "\x07%s", buffer);
		
		sprintf( buffer, "%d%c", enemy->getHitPoints(), unitInWeaponRange( enemy, me ) ? '+' : '-' );
		Broodwar->drawTextMap(enemy->getPosition().x(), enemy->getPosition().y(), "\x06%s", buffer);*/
		sprintf( buffer, "%d,%d", me->getGroundWeaponCooldown(), GetNumberOfShotsLeftToDeath(me->getHitPoints(), GetPotentialDamage(enemy, me)) );
		Broodwar->drawTextMap(me->getPosition().x(), me->getPosition().y(), "\x07%s", buffer);
		
		sprintf( buffer, "%d,%d", enemy->getGroundWeaponCooldown(), GetNumberOfShotsLeftToDeath(enemy->getHitPoints(), GetPotentialDamage(me, enemy)) );
		Broodwar->drawTextMap(enemy->getPosition().x(), enemy->getPosition().y(), "\x06%s", buffer);
	}
}

void OneVOneGAModule::doAction( int action )
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
	case FORWARD:
		{
				/*Position moveTo( myPos.x() - ((float)dir.x()) / length, myPos.y() - ((float)dir.y()) / length );
				moveTo.makeValid();
				me->move( moveTo );
				if( m_lastAction != action )
				{
					Broodwar->printf( "forward" );
				}*/
				me->move(enemy->getPosition());
				if( m_lastAction != action )
				{
					Broodwar->printf( "forward" );
				}

				/*m_bAttackAborted = false;
				if( m_lastAction == ATTACK && m_bIsWaitingForAttack)
				{
					m_bAttackAborted = true;
				}
				m_bIsWaitingForAttack = false;*/
		}
		break;
	case WITHDRAW:
		{
				Position moveTo( myPos.x() + ((float)dir.x()) / length, myPos.y() + ((float)dir.y()) / length );
				moveTo.makeValid();
				me->move( moveTo );
				if( m_lastAction != action )
				{
					Broodwar->printf( "backward" );
				}

				/*m_bAttackAborted = false;
				if( m_lastAction == ATTACK && m_bIsWaitingForAttack)
				{
					m_bAttackAborted = true;
				}
				m_bIsWaitingForAttack = false;*/
		}
		break;
	case ATTACK:
		{
			/*if( action != m_lastAction)
			{ 
				bool canAttack = me->attackUnit( enemy );
				if( !canAttack )
				{
					Broodwar->printf( "Cannot attack Enemy. Why ever?" );
				}

				m_bIsWaitingForAttack = true;
				Broodwar->printf( "attacking" );
			}*/
			//me->attackMove( enemy->getPosition() );
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

//--------------------------------------------- IS IN WEAPON RANGE -----------------------------------------
bool OneVOneGAModule::unitInWeaponRange( Unit * me, Unit *target ) const
{
	if ( !me || !target || !target->exists() || !me->exists() || me == target )
	  return false;

	UnitType thisType = me->getType();
	UnitType targType = target->getType();

	WeaponType wpn = me->getType().groundWeapon();
	int minRange = wpn.minRange();
	int maxRange = me->getPlayer()->groundWeaponMaxRange( me->getType() );
	if ( targType.isFlyer() || target->isLifted() )
	{
	  wpn = thisType.airWeapon();
	  minRange = wpn.minRange();
	  maxRange = me->getPlayer()->airWeaponMaxRange(thisType);
	}
	if ( wpn == WeaponTypes::None || wpn == WeaponTypes::Unknown )
	  return false;

	double distance = me->getDistance( target );
	return minRange < distance && maxRange >= distance;
}

float OneVOneGAModule::getFitness() const
{
	int newOwnHP = 0, newEnemyHP = 0;
	if(Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0)
		newEnemyHP = (* Broodwar->enemy()->getUnits().begin() )->getHitPoints();
    
	if(Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
		newOwnHP = (* Broodwar->self()->getUnits().begin() )->getHitPoints();
	
	float difference = 0.0;

	if(m_ownMaxHP>m_enemyMaxHP)
		difference = ((float)newOwnHP - (float)newEnemyHP)/(float)m_ownMaxHP;
	else
		difference = ((float)newOwnHP - (float)newEnemyHP)/(float)m_enemyMaxHP;

    return difference;
}

const float * OneVOneGAModule::getCurrentState()
{
    // my and the enemy unit.
    Unit * me = (* Broodwar->self()->getUnits().begin() );
    Unit * enemy = (* Broodwar->enemy()->getUnits().begin() );

    fillWithUnitData( me, enemy, 0 );
    fillWithUnitData( enemy, me, STATE_DATA_PER_UNIT_SIZE );
   
    //float * f = m_pStateData + 2*STATE_DATA_PER_UNIT_SIZE;
	//float * f = 2*STATE_DATA_PER_UNIT_SIZE;

	// setting previous action input
	//f[0] = m_lastAction == ATTACK ? 1.0f : 0.0f;

	if( s_bWriteTable )
	{
		// ugly I know! :(
		gTable.addRow( false, m_pStateData[0], m_pStateData[1], m_pStateData[2], m_pStateData[3], 
			 m_pStateData[4], m_pStateData[5], m_pStateData[6], m_pStateData[7] );
	}

	//Broodwar->printf( "My Shots = %f; His Shots = %f", m_pStateData[0], m_pStateData[3] );

    return m_pStateData;
}

void OneVOneGAModule::fillWithUnitData(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit, int offset)
{
	UnitType type = pUnit->getType();
	WeaponType weapon = type.groundWeapon();
	UnitType enemyType = opponentUnit->getType();
	WeaponType enemyWeapon = enemyType.groundWeapon();

    float * f = m_pStateData + offset;

	float temp_max = 0;		// temporary maximum value

	f[0] = (float)GetNumberOfShotsLeftToDeath((float)pUnit->getHitPoints(), GetPotentialDamage(opponentUnit, pUnit))
		/ std::max<float>( m_ownTotalShotsToDeath, m_enemyTotalShotsToDeath ); // normalized

	// setting current cooldown time input
	f[1] = (float)pUnit->getGroundWeaponCooldown() 
		/ (float)(std::max<int>( m_ownMaxCooldown, m_enemyMaxCooldown ) + 1); // current cooldown normalized

	float distance = pUnit->getDistance( opponentUnit );
	float range = pUnit->getPlayer()->groundWeaponMaxRange( pUnit->getType() );

	// -1 to 0 is in range and 0 to 1 is out of range with distances further away mapped to 1
	f[2] = std::min<float>( 1.0, ( distance - range ) / ( 2 * range ) );

	f[3] = (float)(type.topSpeed()/std::max<double>(type.topSpeed(), enemyType.topSpeed()));
}

int OneVOneGAModule::GetNumberOfShotsLeftToDeath(float hp, float damage)
{
	return (int)(ceil(hp/damage));
}

float OneVOneGAModule::GetPotentialDamage(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit)
{
	UnitType type = pUnit->getType();
	//WeaponType weapon = type.groundWeapon();
	UnitType enemyType = opponentUnit->getType();
	//WeaponType enemyWeapon = enemyType.groundWeapon();

	//int myArmor = type.armor();
	float hisArmor = (float)enemyType.armor();

	float myAttack = (float)type.groundWeapon().damageAmount();
	//int hisAttack = enemyType.groundWeapon().damageAmount();

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