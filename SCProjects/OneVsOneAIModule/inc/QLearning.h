#pragma once

#include <string>

/// PREDECLARATIONS ////////////////////////
class NeuralNetwork;

class QLearning
{
public:

	////////////////////////////////////////
	class DataProvider {
	private:
		int		m_nrOfActions;
        int     m_stateSize;
	public:
        DataProvider( int nrOfActions, int stateSize ):
			m_nrOfActions( nrOfActions ),
            m_stateSize( stateSize )
        {}
		virtual ~DataProvider( void ){}

		// provides the reward for the previous action
		virtual float getReward() const = 0;
		// the current state which can be used in the Network
		// the values should be copied
		virtual const float * getCurrentState() = 0;
		// the number of possible Actions;
		const int getNrOfActions() const
        { return m_nrOfActions; }
        const int getStateSize() const
        { return m_stateSize; }
	};
	//////////////////////////////////////////
private:
	DataProvider *	m_pGame;
	bool			m_bGameOver;
	float *			m_pPrevState;
    int             m_prevAction;
    int             m_nrOfActions;
	int				m_nrOfStates;

    // Neural Network stuff
    NeuralNetwork** m_pNetworks;
    int             m_nrOfNNLayers;
    int *           m_pNeuronsPerLayer;
	std::string		m_networkFiles;

	// SETTINGS
	float			m_discountFactor;   //gamma
	float			m_learningRate;     // alpha
	float			m_exploreProbability; // epsilon

    bool            m_bLearning; // we only backprop the NN and use the exploring if this is TRUE!

	// FUNCTIONS
	void initNeuralNetworks( );
	void saveNeuralNetworks( );

public:
	QLearning(	DataProvider * pGame,
				const std::string & rNeuralNetworkFileName,
				float discountFactor = 0.9f, 
				float learningRate = 0.1f, 
				float exploreProbability = 0.3f );
	
	virtual ~QLearning( void );
	int	step( );
    int getBestAction( const float * pCurrentState );
    float getQ( const float * pCurrentState, int action );


    // GETTER AND SETTER
    float getDiscountFactor(){ return m_discountFactor; }
    float getLearningRate(){ return m_learningRate; }
    float getExploreProbability(){ return m_exploreProbability; }
    bool getIsLearning(){ return m_bLearning; }

    void setDiscountFactor( float discountFactor )
    { m_discountFactor = discountFactor; }
    void setLearningRate( float learningRate );
    void setExploreProbability( float exploreProbability )
    { m_exploreProbability = exploreProbability; }
    void setIsLearning( bool value )
    {m_bLearning = value; }

    void setGameOver( bool isOver ){ m_bGameOver = isOver; }
};
