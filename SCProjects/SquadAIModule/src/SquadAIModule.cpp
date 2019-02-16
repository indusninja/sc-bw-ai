#include <sstream>

#include "SquadAIModule.h"
#include "FuzzyIncludes.h"

#include "BroodwarPrinter.h"
#include "LogWriter.h"

using namespace BWAPI;

// LOGGING
LogWriter gLog( "FuzzyModule_v1.log" );
TableWriter gTable( "FuzzyModule.csv" );
BroodwarPrinter gPrinter;

/*
1289478617; "bot LOST the game!"; 
1289478625; "bot LOST the game!"; 
*/

SquadAIModule::SquadAIModule()
:   m_bGameOver( true ),
    m_frameCounter( 0 ),
    m_pFuzzySystem( 0 ),
	m_initialHitPoints( 0 ),
	m_initialNrOfUnits( 0 )
{
	// Sleep for 5 seconds so we can hook in to debug it
	//Sleep( 5000 );
}

SquadAIModule::~SquadAIModule()
{
    if( m_pFuzzySystem != 0 )
    {
        delete m_pFuzzySystem;
    }
}



void SquadAIModule::onStart()
{
	m_bGameOver = false;
	char buffer[512];
	gLog.setPrinter( &gPrinter );
	//gTable.setPrinter( &gPrinter );

	// === DO SOME BWAPI STUFF ===
    // Enable some cheat flags
	Broodwar->enableFlag( Flag::UserInput  );
	Broodwar->enableFlag( Flag::CompleteMapInformation );
	// speed increase
	Broodwar->setLocalSpeed( s_localSpeed );
	//Broodwar->setGUI( false );
	// Uncomment to enable complete map information
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	//read map information into BWTA so terrain analysis can be done in another thread
	//BWTA::readMap();

    // some initializations
    initFuzzySystem( );
    initStates( );

	gTable.setColumnHeaders( "iiiii", "initial HP", "initial nrOfUnits", "end HP", "end nrOfUnits", "nrOfFrames" );

}

void SquadAIModule::initFuzzySystem( )
{
    try
    {
        // create FuzzySystem
        m_pFuzzySystem = new FuzzySystem();

        // === linguistic variables and sets ===
        // INPUTS
        LinguisticVariable lvDeltaHealth( "DeltaHealth", 0.0, 1.0 );
        // |__+++!*****|
	    lvDeltaHealth.addFuzzySet( "High",	TrapezoidalFunction( 0.0001, 0.05, TrapezoidalFunction::LeftEdge ) );
    	
	    LinguisticVariable lvHealth( "Health", 0, 1.0 );
        // 
	    lvHealth.addFuzzySet( "Danger",		TrapezoidalFunction( 0.4, 0.5, TrapezoidalFunction::RightEdge ) );

        // OUTPUT
        LinguisticVariable lvStance ( "Stance", 0.0, 1.0 );
        // |****+!+____|
        lvStance.addFuzzySet( "Attack",	    TrapezoidalFunction( 0.4, 0.6, TrapezoidalFunction::RightEdge ) );
        // |____+!+****|
        lvStance.addFuzzySet( "Retreat",	TrapezoidalFunction( 0.4, 0.6, TrapezoidalFunction::LeftEdge ) );


        // addVariables
        m_pFuzzySystem->addVariable( lvDeltaHealth );
        m_pFuzzySystem->addVariable( lvHealth );
        m_pFuzzySystem->addVariable( lvStance );


        // RULES

		// IF DeltaHealth IS High AND Health IS Danger THEN Stance IS Retreat
        RuleStack rule1;
        rule1.add( Clause( "DeltaHealth", "High" ) );
        rule1.add( ANDOperator() );
        rule1.add( Clause( "Health", "Danger" ) );
		rule1.setThen( Clause( "Stance", "Retreat" ) );

		/*
		// IF DeltaHealth IS NOT High AND Health IS NOT Danger THEN Stance IS ATTACK
        RuleStack rule2;
        rule2.add( NotClause( "DeltaHealth", "High" ) );
        rule2.add( ANDOperator() );
        rule2.add( NotClause( "Health", "Danger" ) );
		rule2.setThen( Clause( "Stance", "Attack" ) );
		*/
		// IF Health IS NOT Danger THEN Stance IS ATTACK
        RuleStack rule2;
        rule2.add( NotClause( "DeltaHealth", "High" ) );
		rule2.setThen( Clause( "Stance", "Attack" ) );


		m_pFuzzySystem->newRule( "rule1", rule1 );
		m_pFuzzySystem->newRule( "rule2", rule2 );


		/*m_pFuzzySystem->setInput( "DeltaHealth", 0.3f );
		m_pFuzzySystem->setInput( "Health", 0.1f );
	

		float stance = m_pFuzzySystem->evaluate( "Stance" );
		Broodwar->printf( "stance-test: %f", stance );*/

    }
    catch( exception& e )
	{
		Broodwar->printf( "Exeption caught: %s", e.what() );
	}

	//Sleep( 1000 );
}

void SquadAIModule::initStates( )
{
    std::set< Unit * > myUnits = Broodwar->self()->getUnits();
    std::set< Unit * >::iterator myUnitsIter = myUnits.begin();

    while( myUnitsIter != myUnits.end() )
    {
        m_unitStances.insert( UnitActionPair( *myUnitsIter, WITHDRAW ) ); 
		updateStats( *myUnitsIter );


		++ m_initialNrOfUnits;
		m_initialHitPoints += getCombinedUnitHealth( *myUnitsIter );

        ++ myUnitsIter;
    }

	
}

void SquadAIModule::onEnd(bool isWinner)
{
	if( m_frameCounter < 10 )
	{
		// there was a bug in OneVsOne ... try to prevent this.
		return;
	}

	char buffer[512];

	int endNrOfUnits = 0;
	int endHitPoints = 0;

	std::set< Unit * > myUnits = Broodwar->self()->getUnits();
    std::set< Unit * >::iterator myUnitsIter = myUnits.begin();

	while( myUnitsIter != myUnits.end() )
    {
        m_unitStances.insert( UnitActionPair( *myUnitsIter, WITHDRAW ) ); 
		updateStats( *myUnitsIter );


		++ endNrOfUnits;
		endHitPoints += getCombinedUnitHealth( *myUnitsIter );

		// iterator
        ++ myUnitsIter;
    }

	gTable.addRow( false, m_initialHitPoints, m_initialNrOfUnits,
		endHitPoints, endNrOfUnits, m_frameCounter );


}

void SquadAIModule::onFrame()
{

    // check if game is not over and we and the enemy exist and still have units
    if( !m_bGameOver 
        && Broodwar->enemy()!= 0 && Broodwar->enemy()->getUnits().size() > 0
        && Broodwar->self() != 0 && Broodwar->self()->getUnits().size() > 0 )
	{
        std::set< Unit * > myUnits = Broodwar->self()->getUnits();
        std::set< Unit * >::iterator myUnitsIter = myUnits.begin();

        //Unit * pWeakEnemy = getWeakestUnit();


        
        while( myUnitsIter != myUnits.end() )
        {
            EStance action = ATTACK; //makeDecision( * myUnitsIter );
			doAction( *myUnitsIter, action );
			updateStats( *myUnitsIter );


			//DEBUG
			drawStats( *myUnitsIter );

            ++ myUnitsIter;
        }
        


    }

    // INCREASE the frame counter
	++ m_frameCounter;

    //drawBullets();
}


BWAPI::Unit * SquadAIModule::getWeakestEnemyUnit( )
{
    int minHealth = numeric_limits<int>::max();
    Unit * pWeakest = *Broodwar->enemy()->getUnits().begin();
    std::set< Unit * >::const_iterator iter = Broodwar->enemy()->getUnits().begin();

    while( iter != Broodwar->enemy()->getUnits().end() )
    {
        int health = getCombinedUnitHealth(*iter);
        if( minHealth > health )
        {
            minHealth = health;
            pWeakest = (* iter);
        }

		//DEBUG
		drawStats( * iter );

        ++ iter;
		
    }

    if( pWeakest != 0 )
    {

        Broodwar->drawTextMap( 
            pWeakest->getPosition().x(), 
            pWeakest->getPosition().y(), "\x06weakest" );
    }

    return pWeakest;
}

const BWAPI::Position & SquadAIModule::getCentroidEnemy( ) const
{
	int unitCounter = 0;
	BWAPI::Position sumPos;
	std::set< Unit * >::const_iterator iter = Broodwar->enemy()->getUnits().begin();

    while( iter != Broodwar->enemy()->getUnits().end() )
    {
		sumPos += (*iter)->getPosition();
        
		++ unitCounter;
        ++ iter;
    }

	if( unitCounter != 0 )
	{
		sumPos.x() /= unitCounter;
		sumPos.y() /= unitCounter;
	}

	return sumPos;
}


void SquadAIModule::updateStats( BWAPI::Unit * pUnit )
{

    /// UPDATE HEALTH
    float oldHealth = getCombinedUnitHealth( pUnit );
	if( m_unitHealths.find( pUnit ) != m_unitHealths.end() )
	{
        oldHealth = m_unitHealths.find( pUnit )->second;
		m_unitHealths.erase( m_unitHealths.find( pUnit ) );
	}
	// health
	m_unitHealths.insert( UnitIntPair( pUnit, getCombinedUnitHealth( pUnit ) ) );

    /// UPDATE DELTA HEALTH
    float oldDeltaH = 0.0f;
    if( m_unitAveragedDeltaHealths.find( pUnit ) != m_unitAveragedDeltaHealths.end() )
    {
       oldDeltaH = m_unitAveragedDeltaHealths.find( pUnit )->second;
       m_unitAveragedDeltaHealths.erase( pUnit );
    }
    // delta health is ( newDeltaHealt + oldDeltaHealt ) / 2
    m_unitAveragedDeltaHealths.insert( 
        UnitFloatPair( pUnit, 
        ( ( oldHealth - getCombinedUnitHealth( pUnit ) ) + oldDeltaH ) / 2.0f ) );

}


SquadAIModule::EStance SquadAIModule::makeDecision( BWAPI::Unit * pUnit )
{
	char buffer[512];

	int health = getCombinedUnitHealth( pUnit );
	float deltaHealth = getStoredAveragedDeltaHealth( pUnit );
	int maxHealth = pUnit->getType().maxHitPoints();

    float normalizedDeltaHealth = deltaHealth / (float) health;
    float normalizedHealth = health / (float) maxHealth;

	m_pFuzzySystem->setInput( "DeltaHealth", ( normalizedDeltaHealth ) );
	m_pFuzzySystem->setInput( "Health", normalizedHealth );
	


	float stance = -1.0; //m_pFuzzySystem->evaluate( "Stance" );

	try
	{
		stance = m_pFuzzySystem->evaluate( "Stance" );
	}
	catch( exception& e )
	{
		Broodwar->printf( "!" ); //"Exeption caught: %s", e.what() );
	}
	
	sprintf( buffer, "unit: %d, deltaHealth: %f (%.2f), health: %f (%.2f) - stance: %f", 
		pUnit->getID(),
		(float)deltaHealth, normalizedDeltaHealth,
		(float)health, normalizedHealth,
		stance );

	gLog.addLine( buffer, false );
	

	if( stance <= 0.5 )
	{
		return ATTACK;
	}
	else
	{
		return WITHDRAW;
	}
}


int SquadAIModule::getStoredHealth( BWAPI::Unit * pUnit )
{
	UnitIntMap::const_iterator iter = m_unitHealths.find( pUnit );
	if( iter != m_unitHealths.end() )
	{
		return iter->second;
	}

	return pUnit->getType().maxHitPoints();
}

float SquadAIModule::getStoredAveragedDeltaHealth( BWAPI::Unit * pUnit )
{
    UnitFloatMap::const_iterator iter = m_unitAveragedDeltaHealths.find( pUnit );
    if( iter != m_unitAveragedDeltaHealths.end() )
	{
		return iter->second;
	}

    return 0.0f;
}


void SquadAIModule::doAction( BWAPI::Unit * pUnit, SquadAIModule::EStance action )
{
	switch( action )
	{
	case WITHDRAW:
		{
			BWAPI::Position myPos = pUnit->getPosition();
			// run to the enemy:
				//BWAPI::Position dir = enemyPos - myPos;
			// run from the enemies:
			BWAPI::Position dir = myPos - getCentroidEnemy();
			
			float length = dir.getLength() / 100; // we shouldn't normalize because then the values get too small for movement
			
			Position moveTo( myPos.x() + ((float)dir.x()) / length, myPos.y() + ((float)dir.y()) / length );
			moveTo.makeValid();
			pUnit->move( moveTo );
			
			if( m_unitAttackUnits.find( pUnit ) != m_unitAttackUnits.end() )
			{
				m_unitAttackUnits.erase( m_unitAttackUnits.find( pUnit ) );
			}

			// DEBUG
			Broodwar->drawLineMap( myPos.x(), myPos.y(), 
				myPos.x() + dir.x(), myPos.y() + dir.y(), Colors::Green );
		}
		break;
	case ATTACK:
		{
			Unit * pWeakEnemy = getWeakestEnemyUnit();

			if( !isAttackingUnit( pUnit, pWeakEnemy ) )
			{
				pUnit->attackUnit( pWeakEnemy );
				if( m_unitAttackUnits.find( pUnit ) != m_unitAttackUnits.end() )
				{
					m_unitAttackUnits.erase( m_unitAttackUnits.find( pUnit ) );
				}
				m_unitAttackUnits.insert( UnitUnitPair( pUnit, pWeakEnemy ) );

				// DEBUG
				Broodwar->drawLineMap( pUnit->getPosition().x(), pUnit->getPosition().y(), 
					pWeakEnemy->getPosition().x(), pWeakEnemy->getPosition().y(), Colors::Red );
				}
			else
			{

				// DEBUG
				Broodwar->drawLineMap( pUnit->getPosition().x(), pUnit->getPosition().y(), 
					pWeakEnemy->getPosition().x(), pWeakEnemy->getPosition().y(), Colors::Blue );
			}
		}
		break;
	}

}

bool SquadAIModule::isAttackingUnit( BWAPI::Unit * pUnit, BWAPI::Unit * pWeakEnemy ) const
{
	UnitUnitMap::const_iterator iter = m_unitAttackUnits.find( pUnit );
	if( iter != m_unitAttackUnits.end() )
	{
		return iter->second->getID() == pWeakEnemy->getID();
	}

	return false;
}

int SquadAIModule::getCombinedUnitHealth( BWAPI::Unit * pUnit )
{
	return pUnit->getHitPoints() + pUnit->getShields();
}

void SquadAIModule::drawStats( BWAPI::Unit * pUnit )
{
	if( pUnit->exists() )
	{

		char buffer[512];
		
		sprintf( buffer, "%d", getCombinedUnitHealth( pUnit ) );
		Broodwar->drawTextMap( pUnit->getPosition().x(), pUnit->getPosition().y(), "\x07%s", buffer);
		
	
	}
}



void SquadAIModule::drawBullets()
{
  std::set<Bullet*> bullets = Broodwar->getBullets();
  for(std::set<Bullet*>::iterator i=bullets.begin();i!=bullets.end();i++)
  {
    Position p=(*i)->getPosition();
    double velocityX = (*i)->getVelocityX();
    double velocityY = (*i)->getVelocityY();
    if ((*i)->getPlayer()==Broodwar->self())
    {
      Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Green);
      Broodwar->drawTextMap(p.x(),p.y(),"\x07%s",(*i)->getType().getName().c_str());
    }
    else
    {
      Broodwar->drawLineMap(p.x(),p.y(),p.x()+(int)velocityX,p.y()+(int)velocityY,Colors::Red);
      Broodwar->drawTextMap(p.x(),p.y(),"\x06%s",(*i)->getType().getName().c_str());
    }
  }
}