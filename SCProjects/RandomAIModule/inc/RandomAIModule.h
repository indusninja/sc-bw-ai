#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
#include "RandomController.h"

extern bool analyzed;
extern bool analysis_just_finished;
extern BWTA::Region* home;
extern BWTA::Region* enemy_base;
DWORD WINAPI AnalyzeThread();

//PREDECLARATIONS
class BWAPI::Unit;

class RandomAIModule : public BWAPI::AIModule
{
private:
	bool				m_bGameOver;
    RandomController	*m_pRandom;

	int					m_lastAction;

	int					m_enemyMaxHP;
	int					m_ownMaxHP;
	float				m_ownTotalShotsToDeath;
	float				m_enemyTotalShotsToDeath;
	int					m_ownMaxCooldown;
	int					m_enemyMaxCooldown;

	// onFrame calls counter - allows to control the bug: always loosing after wining in order not to update q table
	int					m_frameCounter;

	/// CONSTANTS ///
	static const int	s_localSpeed = 0;

	enum EActions{ WITHDRAW, ATTACK };

public:
    RandomAIModule();
    ~RandomAIModule();
    virtual void onStart();
    virtual void onEnd( bool isWinner );
    virtual void onFrame();

    void doAction( int action );

	void drawStats();
	int GetNumberOfShotsLeftToDeath(float hp, float damage);
	float GetPotentialDamage(BWAPI::Unit* pUnit, BWAPI::Unit* opponentUnit);
};
