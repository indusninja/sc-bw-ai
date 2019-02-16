/****************************************************************

Author:		Kacper Kokoszka
Date:		7.11.2010
Name:		Neuron.h

Content:	definition of class Neuron

****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#pragma once

class Neuron
{
private:
	int noi;		// number of inputs
	float *inputs;
	float *weights;
	float *oldWeights;
	float output;

public:
	Neuron();
	Neuron(int numberOfInputs);
	~Neuron();

	void initializeNeuron(int numberOfInputs);	// does the same thing as Neuron(int numberOfInputs) constructor
	void activateNeuron();
	float countSmallDeltaForOutput(float expectedOutput);
	float returnWeight(int index);
	void updateWeight(int index, float delta, float momentum);
	float countSmalldeltaForMiddle(float outputLayerSmallDelta[], int outputLayerSmallDeltaLenght, Neuron outputLayer[], int actualNeuronIndex);
	void setInput(int inputIndex, float inputValue);
	float returnOutput();
	float returnInput(int index);
	int returnNumberOfInputs();
	void initializeWeights();
	void initializeWeights(float *weights);

	void saveNeuron(FILE* file);
	void loadNeuron(FILE* file);
	void printNeuron();
};