/****************************************************************

Author:		Kacper Kokoszka
Date:		7.11.2010
Name:		NeuralNetwork.h

Content:	definition of class NeuralNetwork

****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "Neuron.h"

#pragma once

class NeuralNetwork
{
private:
	float learningRate;		// backpropagation learning rate
	float momentum;			// mometum parameter

	Neuron **neuralNetwork;
	int nol;	// number of layers
	int *nonil;	// number of neurons in layers

	float *outputs;		// values of outputs

	float ***weightsDeltas;		// table containing deltas needs to be added to weights

	static bool s_FirstRun;

public:
	/** Allocate memory for the neural network and setup data structure 
	* @param neuronsPerLayer an array containing the numbers of neurons for each layer (input layer, hidden layer 1... , output layer)
	* @exception InvalidParameterException thrown if less then two layers are requested (input and output layers are mandatory) and if some layers don't contain at least 1 neuron. 
	*/
	NeuralNetwork();
	NeuralNetwork( int numberOfLayers, int *neuronsPerLayer, float * initialWeights = 0 );
	~NeuralNetwork();

	/** Copy the inputs vector onto the inputs of the neural net 
	* @param inputs inputs values of the neural net
	*/
	void setInput( const float *inputs, int numberOfInputs);

	/** Return a copy of the current output values 
	* @return a copy of the current output values
	*/
	float* getOutput();

	/** Calculate the output of the neural network based on the input
	* 
	*/
	void activate();

	/** update the weights of the network based on the different between the actual output and the expectedOutput */
	void applyBackpropagation(float *expectedOutput);

	/** Learning rate getter
	* @return the learningRate
	*/
	float getLearningRate();

	/** Learning rate setter
	* @param learningRate the learningRate to set
	*/
	void setLearningRate(float learningRate);

	float getMomentum();
	void setMomentum(float momentum);

	int saveNet( const std::string & filename );
	int loadNet( const std::string & filename );
	void printNet();
};
