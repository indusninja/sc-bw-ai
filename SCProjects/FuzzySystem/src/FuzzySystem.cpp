#include <algorithm>
#include <sstream>

#include "FuzzySystem.h"
#include "LinguisticVariable.h"
#include "RuleStack.h"
#include "Clause.h"
#include "LinguisticVariable.h"
#include "FuzzyMemberFunctions.h"


FuzzySystem::FuzzySystem(void)
:   m_defuzzficationInterval( 1000 )
{
}

FuzzySystem::~FuzzySystem(void)
{
    // ToDo:
    // delete rules!!!
    while( m_Rules.size() > 0 )
	{
        RulesMap::iterator toDelete = (m_Rules.begin());
        delete toDelete->second;
        m_Rules.erase( toDelete );
    }

	while( m_LinguisticVariables.size() > 0 )
	{
		LinguisticMap::iterator toDelete ( m_LinguisticVariables.begin() );
		delete toDelete->second;
		m_LinguisticVariables.erase( toDelete );
	}
}


void FuzzySystem::addVariable( LinguisticVariable & rVariable )
{
    m_LinguisticVariables.insert( 
		LinguisticPair( rVariable.getName(), new LinguisticVariable( rVariable ) )  );
}

void FuzzySystem::setInput( const std::string & rVarName, double value )
{
    LinguisticMap::iterator iter = m_LinguisticVariables.find( rVarName );
    if( iter == m_LinguisticVariables.end() )
    {
        throw std::exception( "FuzzySystem.setInput: cannot find specified variable!" );
    }

    iter->second->setInput( value );
}

void FuzzySystem::newRule( const std::string & rName, const RuleStack & stack  )
{
	RuleStack * pCopy = ( RuleStack * )stack.copy();

    // check all Clauses and the consistency and also the THEN Clause
    checkConsistency( pCopy, true );

	m_Rules.insert( RulesPair( rName, pCopy ) );
}

void FuzzySystem::checkConsistency( RuleStack * pStack, bool bCheckThenRule )
{

    // check also the then rule
    if( bCheckThenRule )
    {
        checkAndUpdateClause( pStack->m_pThenClause );
    }

	std::vector<IStackable *>::const_iterator iter = pStack->m_stack.begin();

	while( iter != pStack->m_stack.end() )
	{
        // we only have to take care of Clauses within the rule base because they need 
        // information from the whole System
		Clause * pClause = dynamic_cast<Clause *>(*iter);
		if( pClause )
		{
            checkAndUpdateClause( pClause );
		}

		// recursive check of nested rule stacks
		RuleStack * pStack = dynamic_cast<RuleStack *>(*iter);
		if( pStack )
		{
			checkConsistency( pStack );
		}

        // next iteration
        ++ iter;
	}
}

void FuzzySystem::checkAndUpdateClause( Clause * pClause )
{
    LinguisticMap::const_iterator linVarIter = m_LinguisticVariables.find( pClause->getLinguisticVariableName() );
    if( linVarIter == m_LinguisticVariables.end() )
	{
		std::stringstream ex;
		ex	<< "FuzzySystem.checkConsistency: linguistic variable \"" << pClause->getLinguisticVariable() 
			<< "\" not found!";
		printf( ex.str().c_str() );
		throw std::exception( ex.str().c_str() );
	}

	LinguisticVariable::FuzzySetMap::iterator fuzzySetIter 
		= linVarIter->second->m_fuzzySets.find( pClause->getFuzzySetName() );
	if( fuzzySetIter == linVarIter->second->m_fuzzySets.end() )
	{
		std::stringstream ex;
		ex	<< "FuzzySystem.checkConsistency: fuzzy set \"" << pClause->getFuzzySetName() 
			<< "\" not found!";
		printf( ex.str().c_str() );
		throw std::exception( ex.str().c_str() );
	}

    pClause->setLinguisticVariable( linVarIter->second );
}


double FuzzySystem::evaluate( const std::string & var ) const
{
    LinguisticMap::const_iterator linIter = m_LinguisticVariables.find( var );
    if( linIter == m_LinguisticVariables.end() )
    {
        throw std::exception( "FuzzySystem.evaluate: Variable not found!" );
    }
    LinguisticVariable * pEvaluateVar = linIter->second;

    // results and accumulators
    double weightSum = 0, membershipSum = 0;

    // for each rule
    for( RulesMap::const_iterator r = m_Rules.begin();
        r != m_Rules.end(); ++ r )
    {
        // variable name then clause in rule
        if( r->second->isThenRule( var ) )
        {
            const Clause * pThenClause = r->second->m_pThenClause;
            const LinguisticVariable * pThenVar = pThenClause->getLinguisticVariable();
            
			double firingStrength = r->second->evaluate();

            
            LinguisticVariable::FuzzySetMap::const_iterator fuzzyIter
                = pEvaluateVar->m_fuzzySets.find( pThenClause->getFuzzySetName() );

            if( fuzzyIter != pEvaluateVar->m_fuzzySets.end() )
            {
                // speech universe
                double start = pThenVar->m_min;
                double end =   pThenVar->m_max;

                // increment
                double increment = ( end - start ) / this->m_defuzzficationInterval;

                //just for debuging
                double prevWeightSum = weightSum;
                double prevMemberShipSum = membershipSum;

                // running through the speech universe and evaluating the labels at each point
                for( double x = start; x < end; x += increment )
                {
                    // getting the membership for X and constraining it with the firing strength
                    double membership 
                        = pThenVar->getMembership( pThenClause->getFuzzySetName(), x );
                   
                    double constrainedMembership = std::min<double>( membership, firingStrength );

                    weightSum += x * constrainedMembership;
                    membershipSum += constrainedMembership;
                }

                printf( "FuzzySystem.evaluate: fuzzy-set: %s \n( "
                        "\n   firingStrenght: %f; \n   memberShipSum: %f; \n   weightSum: %f \n)\n", 
                    pThenClause->getFuzzySetName().c_str(), firingStrength,
                    membershipSum - prevMemberShipSum, weightSum - prevWeightSum);
            }
        }
    }

    // if no membership was found, then the membershipSum is zero and the numerical output is unknown.
    if ( membershipSum == 0 )
	{
		printf( "\nFuzzySystem.evaluate: The numerical output is unavaliable. All memberships are zero.\n" );
        throw std::exception( "FuzzySystem.evaluate: The numerical output is unavaliable. All memberships are zero." );
		
	}
    return weightSum / membershipSum;

}


/*
void FuzzySystem::newRule( const std::string & rName, const std::string & rRule )
{
    //m_Rules.insert( 
      //  RulesPair( rName, parseRuleString( rRule ) ) );
    
    std::string rule = rRule;
    // To Upper case:
    std::transform( rule.begin(), rule.end(), rule.begin(), toupper );

    // Remove all (double?) white spaces:
    std::remove( rule.begin(), rule.end(), '  ' );
    
    int posIF  = rule.find( "IF" );
    int posEND = rule.find( "THEN" );

    if( std::string::npos == posIF || std::string::npos == posEND )
    {
        throw std::exception( "FuzzySystem.newRule: IF and THEN not found in rule!" );
    }

    RuleStack * stack = parseRuleString( rule.substr( posIF+2, posEND ) );

}

RuleStack * FuzzySystem::parseRuleString( std::string rule )
{
    int pos = std::min<int>( rule.find( "IS" ), 
                std::min<int>( rule.find( "AND" ), 
                    std::min<int>(rule.find( "OR" ),
                        rule.find( "NOT" ) ) ) );


    

    return 0;
}
*/


