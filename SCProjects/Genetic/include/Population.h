#define MUTATION_PROBABILITY	0.2f

#pragma once
#include <string.h>
#include <fstream>
#include <sstream>
#include "Chromosome.h"

class Population
{
public:
	Population();
	Population(int cSize, int avgCount = 10);
	Population(int cSize, std::string filename, int avgCount = 10);
	~Population();

	float GetMutationRate();
	void SetMutationRate(float fMutationRate = MUTATION_PROBABILITY);
	int GetPopulationSize();
	void SetPopulationSize(int value);
	void RunEpoch();

	void Sort();

	Chromosome* GetChromosomeAt(int anIndex);
	void SetChromosomeAt(int anIndex, Chromosome *pC);

	int GetNextChromosome2Evaluate();
	bool IsPopulationEvaluated();
	float GetFittestValue();
	float GetUnFittestValue();
	float GetAverageFitness();
	float GetFirstDifference();
	float GetSecondDifference();
	Chromosome* GetFittestChromosome();

	void Load(int pSize, int cSize);
	void Save();

protected:

	int					m_nWeightCount;
	int					m_nPopulationSize;
	std::string			m_CStrDataFileName;
	float				m_fMutationRate;
	Chromosome**		m_pPopulation;
	int					m_nFittest;
	int					m_nAvgCount;
};

template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&));