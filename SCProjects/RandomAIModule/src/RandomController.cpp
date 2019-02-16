#include <string>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <sstream>

#include "RandomController.h"
#include "Chromosome.h"

RandomController::RandomController() : m_bGameOver(false)
{
	// we use the randomNumberGenerator so seed first! ;-)
	srand ( clock() );
}

RandomController::~RandomController(void)
{
}

int	RandomController::step( )
{
	float random = getRandom(0.0, 1.0);

	if(random<0.5)
		return 0;
	else
		return 1;
}

void RandomController::setGameOver( bool isOver)
{ 
	m_bGameOver = isOver;
}