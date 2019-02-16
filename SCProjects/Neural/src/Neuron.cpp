/****************************************************************

Author:		Kacper Kokoszka
Date:		7.11.2010
Name:		Neuron.cpp

Content:	content of class NeuralNetwork

****************************************************************/

#include "Neuron.h"

Neuron::Neuron()
{
	noi = 0;
	inputs = NULL;
	weights = NULL;
	oldWeights = NULL;
}

Neuron::Neuron(int numberOfInputs)
{
	if(numberOfInputs<1)
	{
		//throw InvalidParameterException("Invalid number of inputs");
		printf("class:Neuron:constructor: invalid number of inputs\n");
		return;
	}

	noi = numberOfInputs+1;
	inputs = new float[noi];
	inputs[noi-1] = -1;
	weights = new float[noi];
	oldWeights = new float[noi];
}

Neuron::~Neuron()
{
	delete []inputs;
	delete []weights;
	delete []oldWeights;
}

void Neuron::initializeNeuron(int numberOfInputs)
{
	if(numberOfInputs<1)
	{
		//throw InvalidParameterException("Invalid number of inputs");
		printf("class:Neuron:constructor: invalid number of inputs\n");
		return;
	}

	noi = numberOfInputs+1;
	inputs = new float[noi];
	inputs[noi-1] = -1;
	weights = new float[noi];
	oldWeights = new float[noi];
}

void Neuron::initializeWeights( float * initWeights )
{
	for(int i=0; i<noi; i++)
	{
		weights[i] = initWeights[i];
	}
}

void Neuron::activateNeuron()
{
	float sum = 0;
	double d = -1.0;
	for(int i=0; i<noi; i++)
		sum += inputs[i]*weights[i];

	output = (float)(1/(1+exp(d*sum)));
}

float Neuron::countSmallDeltaForOutput(float expectedOutput)
{
	float smallDelta = -(expectedOutput - output) * output * (1 - output);

	return smallDelta;
}

float Neuron::returnWeight(int index)
{
	return weights[index];
}

void Neuron::updateWeight(int index, float delta, float momentum)
{
	if(index>=noi)
	{	
		//throw InvalidParameterException("Invalid update weight index");
		printf("class:Neuron:updateWeight: invalid update weight index\n");
		return;
	}

	float temp = weights[index];	// rememberig new old weight
	weights[index] += (delta + momentum*(weights[index]-oldWeights[index]));
	oldWeights[index] = temp;
}

float Neuron::countSmalldeltaForMiddle(float outputLayerSmallDelta[], int outputLayerSmallDeltaLenght, Neuron outputLayer[], int actualNeuronIndex)
{
	float smallDelta = 0;
	for(int i=0; i<outputLayerSmallDeltaLenght; i++)
		smallDelta += outputLayerSmallDelta[i]*outputLayer[i].returnWeight(actualNeuronIndex);

	smallDelta *= output * (1 - output);

	return smallDelta;
}

void Neuron::setInput(int inputIndex, float inputValue)
{
	inputs[inputIndex] = inputValue;
}

float Neuron::returnOutput()
{
	return output;
}

float Neuron::returnInput(int index)
{
	return inputs[index];
}

int Neuron::returnNumberOfInputs()
{
	return noi;
}

void Neuron::initializeWeights()
{
	for(int i=0; i<noi; i++)
	{
		weights[i] = ((rand()%10) +1)*0.1f;
		oldWeights[i] = weights[i];
		if(rand()%2==0)
			weights[i] *= -1.0f;
	}
}

void Neuron::saveNeuron(FILE* file)
{
	for(int i=0; i<noi; i++)
		fprintf_s(file, "%f ", weights[i]);
	fprintf_s(file, "\n");
	for(int i=0; i<noi; i++)
		fprintf_s(file, "%f ", oldWeights[i]);
	fprintf_s(file, "\n");
}

void Neuron::loadNeuron(FILE* file)
{
	for(int i=0; i<noi; i++)
		fscanf_s(file, "%f", &weights[i]);
	for(int i=0; i<noi; i++)
		fscanf_s(file, "%f", &oldWeights[i]);
}

void Neuron::printNeuron()
{
	printf("Inputs: ");
	for(int i=0; i<noi; i++)
		printf("%f, ", inputs[i]);
	printf("\n");
	printf("Weights: ");
	for(int i=0; i<noi; i++)
		printf("%f, ", weights[i]);
	printf("\n");
	printf("Output: %f\n", output);
}