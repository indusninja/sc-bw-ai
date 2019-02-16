#define TEST_COUNT 10
#pragma once
#include <string>
#include "LogWriter.h"
#include "DataProvider.h"

/// PREDECLARATIONS ////////////////////////
class NeuralNetwork;
class Population;

class GAController
{
private:
	DataProvider	*m_pGame;
	bool			m_bGameOver;
	int             m_nrOfActions;
	int				m_nrOfStates;
	int				m_nTestCount;

	Population		*m_pPop;
	int				m_nChromosomeSize;

    // Neural Network stuff
    NeuralNetwork*	m_pNetwork;
    int             m_nrOfNNLayers;
    int*			m_pNeuronsPerLayer;
	std::string		m_networkFiles;

	// SETTINGS
	int				m_nPopSize;				// Population size
	std::string		m_sFileName;			// Population Filename
	
	// FUNCTIONS
	void initNeuralNetworks( );

public:
	GAController(	DataProvider * pGame,
					int popSize = 20, 
					std::string filename = "Population.csv",
					int testCount = TEST_COUNT);
	
	virtual ~GAController( void );
	int	step( );
    
    // GETTER AND SETTER
    int getPopulationSize(){ return m_nPopSize; }
	std::string getPopulationFileName(){ return m_sFileName; }

    void setPopulationSize( int popSize )
    { m_nPopSize = popSize; }
	void setPopulationFileName( std::string filename )
	{ m_sFileName = filename; }
	void setGameOver( bool isOver, float fitness, bool winOrLose );
};
