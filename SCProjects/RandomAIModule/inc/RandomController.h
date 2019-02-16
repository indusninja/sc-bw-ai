#pragma once
#include <string>

class RandomController
{
private:
	bool			m_bGameOver;
public:
	RandomController();
	virtual ~RandomController( void );
	
	int	step( );
	void setGameOver( bool isOver );
};
