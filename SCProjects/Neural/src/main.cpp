/****************************************************************

Author:		Kacper Kokoszka
Date:		7.11.2010
Name:		main.cpp

Content:	testing of neural network function with xor example

****************************************************************/

#include <stdio.h>
#include "NeuralNetwork.h"

void main()
{
	//Inputs for "exclusive or" problem
	float inputs[4][2] = {{ 1.0, 1.0}, { 1.0, 0.0}, { 0.0, 1.0}, { 0.0, 0.0}};
	int noi = 2;	// number of inputs

	//Corresponding outpus
	float expectedOutputs[4][1] = {{0}, {1}, {1}, {0}}; 
	float *output = NULL;

	//Create neural network
	int nol = 4;	// number of layers
	int layers[] = {2,3,2,1};
	NeuralNetwork *neuralNet = new NeuralNetwork(nol, layers);
	//NeuralNetwork *neuralNet = new NeuralNetwork();
	//neuralNet->loadNet("file.txt");

	//Train neural network and print out to screen as we go along
	for(int i=0; i<100000; i++ )
	{
		printf("##### EPOCH %d\n", i);
		for(int p=0; p<4; p++ )
		{
			if((i+1)%500==0)
			{
				printf("INPUTS: ");
				for( int x=0; x<layers[0]; x++ ) { printf("%f ", inputs[p][x]); }
				printf("\n");
			}
			neuralNet->setInput(inputs[p], noi);

			neuralNet->activate();

			//neuralNet->printNet();
			//getchar();

			output = neuralNet->getOutput();
			if((i+1)%500==0)
			{
				printf("OUTPUTS: ");
				for( int x=0; x<layers[nol-1]; x++ )
					printf("%f", output[x]);
				printf("\n");
			}

			neuralNet->applyBackpropagation(expectedOutputs[p]);
			if((i+1)%500==0)
			{
				printf("EXPECTED OUTPUTS: ");
				for( int x=0; x<layers[nol-1]; x++ ) { printf("%f", expectedOutputs[p][x]); }
				printf("\n");
			}
		}
		
		//getchar();

		if((i+1)%500==0)
		{
			neuralNet->saveNet("file.txt");
			getchar();
		}
	}

	delete neuralNet;

	getchar();
}