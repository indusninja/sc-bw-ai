#pragma once

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
	  virtual float getFitness() const = 0;
	  // the current state which can be used in the Network
	  // the values should be copied
	  virtual const float * getCurrentState() = 0;
	  // the number of possible Actions;
	  const int getNrOfActions() const
	  { return m_nrOfActions; }
	  const int getStateSize() const
	  { return m_stateSize; }
};