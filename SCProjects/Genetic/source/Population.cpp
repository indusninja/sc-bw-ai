#include <set>
#include <vector>
#include <algorithm>
#include <iostream>
#include "Population.h"

Population::Population()
{
	m_nWeightCount = 0;
	m_CStrDataFileName = "test.csv";
	m_nPopulationSize = 100;
	m_fMutationRate = 0.75f;
	m_pPopulation = NULL;
	m_nAvgCount = 1;
}

Population::Population(int cSize, int avgCount)
{
	m_nWeightCount = cSize;
	m_CStrDataFileName = "test.csv";
	m_nPopulationSize = 100;
	m_fMutationRate = 0.75f;
	m_nAvgCount = avgCount;
	m_pPopulation = new Chromosome*[m_nPopulationSize];
	for(int i=0;i<m_nPopulationSize;i++)
	{
		m_pPopulation[i] = new Chromosome(m_nWeightCount, m_nAvgCount);
	}
}

Population::Population(int cSize, std::string filename, int avgCount)
{
	m_nWeightCount = cSize;
	m_CStrDataFileName = filename;
	m_nPopulationSize = 100;
	m_fMutationRate = 0.75f;
	m_nAvgCount = avgCount;
	m_pPopulation = NULL;
}

Population::~Population()
{
	if(m_pPopulation!=NULL)
	{
		for( int indx = 0; indx < m_nPopulationSize; ++indx )
		{
			delete m_pPopulation[indx];
		}
		delete [] m_pPopulation;
		//m_pPopulation = NULL;
	}
	//delete m_pPopulation;
}

float Population::GetMutationRate()
{
	return m_fMutationRate;
}

void Population::SetMutationRate(float fMutationRate)
{
	m_fMutationRate = fMutationRate;
}

int Population::GetPopulationSize()
{
	return m_nPopulationSize;
}

void Population::SetPopulationSize(int value)
{
	m_nPopulationSize = value;
}

void Population::RunEpoch()
{
	// Pre-sort for assigning sampling bias (range on the line that each chromosome lies within)
	Sort();

	// Start selection
	//std::cout<<"Selecting Parents: 0|";
	std::set<int> uniqueIndices;
	uniqueIndices.insert( 0 );
	int beginPointer = 1, endPointer = m_nPopulationSize-1;
	while(uniqueIndices.size()<(unsigned)(m_nPopulationSize/3))
	{
		if(getRandom(-1.0,1.0)<0.0)
		{
			uniqueIndices.insert(beginPointer);
			//std::cout<<beginPointer<<"|";
			beginPointer++;
		}
		else
		{
			int newIndex = (int)getRandom((float)beginPointer+1, (float)endPointer);
			uniqueIndices.insert(newIndex);
			//std::cout<<newIndex<<"|";
		}
	}
	//std::cout<<std::endl<<"Unique indices selected: "<<uniqueIndices.size()<<std::endl;

	std::vector<int> selectedIndices, unselectedIndices;
	std::set<int>::const_iterator setIter;
	for(int i=0;i<m_nPopulationSize;i++)
	{
		setIter = uniqueIndices.find( i );
		// Selected for Parenting
		if ( setIter == uniqueIndices.end( ) )
		{
			unselectedIndices.push_back(i);
		}
		// Selected for replacing
		else
		{
			selectedIndices.push_back(i);
		}
	}
	//std::cout<<"Vector size of Selected indices: "<<selectedIndices.size()<<std::endl;
	//std::cout<<"Vector size of UnSelected indices: "<<unselectedIndices.size()<<std::endl;

	std::random_shuffle(selectedIndices.begin(), selectedIndices.end());
	for(int i=0;i<(int)selectedIndices.size();i+=2)
	{
		if(i+1==selectedIndices.size())
			break;
		int crossOverPoint = (int)getRandom(1.0, (float)(m_nWeightCount - 1.0));
		//std::cout<<"Reproducing "<<selectedIndices[i]<<" & "<<selectedIndices[i+1]<<" @ "<<crossOverPoint<<std::endl;
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]]->~Chromosome();
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]] = NULL;
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]] = m_pPopulation[selectedIndices[i]]->Reproduce(m_pPopulation[selectedIndices[i+1]], crossOverPoint);
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]]->Mutate(m_fMutationRate);
		unselectedIndices.pop_back();
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]]->~Chromosome();
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]] = NULL;
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]] = m_pPopulation[selectedIndices[i+1]]->Reproduce(m_pPopulation[selectedIndices[i]], crossOverPoint);
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]]->Mutate(m_fMutationRate);
		unselectedIndices.pop_back();
		//std::cout<<"Reproduction cycle, replacable chromosomes left = "<<unselectedIndices.size()<<std::endl;
	}

	// Replace the rest of the population (hardcore mutation, meaning MUST mutate, no probability check)
	while(unselectedIndices.size()>0)
	{
		m_pPopulation[unselectedIndices[unselectedIndices.size()-1]]->Mutate(1.1);
		unselectedIndices.pop_back();
		//std::cout<<"Mutation cycle, chromosomes left = "<<unselectedIndices.size()<<std::endl;
	}

	uniqueIndices.clear();
	selectedIndices.clear();
	unselectedIndices.clear();
	//system("pause");
}

// This criteria should be changed for different purposes
void Population::Sort()
{
	for( int x=0; x<m_nPopulationSize; x++ )
	{
		Chromosome* pFirst = GetChromosomeAt( x );
		for( int y=x+1; y<m_nPopulationSize; y++ )
		{
			Chromosome* pSecond = GetChromosomeAt( y );
			if( pFirst->GetFitness() < pSecond->GetFitness() )//swap
			{				
				SetChromosomeAt( x, pSecond );
				SetChromosomeAt( y, pFirst );
				pFirst = pSecond;
			}
		}
		pFirst = NULL;
		delete pFirst;
	}
}

Chromosome* Population::GetChromosomeAt(int anIndex)
{
	return m_pPopulation[anIndex];
}

void Population::SetChromosomeAt(int anIndex, Chromosome *pC)
{
	m_pPopulation[ anIndex ] = pC;
}

// This criteria should be changed for different purposes
float Population::GetFittestValue()
{
	m_nFittest = -1;
	float fittestval = -10005.0;
	for(int i=0;i<m_nPopulationSize;i++)
	{
		if(m_pPopulation[i]->GetFitness()>fittestval)
		{
			fittestval = m_pPopulation[i]->GetFitness();
			m_nFittest = i;
		}
	}
	return fittestval;
}

float Population::GetUnFittestValue()
{
	m_nFittest = -1;
	float fittestval = 10000.0;
	for(int i=0;i<m_nPopulationSize;i++)
	{
		if(m_pPopulation[i]->GetFitness()<fittestval)
		{
			fittestval = m_pPopulation[i]->GetFitness();
			m_nFittest = i;
		}
	}
	return fittestval;
}

float Population::GetAverageFitness()
{
	float average = 0.0;
	for(int i=0;i<m_nPopulationSize;i++)
	{
		average += m_pPopulation[i]->GetFitness();
	}
	average /= (float)m_nPopulationSize;
	return average;
}

Chromosome* Population::GetFittestChromosome()
{
	GetFittestValue();
	if(m_nFittest!=-1)
		return m_pPopulation[m_nFittest];
	else
		return NULL;
}

float Population::GetFirstDifference()
{
	return m_pPopulation[0]->GetDifference(m_pPopulation[1]);
}

float Population::GetSecondDifference()
{
	return m_pPopulation[1]->GetDifference(m_pPopulation[2]);
}

int Population::GetNextChromosome2Evaluate()
{
	for(int i=0;i<m_nPopulationSize;i++)
	{
		if(!m_pPopulation[i]->IsEvaluated())
			return i;
	}
	return -1;
}

bool Population::IsPopulationEvaluated()
{
	for(int i=0;i<m_nPopulationSize;i++)
	{
		if(!m_pPopulation[i]->IsEvaluated())
			return false;
	}
	return true;
}

void Population::Load(int pSize, int cSize)
{
	std::ifstream  data(m_CStrDataFileName.c_str());
	std::string line;
	int iter = 0;

	if(m_pPopulation!=NULL)
	{
		for( int indx = 0; indx < m_nPopulationSize; ++indx )
		{
			delete m_pPopulation[indx];
		}
		delete [] m_pPopulation;
		m_pPopulation = NULL;
	}

	m_nPopulationSize = pSize;
	m_pPopulation = new Chromosome*[m_nPopulationSize];

	while(getline(data,line))
	{
		if(iter<m_nPopulationSize)
		{
			m_pPopulation[iter] = new Chromosome(cSize, m_nAvgCount);

			std::stringstream	lineStream(line);
			std::string			cell;
			int					location = 0;
			while(getline(lineStream,cell,';'))
			{
				float f;
				if(from_string<float>(f, cell, std::dec))
				{
					if(location==cSize)
					{
						m_pPopulation[iter]->SetFitness(f);
					}
					else if (location==cSize+1)
					{
						/*if(f>=0&&f<=0.0001)
							m_pPopulation[iter]->SetUnChecked();
						else
							m_pPopulation[iter]->SetChecked();*/
						m_pPopulation[iter]->SetCheckCount((int)f);
					}
					else if (location==cSize+2)
					{
						m_pPopulation[iter]->SetCheckCount((int)f);
					}
					else
					{
						m_pPopulation[iter]->SetGene(location, f);
					}
				}
				if (location==cSize+2)
				{
					m_pPopulation[iter]->SetRecord(cell);
				}
				location++;
			}
		}
		iter++;
	}

	for(int i=iter;i<m_nPopulationSize;i++)
	{
		m_pPopulation[i] = new Chromosome(cSize, m_nAvgCount);
	}
}

void Population::Save()
{
	std::ofstream myfile;
	myfile.open(m_CStrDataFileName.c_str());
	for(int i=0;i<m_nPopulationSize;i++)
	{
		for(int j=0;j<m_nWeightCount;j++)
			myfile<<m_pPopulation[i]->GetGene(j)<<";";
		myfile<<m_pPopulation[i]->GetFitness()<<";";
		myfile<<m_pPopulation[i]->GetCheckCount()<<";";
		myfile<<m_pPopulation[i]->GetRecord()<<";"<<std::endl;
	}
	myfile.close();
}

template <class T>
bool from_string(T& t, const std::string& s, std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}
