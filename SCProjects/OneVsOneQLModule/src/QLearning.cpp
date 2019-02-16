#include <string>
#include <stdlib.h>
#include <time.h>
#include <limits>
#include <sstream>

#include "QLearning.h"
#include "NeuralNetwork.h"

/**
 * class for QLearning
 * ToDo:
    [x] how should we handle the last case??
    [x] Add Neural Network!!
 */

QLearning::QLearning( DataProvider * pGame, 
					 const std::string & nnFile,
					 float discountFactor, 
					 float learningRate, 
                     float exploreProbability )
:	m_pGame( pGame ),
	m_bGameOver( false ),
	m_pPrevState( 0 ),
    m_prevAction( -1 ),
	//SETTINGS
	m_discountFactor( discountFactor ),
	m_learningRate( learningRate ),
	m_exploreProbability( exploreProbability ),
    m_bLearning( true ),
    m_nrOfNNLayers( 3 ),
	m_networkFiles( nnFile )
{
	m_nrOfActions = m_pGame->getNrOfActions();
	m_nrOfStates =  m_pGame->getStateSize();

    // we use the randomNumberGenerator so seed first! ;-)
    srand ( clock() );

	//[x] ToDo 
	initNeuralNetworks();
}

QLearning::~QLearning(void)
{
    // FREE memory
    //if( m_pPrevState != 0 ){ delete m_pPrevState; }

    // FREE NeuralNetwork stuff
    for( int i=0; i < m_nrOfActions ; ++ i )
    {
        delete m_pNetworks[i];
    }
    delete m_pNetworks;
    delete m_pNeuronsPerLayer;
}

void QLearning::initNeuralNetworks( )
{
	m_pNeuronsPerLayer = new int[ m_nrOfNNLayers ]; // layers
    
    m_pNeuronsPerLayer[0] = m_nrOfStates; // input layer
    m_pNeuronsPerLayer[1] = (int)( m_nrOfStates / 2.0f ); // hidden layer
    m_pNeuronsPerLayer[2] = 1; // output layer

	m_pNetworks = new NeuralNetwork * [m_nrOfActions];
    for( int i=0; i < m_nrOfActions ; ++ i )
    {
        m_pNetworks[i] = new NeuralNetwork(m_nrOfNNLayers, m_pNeuronsPerLayer);
		std::stringstream str;
		str << m_networkFiles << i;
		// try to load the files! if not loadable we already have a new network
		m_pNetworks[i]->loadNet( str.str() );
        m_pNetworks[i]->saveNet( str.str() );
    }
	setLearningRate( m_learningRate );
}

void QLearning::saveNeuralNetworks( )
{
	for( int i=0; i < m_nrOfActions ; ++ i )
    {
		std::stringstream str;
		str << m_networkFiles << i;
		// try to save the files!
		m_pNetworks[i]->saveNet( str.str() );
    }
}


void QLearning::setLearningRate( float learningRate )
{
    m_learningRate = learningRate;
    //[x] ToDo:
    for( int i=0; i < m_nrOfActions ; ++ i )
    {
       m_pNetworks[i]->setLearningRate( learningRate );
    }
}



int	QLearning::step( )
{

    // we only train the network if we have a previous state.
    bool bTrain = m_prevAction != -1;


    // ALLOCATE memory and fill it with the current state.
	float * pCurrentState = new float[m_nrOfStates];

    if( !m_bGameOver )
    {
        memcpy_s( 
            pCurrentState, sizeof(float) * m_nrOfStates,
            m_pGame->getCurrentState(), sizeof(float) * m_nrOfStates );
    }
    else
    {
        delete pCurrentState;
        pCurrentState = m_pPrevState;
        //m_pPrevState = 0;
    }
    
    // get the Best action the network in its current training state would suggest
    int bestAction = getBestAction( pCurrentState );
    int currentAction = bestAction;


    if( m_bLearning && bTrain )
    {
        float reward = m_pGame->getReward();
        
        float maxQ = getQ( pCurrentState, bestAction );

        //[x] Todo:
        // set the Inputs with m_pPrevState for the network with the index m_prevAction!!
        m_pNetworks[m_prevAction]->setInput( m_pPrevState, m_pGame->getStateSize()  );
        
        // activate it to set the weights for the backprop right!
        m_pNetworks[m_prevAction]->activate();

        float desiredOutput = reward + m_discountFactor * maxQ;
        
        //[x] Todo:
        // apply backpropagation with the desiredOutput.
        m_pNetworks[m_prevAction]->applyBackpropagation( &desiredOutput );
    }

    
    // exploring or exploiting
    float r = (float)rand()/(float)RAND_MAX; // random value between 0 and 1;
    if( m_bLearning && m_exploreProbability > r )
    {
        // with the probablity of m_exploreProb.. we set the currentAction to a random value
        // for exploring the state space. Otherwise the bestAction will be chosen
        currentAction = rand() % m_pGame->getNrOfActions();
    }
    

    // FREE memory!
    if( m_pPrevState != 0 ){ delete m_pPrevState; }
    m_prevAction = currentAction;
    m_pPrevState = pCurrentState;


	// SAVE the NeuralNetworks ... every frame so we always have some intermediate results
	saveNeuralNetworks( );


    return currentAction;
}



int QLearning::getBestAction( const float * pCurrentState )
{   
    int     bestAction = -1;
    float   bestQ = - std::numeric_limits<float>::max(); 

    for( int i = 0; i < m_pGame->getNrOfActions(); ++ i )
    {
        float value = getQ( pCurrentState, i );
        if( value > bestQ )
        {
            bestQ = value;
            bestAction = i;
        }
    }

    return bestAction;
}


float QLearning::getQ( const float * pCurrentState, int action )
{  
    //[x] ToDo:
    //  fill in the code for the network
    m_pNetworks[action]->setInput( pCurrentState, m_pGame->getStateSize()  );
        
    // activate it to set the weights for the backprop right!
    m_pNetworks[action]->activate();

    return * m_pNetworks[action]->getOutput();
}