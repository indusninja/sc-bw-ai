#pragma once
#include <BWAPI.h>

#include <BWTA.h>
#include <windows.h>

//PREDECLARATIONS
class BWAPI::Unit;
class BWAPI::Position;
class FuzzySystem;

class SquadAIModule : public BWAPI::AIModule
{
private:
    // enum
	enum EStance{ ATTACK, WITHDRAW };
	
	// onFrame calls counter - allows to control the bug: always loosing after wining in order not to update q table
	int                 m_frameCounter;
    bool                m_bGameOver;
    FuzzySystem  *      m_pFuzzySystem;


	/// Units Stats TYPEDEFS///
    typedef std::map<  BWAPI::Unit *, EStance> UnitActionMap;
    typedef std::pair< BWAPI::Unit *, EStance> UnitActionPair;

    typedef std::map<  BWAPI::Unit *, int> UnitIntMap;
    typedef std::pair< BWAPI::Unit *, int> UnitIntPair;
    
    typedef std::map<  BWAPI::Unit *, float> UnitFloatMap;
    typedef std::pair< BWAPI::Unit *, float> UnitFloatPair;


	typedef std::map<  BWAPI::Unit *, BWAPI::Unit *> UnitUnitMap;
    typedef std::pair< BWAPI::Unit *, BWAPI::Unit *> UnitUnitPair;

	/// Units Stats ///
	UnitActionMap       m_unitStances;
    UnitIntMap			m_unitHealths;
    UnitFloatMap        m_unitAveragedDeltaHealths;
	UnitUnitMap			m_unitAttackUnits;

	/// CONSTANTS ///
	static const bool	s_bWaitForAttack = true;
	static const bool	s_bWriteTable = false;
	static const int	s_localSpeed = 0;


	/// FOR FITNESS ///
	int					m_initialHitPoints;
	int					m_initialNrOfUnits;

    // PRIVATE FUNCTIONS
    void initFuzzySystem( );
    void initStates( );
    
	BWAPI::Unit * getWeakestEnemyUnit( );
	const BWAPI::Position & getCentroidEnemy( ) const;
	int     getStoredHealth( BWAPI::Unit * pUnit );
    float   getStoredAveragedDeltaHealth( BWAPI::Unit * pUnit );
	bool    isAttackingUnit( BWAPI::Unit * pUnit, BWAPI::Unit * pWeakEnemy ) const;
    
	void    updateStats( BWAPI::Unit * pUnit );
	EStance makeDecision( BWAPI::Unit * pUnit );
	void    doAction( BWAPI::Unit * pUnit, EStance action );

	int getCombinedUnitHealth( BWAPI::Unit * pUnit );
	void drawStats( BWAPI::Unit * pUnit );

public:
    SquadAIModule();
    ~SquadAIModule();
    virtual void onStart();
    virtual void onEnd( bool isWinner );
    virtual void onFrame();

    //virtual void onSendText(std::string text);
    //virtual void onReceiveText(BWAPI::Player* player, std::string text);
    //virtual void onPlayerLeft(BWAPI::Player* player);
    //virtual void onNukeDetect(BWAPI::Position target);
    //virtual void onUnitDiscover(BWAPI::Unit* unit);
    //virtual void onUnitEvade(BWAPI::Unit* unit);
    //virtual void onUnitShow(BWAPI::Unit* unit);
    //virtual void onUnitHide(BWAPI::Unit* unit);
    //virtual void onUnitCreate(BWAPI::Unit* unit);
    //virtual void onUnitDestroy(BWAPI::Unit* unit);
    //virtual void onUnitMorph(BWAPI::Unit* unit);
    //virtual void onUnitRenegade(BWAPI::Unit* unit);
    //virtual void onSaveGame(std::string gameName);
    //void drawStats(); //not part of BWAPI::AIModule
    void drawBullets();
    //void drawVisibilityData();
    //void drawTerrainData();
    //void showPlayers();
    //void showForces();
    //bool show_bullets;
    //bool show_visibility_data;

};
