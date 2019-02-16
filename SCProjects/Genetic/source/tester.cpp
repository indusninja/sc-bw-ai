#include <iostream>
#include "NeuralNetwork.h"
#include "Population.h"
using namespace std;

int main()
{
	cout<<"This is a tester for the GA code (Yes, in C++)..."<<endl;

	//Inputs for "exclusive or" problem
	float inputs[4][2] = {{ 1.0, 1.0}, { 1.0, 0.0}, { 0.0, 1.0}, { 0.0, 0.0}};
	int noi = 2;	// number of inputs

	//Corresponding outpus
	float expectedOutputs[4][1] = {{0}, {1}, {1}, {0}}; 
	float *output = NULL;

	//Create neural network
	int nol = 3;	// number of layers
	int layers[] = {2,2,1};
	int chromeSize = ((layers[0]+1)*layers[1]) + ((layers[1]+1)*layers[2]);
	//	NeuralNetwork *neuralNet = new NeuralNetwork(nol, layers);

	float currentFittest = 10000, historicalBest = 10000;
	// TODO: This loop condition looks wrong
	while(currentFittest>0.01)
	{
		Population *pop = new Population(chromeSize, "test.csv", 2);
		pop->Load(20, chromeSize);
		if(pop->IsPopulationEvaluated())
		{
			// Reproduce before picking a chromosome to evaluate
			currentFittest = pop->GetFittestValue();
			if(currentFittest<historicalBest)
				historicalBest = currentFittest;
			cout<<"Fittest Chromosome is = "<<currentFittest<<"; \tHistorical Best = "<<historicalBest<<endl;
			pop->RunEpoch();
			//pop->Sort();
			//pop->Save();
			//return 1;
		}
		int evalChrome = pop->GetNextChromosome2Evaluate();
		NeuralNetwork *neuralNet = new NeuralNetwork(nol, layers, pop->GetChromosomeAt(evalChrome)->GetChromosome());

		// Eval Neural Network error for this network
		float error = 0.0;
		for(int p=0; p<4; p++ )
		{
			neuralNet->setInput(inputs[p], noi);

			neuralNet->activate();

			output = neuralNet->getOutput();
			error += (output[0] - expectedOutputs[p][0])*(output[0] - expectedOutputs[p][0]);
		}
		error /= 4.0; //error = 1- error;

		pop->GetChromosomeAt(evalChrome)->AddFitness(error);
		//pop->GetChromosomeAt(evalChrome)->SetChecked();

		neuralNet->~NeuralNetwork();
		pop->Save();
		pop->~Population();
		//system("pause");
	}

	system("pause");
	return 1;
}