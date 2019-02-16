#include <sstream>

#include "OneVsOneAIModule.h"

#include "BroodwarPrinter.h"
#include "LogWriter.h"

using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

// LOGGING
LogWriter gLog( "QLearning_v1.log" );
TableWriter gTable( "QLearning_v1_1Vs1.csv" );
TableWriter gEvaluationTable( "QLearning_v1_1Vs1_evaluate.csv" );
BroodwarPrinter gPrinter;

/*
1289478617; "bot LOST the game!"; 
1289478625; "bot LOST the game!"; 


*/

OneVsOneAIModule::OneVsOneAIModule()
:   DataProvider( 2, STATE_DATA_SIZE ),
    m_prevReward( 0 ),
	m_bIsWaitingForAttack( false ),
	m_bAttackAborted( false ),
	m_bWeaponRangeHack( false ),
	m_iterationCounter( 0 ),
	m_nrOfLearningIterations( 0 ),
	m_nrOfTestingIterations( 0 ),
	m_frameCounter( 0 ),
	m_bIsEvaluationRound( false )
{
	// Sleep for 5 seconds so we can hook in to debug it
	//Sleep( 5000 );

    m_pQLearning = new QLearning( this, "QLearning_v1_a", 0.95f, 0.2f, 0.05f );
    
    // If we want to test the performance of the trained network we have to set IsLearing to false!
    m_pQLearning->setIsLearning( true );

    m_pStateData = new float[STATE_DATA_SIZE];
}

OneVsOneAIModule::~OneVsOneAIModule()
{
    delete m_pStateData;
    delete m_pQLearning;
}

void OneVsOneAIModule::onStart()
{
	m_bGameOver = false;
	char buffer[512];
	gLog.setPrinter( &gPrinter );
	//gTable.setPrinter( &gPrinter );


	initFromFile( "QLearning_v1.ini" );

	int nr = m_iterationCounter % ( m_nrOfLearningIterations + m_nrOfTestingIterations );
	if( nr >= m_nrOfLearningIterations )
	{
		m_bIsEvaluationRound = true;
		m_pQLearning->setIsLearning( false );
		gEvaluationTable.setColumnHeaders( "is", "nrOfGames", "result" );
	}



	sprintf( buffer, "lSpeed: %d; isLearing: %s; dF: %.2f;"
		"e: %.2f; lR: %.2f; #: %d",
		s_localSpeed, 
		m_pQLearning->getIsLearning() ? "on" : "off",
		m_pQLearning->getDiscountFactor(),
		m_pQLearning->getExploreProbability(),
		m_pQLearning->getLearningRate(),
		m_iterationCounter );
	
	Broodwar->sendText( buffer );

	/**
	*	the following unit features should be collected
	*	Health (HitPoints)
	*	Attack
	*	Range
	*	Attack Cooldown
	*	Speed
	*	Armor
	*/

	if( s_bWriteTable )
	{
		gTable.setColumnHeaders( "fffffffffffff", "health", "damage", "range", 
			"currCooldown", "cooldown", "topSpeed", "health", "damage", "range", 
			"currCooldown", "cooldown", "topSpeed", "Distance"  ); 
	}

	//sprintf( buffer, "The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
	//gLog.addLine( buffer );
	
	// Enable some cheat flags
	Broodwar->enableFlag( Flag::UserInput );
	Broodwar->enableFlag( Flag::CompleteMapInformation );
	// speed increase
	Broodwar->setLocalSpeed( s_localSpeed );
	//Broodwar->setGUI( false );
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//read map information into BWTA so terrain analysis can be done in another thread
	BWTA::readMap();

	/*
	sprintf( buffer, "The match up is %s v %s",
		Broodwar->self()->getRace().getName().c_str(),
		Broodwar->enemy()->getRace().getName().c_str());
	gLog.addLine( buffer );
	*/


    // init own and enemy hit points
    m_ownHP = (* Broodwar->self()->getUnits().begin() )->getHitPoints();
    m_enemyHP = (* Broodwar->enemy()->getUnits().begin() )->getHitPoints();
	m_enemyMaxHP = m_enemyHP;
	m_ownMaxHP = m_ownHP;
}

void OneVsOneAIModule::initFromFile( const std::string & rFileName )
{
	char buffer[512];

	{
		// handler for configuration file
		std::fstream file;
		bool bStop = false;

		file.open( rFileName.c_str(), std::ios::in );
		if(!file.is_open())
		{
			gLog.addLine( "Error: loading config file!", true );
			return;
		}

		//file<<"\t\t testmessage" << m_iterationCounter;

		
		std::string temp = "";
		int value;	// buffor for values to be loaded
		int pos = 0;

		
		while(!file.eof() && ! bStop )
		{
			pos = file.tellg();
			file>>buffer;
			temp = buffer;
			file>>value;

			if(temp.compare("s_localSpeed")==0)
			{
				s_localSpeed = value;
			}
			if(temp.compare("is_learning")==0)
			{
				bool is_learning = value;
				m_pQLearning->setIsLearning(is_learning);
			}
			if(temp.compare("max_learning_iterations")==0)
				m_nrOfLearningIterations = value;
			if(temp.compare("max_testing_iterations")==0)
				m_nrOfTestingIterations = value;
			if(temp.compare("curr_iteration")==0)
			{
				m_iterationCounter = value;
			}
		}

		file.close();
	}

	{
		std::ofstream file;
		file.open( rFileName.c_str() );
		if(!file.is_open())
		{
			gLog.addLine( "Error: loading config file for writing!", true );
			return;
		}

		sprintf_s( buffer, 512, "s_localSpeed %d\n", s_localSpeed );
		file << buffer;

		sprintf_s( buffer, 512, "is_learning %d\n", m_pQLearning->getIsLearning() );
		file << buffer;

		sprintf_s( buffer, 512, "max_learning_iterations %d\n", m_nrOfLearningIterations );
		file << buffer;	

		sprintf_s( buffer, 512, "max_testing_iterations %d\n", m_nrOfTestingIterations );
		file << buffer;
		
		sprintf_s( buffer, 512, "curr_iteration %d", m_iterationCounter + 1 );
		file << buffer;

		file.flush();

		//delete buf;
		file.close();
	}
}

void OneVsOneAIModule::onEnd(bool isWinner)
{
	char buffer[512];

	// an ugly hack, but we have the problem that this is called 2 times if 
	// have won the previous match
	if(m_frameCounter>10)
	{
		char resultBuffer[256];
		if (isWinner)
		{
			//log win to file
			sprintf( resultBuffer, "WON" );
		
			// reward for wining depending on own hp points left
			//m_prevReward = 0.5+(0.5*(float)m_enemyHP/(float)m_enemyMaxHP);

			m_prevReward = +1.0;
		} 
		else 
		{
			//log win to file
			sprintf( resultBuffer, "LOST" );

			// punischment for loosing depending on enemy hp points left
			//m_prevReward = -0.5-(0.5*(float)m_enemyHP/(float)m_enemyMaxHP);

			m_prevReward = -1.0;
		}

		sprintf( buffer, " %4d, %5.2f; %5.2f; %5.2f; %s ",
			m_iterationCounter, 
			m_pQLearning->getDiscountFactor(),
			m_pQLearning->getExploreProbability(),
			m_pQLearning->getLearningRate(),
			resultBuffer );
	
		gLog.addLine( buffer, false );


		m_bGameOver = true;

		// do the last step of our QLearning - update the NN 
		m_pQLearning->setGameOver( m_bGameOver );
		m_pQLearning->step( );

		if( m_bIsEvaluationRound )
		{
			gEvaluationTable.addRow( false, m_iterationCounter, buffer );
		}
	}
}

void OneVsOneAIModule::onFrame()
{
	//m_pRLLogic->round();

    // check if the game is not over,
    // if we have an enemy 
    // or if we are still alive, otherwise we cannot do anything
	if( !m_bGameOver 
        && Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0
        && Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
	{
		
		
		if( !s_bWaitForAttack || !m_bIsWaitingForAttack )
		{
			int action = m_pQLearning->step( );
            //reward update ... just do it before we do a action
            //so no rewards can get lost!
            updateReward( );
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
				me->attackUnit( enemy );
				Broodwar->printf( "Enemy in range! ATTACK NOW!!" );
				m_bWeaponRangeHack = true;
				m_bIsWaitingForAttack = true;
			}

			if( me->isStartingAttack() )
			{
				m_bIsWaitingForAttack = false;
				m_bWeaponRangeHack = false;
			}	
		}
	} 
    else 
    {
		//char buffer[512];
        static int counter = 0;
        //Broodwar->printf( "tick[%d]", counter ++  );
		//sprintf( buffer, "tick[%d]", counter ++  );
		//gLog.addLine( buffer );
		Broodwar->printf( "... now we do nothing anymore[%d]", counter ++  );
    }

	m_frameCounter++;

	// debug output
	drawStats();
}

void OneVsOneAIModule::updateReward( )
{
    m_prevReward = 0;

    // init own and enemy hit points
    int newOwnHP = (* Broodwar->self()->getUnits().begin() )->getHitPoints();
    int newEnemyHP = (* Broodwar->enemy()->getUnits().begin() )->getHitPoints();

    // lost lives: PUNISH
    if( newOwnHP < m_ownHP ){
        m_prevReward = -0.01;
        Broodwar->printf( "PUNISH because we lost lives!" );
    }

    // enemy lost lives: REWARD!
    if( newEnemyHP < m_enemyHP )
    {
        m_prevReward = +0.1;
        Broodwar->printf( "REWARD because we hit the enemy!" );
    }

	//if(newOwnHP<newEnemyHP)
	//{
	//	m_prevReward = - 0.01;
	//	Broodwar->printf("PUNISH because we have less HP");
	//}
	//else
	//{
	//	m_prevReward = + 0.1;
	//	Broodwar->printf("REWARD because we have more or equal HP");
	//}

	//if(b_attackAborted)
	//{
 //       m_prevReward = -0.5;
 //       Broodwar->printf( "PUNISH because we aborted attack!" );
	//}

    // set new values
    m_enemyHP = newEnemyHP;
    m_ownHP = newOwnHP;

}

void OneVsOneAIModule::doAction( int action )
{
    //ToDo:
    // add withdraw and attack as actions

    // my and the enemy unit.
    Unit * me = (* Broodwar->self()->getUnits().begin() );
    Unit * enemy = (* Broodwar->enemy()->getUnits().begin() );

    BWAPI::Position myPos = me->getPosition();
    BWAPI::Position enemyPos = enemy->getPosition();

    // run to the enemy:
    //BWAPI::Position dir = enemyPos - myPos;

    // run from the enemy:
    BWAPI::Position dir = myPos - enemyPos;

    float length = dir.getLength() / 100; // we shouldn't normalize because then the values get too small for movement

	//static int lastAction = -1;	IS THIS CORRECT???
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
					Broodwar->printf( "withdrawing" );
				}

				m_bAttackAborted = false;
				if( m_lastAction == ATTACK && m_bIsWaitingForAttack)
				{
					m_bAttackAborted = true;
				}
				m_bIsWaitingForAttack = false;
		}
		break;
	case ATTACK:
		{
			if( action != m_lastAction)
			{ 
				bool canAttack = me->attackUnit( enemy );
				if( !canAttack )
				{
					//char buffer[512];
					//sprintf( buffer, "vector between 2 marines is: (%d,%d) length: %f", dir.x(), dir.y(), dir.getLength() );
					Broodwar->printf( "Cannot attack Enemy. Why ever?" );
				}

				m_bIsWaitingForAttack = true;
				//isWaitingForAttack = true;
				Broodwar->printf( "attacking" );
			}
		}
		break;
	}

	m_lastAction = action;

    //char buffer[512];
    //sprintf( buffer, "vector between 2 marines is: (%d,%d) length: %f", dir.x(), dir.y(), dir.getLength() );
	//Broodwar->printf( "%s", buffer );
    //gLog.addLine( buffer );
}

float OneVsOneAIModule::getReward() const
{
    return m_prevReward;
}


const float * OneVsOneAIModule::getCurrentState()
{
    // my and the enemy unit.
    Unit * me = (* Broodwar->self()->getUnits().begin() );
    Unit * enemy = (* Broodwar->enemy()->getUnits().begin() );

    fillWithUnitData( me, enemy, 0 );
    fillWithUnitData( enemy, me, STATE_DATA_PER_UNIT_SIZE );
   
    float * f = m_pStateData + 2*STATE_DATA_PER_UNIT_SIZE;

	//// counting the distance state input
	//UnitType type = me->getType();
	//WeaponType weapon = type.groundWeapon();
	//UnitType type_e = enemy->getType();
	//WeaponType weapon_e = type_e.groundWeapon();

	float distance = me->getDistance(enemy);
	//float distance_input = 0.0f;

	//if(distance>weapon.maxRange() && distance>weapon_e.maxRange())
	//	distance_input = 0.0f;
	//if(distance>weapon.maxRange() && distance<weapon_e.maxRange())
	//	distance_input = 0.16f;
	//if(distance>weapon.maxRange() && distance==weapon_e.maxRange())
	//{
	//	distance_input = 0.33f;
	//}
	//if(distance<weapon.maxRange() && distance<weapon_e.maxRange())
	//	distance_input = 0.5f;
	//if(distance==weapon.maxRange() && distance==weapon_e.maxRange())
	//{
	//	distance_input = 0.66f;
	//}
	//if(distance<weapon.maxRange() && distance>weapon_e.maxRange())
	//	distance_input = 0.83f;
	//if(distance==weapon.maxRange() && distance>weapon_e.maxRange())
	//{
	//	distance_input = 1.0f;
	//}
	//   
	//f[0] = distance_input;

	//// setting distance input (normalized based on experiments)
	f[0] = distance/500.0f;
	if(f[0]>1.0f)
		f[0] = 1.0f;

	// setting previous action input
	f[1] = m_lastAction == ATTACK ? 1.0f : 0.0f;
	


	if( s_bWriteTable )
	{
		gTable.addRow( false, m_pStateData );
	}

    return m_pStateData;
}

void OneVsOneAIModule::fillWithUnitData(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit, int offset)
{
	UnitType type = pUnit->getType();
	WeaponType weapon = type.groundWeapon();
	UnitType type_o = opponentUnit->getType();
	WeaponType weapon_o = type_o.groundWeapon();

    float * f = m_pStateData + offset;

	float temp_max = 0;		// temporary maximum value

	// setting health input
	f[0] = pUnit->getHitPoints()/(float)type.maxHitPoints();	// health

	// setting damage input
	if(weapon.damageAmount()>=weapon_o.damageAmount())
		temp_max = weapon.damageAmount();
	else
		temp_max = weapon_o.damageAmount();
	f[1] = weapon.damageAmount()/temp_max;	// attack amount

	// setting weapon range input
	if(weapon.maxRange()>=weapon_o.maxRange())
		temp_max = weapon.maxRange();
	else
		temp_max = weapon_o.maxRange();
	f[2] = weapon.maxRange()/temp_max;		// range

	// setting current cooldown time input
	f[3] = pUnit->getGroundWeaponCooldown()/(float)weapon.damageCooldown(); // current cooldown

	// setting maximum cooldawn time for weapon type input
	if(weapon.damageCooldown()>=weapon_o.damageCooldown())
		temp_max = weapon.damageCooldown();
	else
		temp_max = weapon_o.damageCooldown();
	f[4] = weapon.damageCooldown()/temp_max;	// cooldown time
	// damageCooldown is strange! :-/ ... cannot reproduce the relation between current cooldown and this.

	//// setting top speed input
	//if(type.topSpeed()>=type_o.topSpeed())
	//	temp_max = type.topSpeed();
	//else
	//	temp_max = type_o.topSpeed();
	//f[5] = type.topSpeed()/temp_max;			// top speed
}

void OneVsOneAIModule::drawStats()
{
	if( Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0  &&
		Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0 )
	{

		char buffer[512];
		Unit * me = (* Broodwar->self()->getUnits().begin() );
		Unit * enemy = * Broodwar->enemy()->getUnits().begin();
		
		sprintf( buffer, "%d%c", me->getHitPoints(), unitInWeaponRange( me, enemy ) ? 'r' : '-' );
		Broodwar->drawTextMap(me->getPosition().x(), me->getPosition().y(), "\x07%s", buffer);
		
		sprintf( buffer, "%d%c", enemy->getHitPoints(), unitInWeaponRange( enemy, me ) ? 'r' : '-' );
		Broodwar->drawTextMap(enemy->getPosition().x(), enemy->getPosition().y(), "\x06%s", buffer);
	}
}


//--------------------------------------------- IS IN WEAPON RANGE -----------------------------------------
bool OneVsOneAIModule::unitInWeaponRange( Unit * me, Unit *target ) const
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

	int distance = me->getDistance( target );
	return minRange < distance && maxRange >= distance;
}
