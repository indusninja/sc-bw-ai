#include <sstream>

#include "OneVsOneQlModule.h"

#include "BroodwarPrinter.h"
#include "LogWriter.h"

using namespace BWAPI;

bool analyzed;
bool analysis_just_finished;
BWTA::Region* home;
BWTA::Region* enemy_base;

// LOGGING
LogWriter	gLog(	"QLearning_v2.log" );
TableWriter gTable( "QLearning_v2_1Vs1.csv" );
TableWriter gEvaluationTable( "QLearning_v2_1Vs1_evaluate.csv" );
BroodwarPrinter gPrinter;

/*
1289478617; "bot LOST the game!"; 
1289478625; "bot LOST the game!"; 


*/

OneVsOneQlModule::OneVsOneQlModule()
:   DataProvider( 2, STATE_DATA_SIZE ),
    m_prevReward( 0 ),
	m_bIsWaitingForAttack( false ),
	m_bAttackAborted( false ),
	m_bWeaponRangeHack( false ),
	m_iterationCounter( 0 ),
	m_nrOfLearningIterations( 0 ),
	m_nrOfTestingIterations( 0 ),
	m_frameCounter( 0 ),
	m_bIsEvaluationRound( false ),
	m_lastRewardFrame( 0 )
{
	// Sleep for 5 seconds so we can hook in to debug it
	//Sleep( 5000 );

    m_pQLearning = new QLearning( this, "QLearning_v2_a", 0.9f, 0.05f, 0.01f );
    
    // If we want to test the performance of the trained network we have to set IsLearing to false!
    m_pQLearning->setIsLearning( true );

    m_pStateData = new float[STATE_DATA_SIZE];
}

OneVsOneQlModule::~OneVsOneQlModule()
{
    delete m_pStateData;
    delete m_pQLearning;
}

void OneVsOneQlModule::onStart()
{
	m_bGameOver = false;
	char buffer[512];
	gLog.setPrinter( &gPrinter );
	//gTable.setPrinter( &gPrinter );


	initFromFile( "QLearning_v2.ini" );

	int nr = m_iterationCounter % ( m_nrOfLearningIterations + m_nrOfTestingIterations );
	if( nr >= m_nrOfLearningIterations )
	{
		m_bIsEvaluationRound = true;
		m_pQLearning->setIsLearning( false );
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

	// intialize the tables
	{
		gTable.setColumnHeaders( "iffffffff", "frame", "health", "cooldown", "range", 
			"enemyHealth", "enemyCooldown", "enemyRange", "prevAction", "reward( NOT in State)" ); 
	

		gEvaluationTable.setColumnHeaders( "ifffsf", "nrOfGames", 
			"discountFactor",  "exploreProbability", "learningRate",
			"result", "score" );
	}
	//sprintf( buffer, "The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
	//gLog.addLine( buffer );
	
	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	Broodwar->enableFlag(Flag::CompleteMapInformation);
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
	m_ownHP = getCombinedUnitHealth(* Broodwar->self()->getUnits().begin() );
    m_enemyHP = getCombinedUnitHealth(* Broodwar->enemy()->getUnits().begin() );
	m_enemyMaxHP = m_enemyHP;
	m_ownMaxHP = m_ownHP;
}


void OneVsOneQlModule::initFromFile( const std::string & rFileName )
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

void OneVsOneQlModule::onEnd(bool isWinner)
{
	char buffer[512];
	float hp = 0.0f;

	// an ugly hack, but we have the problem that this is called 2 times if 
	// have won the previous match
	if( m_frameCounter>10 )
	{
		char resultBuffer[256];
		if (isWinner)
		{
			//log win to file
			sprintf( resultBuffer, "WON" );

			if( Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
			{
				BWAPI::Unit * pUnit = (* Broodwar->self()->getUnits().begin() );
				hp = (float)getCombinedUnitHealth( pUnit ) 
						/ (float)std::max<int>( m_enemyMaxHP, m_ownMaxHP ); // normalized
			}
			// reward for wining depending on own hp points left
			//m_prevReward = 0.5+(0.5*(float)m_enemyHP/(float)m_enemyMaxHP);

			m_prevReward = +1.0f;
		} 
		else 
		{
			//log win to file
			sprintf( resultBuffer, "LOST" );

			if( Broodwar->enemy() != 0 && Broodwar->enemy()->getUnits().size() > 0 )
			{
				BWAPI::Unit * pUnit = (* Broodwar->enemy()->getUnits().begin() );
				hp = - (float)getCombinedUnitHealth( pUnit ) 
						/ (float)std::max<int>( m_enemyMaxHP, m_ownMaxHP ); // normalized
			}
			// punischment for loosing depending on enemy hp points left
			//m_prevReward = -0.5-(0.5*(float)m_enemyHP/(float)m_enemyMaxHP);

			m_prevReward = -1.0f;
		}

		sprintf( buffer, " %4d, %5.2f; %5.2f; %5.2f; %s  %5.2f",
			m_iterationCounter, 
			m_pQLearning->getDiscountFactor(),
			m_pQLearning->getExploreProbability(),
			m_pQLearning->getLearningRate(),
			resultBuffer,
			hp );
	
		gLog.addLine( buffer, false );


		m_bGameOver = true;

		// do the last step of our QLearning - update the NN 
		m_pQLearning->setGameOver( m_bGameOver );
		m_pQLearning->step( );

		if( m_bIsEvaluationRound )
		{
			gEvaluationTable.addRow( false, 
				m_iterationCounter, 
				m_pQLearning->getDiscountFactor(),
				m_pQLearning->getExploreProbability(),
				m_pQLearning->getLearningRate(),
				resultBuffer,
				hp );
		}
	}
}

void OneVsOneQlModule::onFrame()
{
	//m_pRLLogic->round();

    // check if the game is not over,
    // if we have an enemy 
    // or if we are still alive, otherwise we cannot do anything
	if( !m_bGameOver 
        && Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0
        && Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
	{
		Unit * me = (* Broodwar->self()->getUnits().begin() );
		Unit * enemy = * Broodwar->enemy()->getUnits().begin();

		//bool bForceDoStep = false;
		if( s_bStopWaitWhenHit )
		{
			if( m_ownHP - getCombinedUnitHealth( me ) > 0 )
			{
				m_bIsWaitingForAttack = false;
				m_bIsMovingOutOfRange = false;

				Broodwar->printf( "We got hit! make a new DECISION!" );
			}
		}

		if( m_bIsMovingOutOfRange )
		{
			m_bIsMovingOutOfRange = unitInWeaponRange( enemy, me );
			if( !m_bIsMovingOutOfRange )
			{
				Broodwar->printf( "We are out of Range again!" );
			}
		}

		
		if( ( !s_bWaitForAttack || !m_bIsWaitingForAttack ) &&
			( !s_bWaitUntilOutOfRange || !m_bIsMovingOutOfRange ) )
		{
			int action = m_pQLearning->step( );
            //reward update ... just do it before we do a action
            //so no rewards can get lost!
            updateReward( );
			//perform action
			doAction( action );

			//Broodwar->printf( "STEP!" );
		}

		if( m_bIsWaitingForAttack )
		{
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
		Broodwar->printf( "... now we do nothing anymore[%d] frame:[%d]", counter ++ , m_frameCounter );
    }

	++ m_frameCounter;

	// debug output
	drawStats();
}

void OneVsOneQlModule::updateReward( )
{
    m_prevReward = 0;

    // init own and enemy hit points
    int newOwnHP = getCombinedUnitHealth(* Broodwar->self()->getUnits().begin() );
    int newEnemyHP = getCombinedUnitHealth(* Broodwar->enemy()->getUnits().begin() );

	/*
    // lost lives: PUNISH
    if( newOwnHP < m_ownHP ){
        m_prevReward += -0.5f;
        //Broodwar->printf( "PUNISH because we lost lives!" );
    }

    // enemy lost lives: REWARD!
    if( newEnemyHP < m_enemyHP )
    {
        m_prevReward += +0.01f;
       // Broodwar->printf( "REWARD because we hit the enemy!" );
    }

	/*
	if(b_attackAborted)
	{
        m_prevReward = -0.5;
        Broodwar->printf( "PUNISH because we aborted attack!" );
	}
	*/

	if( m_frameCounter - s_rewardInterval > m_lastRewardFrame ){
		BWAPI::Unit * me = (* Broodwar->self()->getUnits().begin() );
		BWAPI::Unit * enemy = (* Broodwar->enemy()->getUnits().begin() );

		m_prevReward = (float)( getCombinedUnitHealth( me ) 
			- getCombinedUnitHealth( enemy ) )
			/ (float)std::max<int>( m_enemyMaxHP, m_ownMaxHP ); 

		// set new values
		m_enemyHP = newEnemyHP;
		m_ownHP = newOwnHP;
	
		m_lastRewardFrame = m_frameCounter;

		Broodwar->printf( "REWARDING" );
	}

}

void OneVsOneQlModule::doAction( int action )
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

    float length = dir.getLength() / 100.0f; // we shouldn't normalize because then the values get too small for movement

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

				m_bIsMovingOutOfRange = false;
				if( unitInWeaponRange( enemy, me ) )
				{
					m_bIsMovingOutOfRange = true;
				}

				// we are not attacking anymore
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

float OneVsOneQlModule::getReward() const
{
    return m_prevReward;
}


const float * OneVsOneQlModule::getCurrentState()
{
    // my and the enemy unit.
    Unit * me = (* Broodwar->self()->getUnits().begin() );
    Unit * enemy = (* Broodwar->enemy()->getUnits().begin() );

    fillWithUnitData( me, enemy, 0 );
    fillWithUnitData( enemy, me, STATE_DATA_PER_UNIT_SIZE );
   
    float * f = m_pStateData + 2*STATE_DATA_PER_UNIT_SIZE;

	// setting previous action input
	f[0] = m_lastAction; // == ATTACK ? 0.0f : 1.0f;
	

	if( s_bWriteTable )
	{
		// ugly I know! :(
		gTable.addRow( false, m_frameCounter, m_pStateData[0], m_pStateData[1], m_pStateData[2],
			 m_pStateData[3], m_pStateData[4], m_pStateData[5],  m_pStateData[6], m_prevReward );
	}

    return m_pStateData;
}

void OneVsOneQlModule::fillWithUnitData(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit, int offset)
{
	UnitType type = pUnit->getType();
	WeaponType weapon = type.groundWeapon();
	UnitType enemyType = opponentUnit->getType();
	WeaponType enemyWeapon = enemyType.groundWeapon();

    float * f = m_pStateData + offset;

	float temp_max = 0;		// temporary maximum value

	// setting health input scaled by the max health 
	f[0] = (float)getCombinedUnitHealth( pUnit ) 
		/ (float)std::max<int>( m_enemyMaxHP, m_ownMaxHP ); // normalized

	// setting current cooldown time input
	f[1] = (float)pUnit->getGroundWeaponCooldown() 
		/ (float)weapon.damageCooldown(); // current cooldown normalized

	float distance = pUnit->getDistance( opponentUnit );
	float range = pUnit->getPlayer()->groundWeaponMaxRange( pUnit->getType() );

	// -1 to 0 is in range and 0 to 1 is out of range with distances further away mapped to 1
	f[2] = std::min<float>( 1.0, ( distance - range ) / ( 2 * range ) );

	
}

void OneVsOneQlModule::drawStats()
{
	if( Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0  &&
		Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0 )
	{

		char buffer[512];
		Unit * me = (* Broodwar->self()->getUnits().begin() );
		Unit * enemy = * Broodwar->enemy()->getUnits().begin();
		
		sprintf( buffer, "%d%c", getCombinedUnitHealth( me ), unitInWeaponRange( me, enemy ) ? 'r' : '-' );
		Broodwar->drawTextMap(me->getPosition().x(), me->getPosition().y(), "\x07%s", buffer);
		
		sprintf( buffer, "%d%c", getCombinedUnitHealth( enemy ), unitInWeaponRange( enemy, me ) ? 'r' : '-' );
		Broodwar->drawTextMap(enemy->getPosition().x(), enemy->getPosition().y(), "\x06%s", buffer);
	}
}


int OneVsOneQlModule::getCombinedUnitHealth( BWAPI::Unit * pUnit )
{
	return pUnit->getHitPoints() + pUnit->getShields();
}


//--------------------------------------------- IS IN WEAPON RANGE -----------------------------------------
bool OneVsOneQlModule::unitInWeaponRange( Unit * me, Unit *target ) const
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
