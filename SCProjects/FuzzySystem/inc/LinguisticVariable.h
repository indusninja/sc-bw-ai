#pragma once

// needed for max in Visual Studio
#undef max
#undef min

#include <string>
#include <map>
#include <limits>

using namespace std;

/// PREDECLARATIONS ///
class IFuzzyMemberFunctions;

class LinguisticVariable
{
	friend class FuzzySystem;
protected:
	
	typedef std::map<  std::string, IFuzzyMemberFunctions * > FuzzySetMap;
    typedef std::pair< std::string, IFuzzyMemberFunctions * > FuzzySetPair;
	FuzzySetMap m_fuzzySets;

	std::string         m_name;
    double              m_input;
    double              m_min, m_max;
public:
    LinguisticVariable( 
                const string & name, 
                double min = - numeric_limits<double>::max(), 
                double max = numeric_limits<double>::max() );
	LinguisticVariable( const LinguisticVariable & var );
	~LinguisticVariable( void );

	void addFuzzySet( const string & name, const IFuzzyMemberFunctions & memberFunction );
    std::string getName( ) const { return m_name; }
    void setInput( double v ){ m_input = v; }
    double getInput( void ) const { return m_input; }

    double getMembership( const string & fuzzySet ) const;
    double getMembership( const string & fuzzySet, double x ) const;
};
