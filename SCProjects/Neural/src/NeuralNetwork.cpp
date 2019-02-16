/****************************************************************

Author:		Kacper Kokoszka
Date:		7.11.2010
Name:		NeuralNetwork.cpp

Content:	content of class NeuralNetwork

****************************************************************/

#include "NeuralNetwork.h"

bool NeuralNetwork::s_FirstRun = true;

NeuralNetwork::NeuralNetwork()
{
	nol = 0;
	nonil = NULL;
	neuralNetwork = NULL;
	outputs = NULL;
	learningRate = 0.9f;
	momentum = 0.5f;
	weightsDeltas = NULL;
}

NeuralNetwork::NeuralNetwork( int numberOfLayers, int *neuronsPerLayer, float *initialWeights )
{   
    learningRate = 0.1f;
	momentum = 0.0f;

	nol = numberOfLayers-1;

	if (nol<2)
	{
		//throw InvalidParameterException("Invalid number of layers");
		printf("class:NeuralNetwork:constructor: invalid number of layers\n");
		return;
	}
		
	for (int i=0; i<nol; i++)
		if (neuronsPerLayer[i] < 1)
		{
			//throw InvalidParameterException("Invalid number of neurons at layer "+i);
			printf("class:NeuralNetwork:constructor: invalid number of neurons at layer: %d\n", i);
			return;
		}
		
	nonil = new int[nol];
	for(int i=0; i<nol; i++)
		nonil[i] = neuronsPerLayer[i+1];

    int tempPrevWeights = 0;

	if( s_FirstRun )
		srand(time(NULL));
	s_FirstRun = false;
	// creating layers
	neuralNetwork = new Neuron*[nol];
	// creating neurons in layers
	for (int i=1; i<numberOfLayers; i++)
	{
		neuralNetwork[i-1] = new Neuron[neuronsPerLayer[i]];
		// initializing weights of neurons
		for(int j=0; j<neuronsPerLayer[i]; j++)
		{
			neuralNetwork[i-1][j].initializeNeuron(neuronsPerLayer[i-1]);
			neuralNetwork[i-1][j].initializeWeights();
            if( initialWeights!= 0 ) // we provided initial weights
            {
                neuralNetwork[i-1][j].initializeWeights( 
                    &( initialWeights[ tempPrevWeights ] ) );
                tempPrevWeights += neuronsPerLayer[i-1]+1; // +1 because of bias
            }
		}
	}

	outputs = new float[nonil[nol-1]];
	
	// creating a weights deltas matrix (excluding input layer)
	weightsDeltas = new float**[nol];
	for(int i=0; i<nol; i++)
	{
		weightsDeltas[i] = new float*[nonil[i]];
		for(int k=0; k<nonil[i]; k++)
			weightsDeltas[i][k] = new float[neuralNetwork[i][k].returnNumberOfInputs()];
	}
}

NeuralNetwork::~NeuralNetwork()
{
	for (int i=0; i<nol; i++)
		delete []neuralNetwork[i];
	delete []neuralNetwork;

	delete []nonil;
	delete []outputs;

	// deleting a weights deltas matrix
	for(int i=0; i<nol; i++)
	{
		for(int k=0; k<nonil[i]; k++)
			delete []weightsDeltas[i][k];
		delete []weightsDeltas[i];
	}
	delete []weightsDeltas;
}

void NeuralNetwork::setInput( const float *inputs, int numberOfInputs)
{
	for(int i=0; i<nonil[0]; i++)
		for(int j=0; j<numberOfInputs; j++)
			neuralNetwork[0][i].setInput(j, inputs[j]);
}

float* NeuralNetwork::getOutput()
{
	int outputLayerNumber = nol-1;

	for(int i=0; i<nonil[outputLayerNumber]; i++)
		outputs[i] = neuralNetwork[outputLayerNumber][i].returnOutput();

	return outputs;
}

void NeuralNetwork::activate()
{
	for (int i=0; i<nol; i++)
	{
		for(int j=0; j<nonil[i]; j++)
		{
			neuralNetwork[i][j].activateNeuron();
			if(i<nol-1)
				for(int k=0; k<nonil[i+1]; k++)
					neuralNetwork[i+1][k].setInput(j, neuralNetwork[i][j].returnOutput());
		}
	}
}

void NeuralNetwork::applyBackpropagation(float expectedOutput[])
{
	// backpropagation of output layer
	int a = nol-1;
	int b = a;	// keeps an index of output layer
	float *outputSmallDelta = new float[nonil[a]];

	// i - neuron in output layer
	for(int i=0; i<nonil[a]; i++)
	{
		outputSmallDelta[i] = neuralNetwork[a][i].countSmallDeltaForOutput(expectedOutput[i]);
		// j - input into neuron
		for(int j=0; j<neuralNetwork[a][i].returnNumberOfInputs(); j++)
			weightsDeltas[a][i][j] = - learningRate * outputSmallDelta[i] * neuralNetwork[a][i].returnInput(j);
	}

	float *middleSmallDelta1 = NULL;	// actual layer
	float *middleSmallDelta2 = outputSmallDelta;	// previous layer
	do{
		a--;
		middleSmallDelta1 = new float[nonil[a]];
		// i - neuron in middle layer
		for(int i=0; i<nonil[a]; i++)
		{
			middleSmallDelta1[i] = neuralNetwork[a][i].countSmalldeltaForMiddle(middleSmallDelta2, nonil[a+1], neuralNetwork[a+1], i);
			// j - input into neuron
			for(int j=0; j<neuralNetwork[a][i].returnNumberOfInputs(); j++)
				weightsDeltas[a][i][j] = - learningRate * middleSmallDelta1[i] * neuralNetwork[a][i].returnInput(j);
		}
		delete []middleSmallDelta2;
		middleSmallDelta2 = middleSmallDelta1;
	}while(a>0);

	// updating weights
	// i - layer number
	for(int i=0; i<nol; i++)
		// j - neuron number
		for(int j=0; j<nonil[i]; j++)
			// k - input weight number
			for(int k=0; k<neuralNetwork[i][j].returnNumberOfInputs(); k++)
				neuralNetwork[i][j].updateWeight(k, weightsDeltas[i][j][k], momentum);

	delete []middleSmallDelta1;
}

float NeuralNetwork::getLearningRate()
{
	return learningRate;
}

void NeuralNetwork::setLearningRate(float learningRate)
{
	this->learningRate = learningRate;
}

float NeuralNetwork::getMomentum()
{
	return momentum;
}

void NeuralNetwork::setMomentum(float momentum)
{
	this->momentum = momentum;
}

int NeuralNetwork::saveNet( const std::string & filename)
{
	FILE* file = NULL;

	fopen_s(&file, filename.c_str(), "wt");
	if(file==NULL)
		return 1;	// file could not be loaded

	// saving number of layers (nol) and munber of neurons in layers (nonil)
	fprintf_s(file, "%d \n", nol+1);	// saving number of layers
	fprintf_s(file, "%d ", neuralNetwork[0][0].returnNumberOfInputs()-1);	// saving input layer
	for(int i=0; i<nol; i++)	// saving other layers
		fprintf_s(file, "%d ", nonil[i]);
	fprintf_s(file, "\n");

	for(int i=0; i<nol; i++)
		for(int j=0; j<nonil[i]; j++)
			neuralNetwork[i][j].saveNeuron(file);

	fclose(file);

	return 0;
}

int NeuralNetwork::loadNet( const std::string & filename )
{
	FILE* file = NULL;

	fopen_s(&file, filename.c_str(), "rt");
	if(file==NULL)
		return 1;

	// loading number of layers
	int numberOfLayers = 0;
	fscanf_s(file, "%d", &numberOfLayers);
	nol = numberOfLayers-1;

	if (nol<2)
	{
		printf("class:NeuralNetwork:loadNet: invalid number of layers\n");
		return 2;
	}

	int *neuronsPerLayer = new int[nol+1];
		
	for (int i=0; i<nol+1; i++)
	{
		fscanf_s(file, "%d", &neuronsPerLayer[i]);
		if (neuronsPerLayer[i] < 1)
		{
			printf("class:NeuralNetwork:constructor: invalid number of neurons at layer: %d\n", i);
			return 3;
		}
	}
		
	nonil = new int[nol];
	for(int i=0; i<nol; i++)
		nonil[i] = neuronsPerLayer[i+1];

	srand(time(NULL));
	// creating layers
	neuralNetwork = new Neuron*[nol];
	// creating neurons in layers
	for (int i=1; i<numberOfLayers; i++)
	{
		neuralNetwork[i-1] = new Neuron[neuronsPerLayer[i]];
		// initializing weights of neurons
		for(int j=0; j<neuronsPerLayer[i]; j++)
		{
			neuralNetwork[i-1][j].initializeNeuron(neuronsPerLayer[i-1]);
			neuralNetwork[i-1][j].loadNeuron(file);
		}
	}

	outputs = new float[nonil[nol-1]];

	learningRate = 0.9f;
	momentum = 0.5f;
	
	// creating a weights deltas matrix (excluding input layer)
	weightsDeltas = new float**[nol];
	for(int i=0; i<nol; i++)
	{
		weightsDeltas[i] = new float*[nonil[i]];
		for(int k=0; k<nonil[i]; k++)
			weightsDeltas[i][k] = new float[neuralNetwork[i][k].returnNumberOfInputs()];
	}

	delete []neuronsPerLayer;
	fclose(file);
	return 0;
}

void NeuralNetwork::printNet()
{
	for(int i=0; i<nol; i++)
		for(int j=0; j<nonil[i]; j++)
			neuralNetwork[i][j].printNeuron();
}