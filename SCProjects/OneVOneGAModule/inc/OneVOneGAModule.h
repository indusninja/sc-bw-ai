#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "GAController.h"

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

//PREDECLARATIONS
class BWAPI::Unit;

class OneVOneGAModule : public BWAPI::AIModule, public DataProvider
{
private:
	bool			m_bGameOver;
    GAController	*m_pGA;

    //saved State
	static const int STATE_DATA_PER_UNIT_SIZE = 4;
    //static const int STATE_DATA_SIZE = STATE_DATA_PER_UNIT_SIZE * 2 + 1;
	static const int STATE_DATA_SIZE = STATE_DATA_PER_UNIT_SIZE * 2;

    float			*m_pStateData;

	int				m_lastAction;

	int				m_enemyMaxHP;
	int				m_ownMaxHP;
	float			m_ownTotalShotsToDeath;
	float			m_enemyTotalShotsToDeath;
	int				m_ownMaxCooldown;
	int				m_enemyMaxCooldown;

	// onFrame calls counter - allows to control the bug: always loosing after wining in order not to update q table
	int			m_frameCounter;

	// actions flags
	/*bool		m_bIsWaitingForAttack;
	bool		m_bWeaponRangeHack;
	bool		m_bAttackAborted;*/

	/// CONSTANTS ///
	/*static const bool	s_bWaitForAttack = true;*/
	static const bool	s_bWriteTable = false;
	static const int	s_localSpeed = 0;

	// pUnit is actual unit to concern, opponentUnit is an unit in opposition to actual unit
	void fillWithUnitData(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit, int offset=0);

	enum EActions{ FORWARD, WITHDRAW, ATTACK };

public:
    OneVOneGAModule();
    ~OneVOneGAModule();
    virtual void onStart();
    virtual void onEnd( bool isWinner );
    virtual void onFrame();

    void doAction( int action );

	bool unitInWeaponRange( BWAPI::Unit * me, BWAPI::Unit *target ) const;
	void drawStats();
	int GetNumberOfShotsLeftToDeath(float hp, float damage);
	float GetPotentialDamage(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit);

    /// MEMBERS from GAController::DataProvider
    virtual float getFitness() const;
    virtual const float * getCurrentState();
};
