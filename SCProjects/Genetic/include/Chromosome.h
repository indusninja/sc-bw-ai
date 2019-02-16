#define GAUSSIAN_SPREAD 1.0f
#define AVERAGING_COUNT 10

#pragma once
#include <ctime>
#include <sstream>

class Chromosome  
{
public:
	Chromosome(int avgNum = AVERAGING_COUNT);
	Chromosome(int size, int avgNum = AVERAGING_COUNT);
	~Chromosome();

	float* GetChromosome();
	float GetGene(int index);
	void SetGene(int index, float value);
	float GetFitness();
	void SetFitness(float value, std::string record = " ");
	void AddFitness(float value, std::string record = " ");
	void SetCheckCount(int value);
	int GetCheckCount();
	void SetUnChecked();
	bool IsEvaluated();
	float GetDifference(Chromosome* chrome);

	void Mutate(float probability);
	Chromosome* Reproduce(Chromosome* chrome, int crossoverPoint);
	void Randomize();

	std::string ToString();
	std::string GetRecord();
	void SetRecord(std::string record);

protected:
	float GetRandomGene(float spread = GAUSSIAN_SPREAD);

	int					m_nGeneSize;   
	float				m_fFitness;
	float				*m_pData;
	int					m_nAverageCount;
	int					m_nCurrentAvgCount;
	std::string			m_sRecord;
};

template <class T>
std::string toString(T &t);

static bool isSeeded = false;

float getRandom(float min, float max);