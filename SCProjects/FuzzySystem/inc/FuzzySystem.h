#pragma once

#include <string.h>
#include <map>

/// PREDECLARATIONS
class LinguisticVariable;
class RuleStack;
class Clause;

class FuzzySystem
{
private:
    
    typedef std::map< std::string, LinguisticVariable * > LinguisticMap;
    typedef std::pair< std::string, LinguisticVariable * > LinguisticPair;
	LinguisticMap m_LinguisticVariables;

    std::map< std::string, RuleStack * > m_Rules;
    typedef std::map< std::string, RuleStack * > RulesMap;
    typedef std::pair< std::string, RuleStack * > RulesPair;

    int m_defuzzficationInterval;

    //RuleStack * parseRuleString( std::string rule );
	void checkConsistency( RuleStack * pStack, bool bCheckThenRule = false );
    void checkAndUpdateClause( Clause * pClause );

public:
    FuzzySystem(void);
    ~FuzzySystem(void);

    void addVariable( LinguisticVariable & rVariable );
    void setInput( const std::string & rVarName, double value );

    //void newRule( const std::string & rName, const std::string & rRule );
	void newRule( const std::string & rName, const RuleStack & stack );


    double evaluate( const std::string & var ) const;
};
