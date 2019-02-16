#include <string>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <sstream>
#include <iostream>
#include <fstream>

#include "GAController.h"
#include "NeuralNetwork.h"
#include "Population.h"

GAController::GAController( DataProvider * pGame,
						   int popSize, 
						   std::string filename,
						   int testCount)
						   :	m_pGame( pGame ),
						   m_bGameOver(false),
						   m_nrOfNNLayers( 3 ),
						   m_nPopSize(popSize),
						   m_sFileName(filename),
						   m_nTestCount(testCount)
{
	m_nrOfActions = m_pGame->getNrOfActions();
	m_nrOfStates =  m_pGame->getStateSize();

	// we use the randomNumberGenerator so seed first! ;-)
	srand ( clock() );

	//m_nChromosomeSize = ((m_nrOfStates+1)*(int)( m_nrOfStates / 2.0f )) + ((m_nrOfStates+1)*(int)( m_nrOfStates / 2.0f )) + (((int)( m_nrOfStates / 2.0f )+1)*2);
	//m_nChromosomeSize = ((m_nrOfStates+1)*2) + ((3)*2);
	m_nChromosomeSize = ((m_nrOfStates+1)*1) + ((1+1)*3);

	m_pPop = new Population(m_nChromosomeSize, m_sFileName, m_nTestCount);

	m_pPop->Load(m_nPopSize, m_nChromosomeSize);

	if(m_pPop->IsPopulationEvaluated())
	{
		std::ofstream myfile;
		std::stringstream s;
		s<<"GA_Analysis"<<m_nChromosomeSize<<".csv";
		myfile.open (s.str().c_str(), ios::out | ios::app);
		myfile<<m_pPop->GetUnFittestValue()<<";"
			<<m_pPop->GetAverageFitness()<<";"
			<<m_pPop->GetFittestValue()<<";";
		
		m_pPop->RunEpoch();

		myfile<<m_pPop->GetFirstDifference()<<";"<<m_pPop->GetSecondDifference()<<";"<<std::endl;

		myfile.close();
	}

	initNeuralNetworks();
}

GAController::~GAController(void)
{
	// FREE NeuralNetwork stuff
	//m_pGame->~DataProvider();
	//delete m_pGame;

	//m_pNetwork->~NeuralNetwork();
	delete m_pNetwork;

	delete [] m_pNeuronsPerLayer;

	//m_pPop->~Population();
	delete m_pPop;
}

void GAController::initNeuralNetworks( )
{
	m_pNeuronsPerLayer = new int[ m_nrOfNNLayers ];			// layers

	m_pNeuronsPerLayer[0] = m_nrOfStates;					// input layer
	m_pNeuronsPerLayer[1] = 1;								// hidden layer 1
	//m_pNeuronsPerLayer[2] = (int)( m_nrOfStates / 2.0f );	// hidden layer 2
	m_pNeuronsPerLayer[2] = 3;								// output layer

	int evalChrome = m_pPop->GetNextChromosome2Evaluate();
	//evalChrome = 0;
	m_pNetwork = new NeuralNetwork(m_nrOfNNLayers, m_pNeuronsPerLayer, m_pPop->GetChromosomeAt(evalChrome)->GetChromosome());
}

int	GAController::step( )
{
	// ALLOCATE memory and fill it with the current state.
	float *pCurrentState = new float[m_nrOfStates];

	if( !m_bGameOver )
	{
		memcpy_s( 
			pCurrentState, sizeof(float) * m_nrOfStates,
			m_pGame->getCurrentState(), sizeof(float) * m_nrOfStates );
	}
	
	m_pNetwork->setInput( pCurrentState, m_pGame->getStateSize()  );

	// activate it to set the weights for the backprop right!
	m_pNetwork->activate();

	float *output = NULL;

	output = m_pNetwork->getOutput();

	delete [] pCurrentState;

	/*if(output[0]>output[1])
		return 0;
	else
		return 1;*/

	if(output[0]>output[1])
	{
		if(output[0]>output[2])
			return 0;
		else
			return 2;
	}
	else
	{
		if(output[1]>output[2])
			return 1;
		else
			return 2;
	}
	//return 2;
	/*if(output[0]>output[1]&&output[0]>output[2])
		return 0;
	else if(output[1]>output[0]&&output[1]>output[2])
		return 1;
	else
		return 2;*/
}

void GAController::setGameOver( bool isOver, float fitness, bool winOrLose)
{ 
	m_bGameOver = isOver;
	int evalChrome = m_pPop->GetNextChromosome2Evaluate();
	if(winOrLose)
		m_pPop->GetChromosomeAt(evalChrome)->AddFitness(fitness, "W|");
	else
		m_pPop->GetChromosomeAt(evalChrome)->AddFitness(fitness, "L|");

	m_pPop->Save();
}