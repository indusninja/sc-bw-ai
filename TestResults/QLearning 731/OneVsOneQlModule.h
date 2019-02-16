#pragma once
#include <BWAPI.h>

#include <BWTA.h>
#include <windows.h>
#include <fstream>

#include "QLearning.h"

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

//PREDECLARATIONS
class BWAPI::Unit;

class OneVsOneQlModule : public BWAPI::AIModule, public QLearning::DataProvider
{
private:
	bool        m_bGameOver;
    float       m_prevReward;
    QLearning * m_pQLearning;
    
    //saved State
	static const int STATE_DATA_PER_UNIT_SIZE = 3;
    static const int STATE_DATA_SIZE = STATE_DATA_PER_UNIT_SIZE * 2 + 1;
    
    float *		m_pStateData;

	int			m_lastAction;

	// onFrame calls counter - allows to control the bug: always loosing after wining in order not to update q table
	int			m_frameCounter;
	// speed setting, previously static
	int			s_localSpeed;

	bool		m_bIsEvaluationRound;

    // variables needed for the reward
    int         m_ownHP;
    int         m_enemyHP;
	int			m_enemyMaxHP;
	int			m_ownMaxHP;

	// actions flags
	bool		m_bIsWaitingForAttack;
	bool		m_bWeaponRangeHack;
	bool		m_bAttackAborted;
	bool		m_bIsMovingOutOfRange;

	// testing iterations
	int			m_nrOfLearningIterations;
	int			m_nrOfTestingIterations;
	int			m_iterationCounter; // number of iterations the game has already been played

	/// CONSTANTS ///
	static const bool	s_bWaitForAttack = true;
	static const bool	s_bWaitUntilOutOfRange = true;
	static const bool	s_bStopWaitWhenHit = true;

	static const int	s_rewardInterval = 10;
	int					m_lastRewardFrame;

	static const bool	s_bWriteTable = true;

	/// ENUM ///
	enum EActions{ ATTACK, WITHDRAW };


	/// FUNCTIONS ///
    // update reward. should be called after each decision
    void updateReward( );
	// pUnit is actual unit to concern, opponentUnit is an unit in opposition to actual unit
	void fillWithUnitData(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit, int offset=0);
	void initFromFile( const std::string & rFileName );

public:
    OneVsOneQlModule();
    ~OneVsOneQlModule();

	// checks if target is in Weapon Range of me. Implementation from BWAP (r3295)
	bool unitInWeaponRange( BWAPI::Unit * me, BWAPI::Unit *target ) const;
	void drawStats();
	int getCombinedUnitHealth( BWAPI::Unit * pUnit );


	// OverWritten functions from BWAPI::AIModule
    virtual void onStart();
    virtual void onEnd( bool isWinner );
    virtual void onFrame();
    /// creates state space data for a unit
    /// Attention you have to free the memory again.
    //float* createUnitData( BWAPI::Unit * pUnit );

    void doAction( int action );


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
    //void drawBullets();
    //void drawVisibilityData();
    //void drawTerrainData();
    //void showPlayers();
    //void showForces();
    //bool show_bullets;
    //bool show_visibility_data;

    /// MEMBERS from QLearning::DataProvider
    virtual float getReward() const;
    virtual const float * getCurrentState();

};
