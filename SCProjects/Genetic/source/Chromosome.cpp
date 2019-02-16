#include <iostream>
#include <math.h>
#include "Chromosome.h"

Chromosome::Chromosome(int avgNum)
{
	m_nGeneSize = 0;
	m_fFitness = -10001;
	m_pData = NULL;
	m_nAverageCount = avgNum;
	m_nCurrentAvgCount = -1 * m_nAverageCount;
	m_sRecord = "";
}

Chromosome::Chromosome(int size, int avgNum)
{
	m_nGeneSize = size;
	m_fFitness = -10001;
	m_pData = new float[m_nGeneSize];
	for(int i=0;i<m_nGeneSize;i++)
	{
		m_pData[i]=GetRandomGene();
	}
	m_nAverageCount = avgNum;
	m_nCurrentAvgCount = -1 * m_nAverageCount;
	m_sRecord = "";
}

Chromosome::~Chromosome()
{
	/*for( int indx = 0; indx < m_nGeneSize; ++indx )
  {
    delete m_pData[indx];
  }*/
  delete [] m_pData;

	/*m_pData = NULL;
	delete m_pData;*/
}

float* Chromosome::GetChromosome()
{
	return m_pData;
}

float Chromosome::GetGene(int index)
{
	return m_pData[index];
}

void Chromosome::SetGene(int index, float value)
{
	m_pData[index] = value;
}

float Chromosome::GetFitness()
{
	return m_fFitness;
}

void Chromosome::AddFitness(float value, std::string record)
{
	if(m_fFitness<-10000)
	{
		m_fFitness = value;
		m_sRecord = record;
	}
	else
	{
		m_fFitness += value;
		m_sRecord += record;
	}
	m_nCurrentAvgCount++;
}

void Chromosome::SetFitness(float value, std::string record)
{ 
	m_fFitness = value;
	m_sRecord = record;
}

void Chromosome::SetCheckCount(int value)
{
	m_nCurrentAvgCount = value;
}

int Chromosome::GetCheckCount()
{
	return m_nCurrentAvgCount;
}

void Chromosome::SetUnChecked()
{
	m_fFitness = -10001;
	m_nCurrentAvgCount = -1 * m_nAverageCount;
}

bool Chromosome::IsEvaluated()
{
	if(m_nCurrentAvgCount==0)
		return true;
	else
		return false;
}

float Chromosome::GetDifference(Chromosome* chrome)
{
	float diff = 0.0;
	for(int i=0;i<m_nGeneSize;i++)
		diff += fabs(this->GetGene(i) - chrome->GetGene(i));
	return diff;
}

void Chromosome::Randomize()
{
	for(int i=0;i<m_nGeneSize;i++)
		m_pData[i]=GetRandomGene(getRandom(1.0,100.0));
	Mutate(1.0);
	m_fFitness = -10001;
	m_nCurrentAvgCount = -1 * m_nAverageCount;
}

void Chromosome::Mutate(float probability)
{
	bool flag = false;
	int count = m_nGeneSize;
	for(int i=0;i<m_nGeneSize;i++)
	{
		if(getRandom(0.0,1.0)<=probability)
		{
			if(GetRandomGene()<0)
				m_pData[i] -= 0.25;
			else
				m_pData[i] += 0.25;
			flag = true;
		}
	}
	if(flag)
	{
		SetUnChecked();
	}
}

Chromosome* Chromosome::Reproduce(Chromosome* chrome, int crossoverPoint)
{
	Chromosome* child = new Chromosome(m_nGeneSize, m_nAverageCount);
	for(int i=0;i<m_nGeneSize;i++)
	{
		if(i<crossoverPoint)
			child->SetGene(i, this->GetGene(i));
		else
			child->SetGene(i, chrome->GetGene(i));
	}
	child->SetUnChecked();
	return child;
}

float Chromosome::GetRandomGene(float spread)
{
	return getRandom(-1.0f*spread, 1.0f*spread);
}

std::string Chromosome::ToString()
{
	std::string str;
	for(int i=0;i<m_nGeneSize;i++)
	{
		str += toString<float>(m_pData[i]) + ";";
	}
	return str;
}

void Chromosome::SetRecord(std::string record)
{
	m_sRecord = record;
}

std::string Chromosome::GetRecord()
{
	return m_sRecord;
}

template <class T>
std::string toString(T &t)
{
	std::stringstream s;
	s<<t;
	return s.str();
}

float getRandom(float min, float max)
{
	if(!isSeeded)
	{
		srand((unsigned)time(NULL));
		isSeeded = true;
	}

	return ((max-min)*((float)rand()/RAND_MAX))+min;
}