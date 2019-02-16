#include "..\inc\LinguisticVariable.h"
#include "FuzzyMemberFunctions.h"

LinguisticVariable::LinguisticVariable( const string & name, double min, double max )
:   m_name( name ),
    m_min( min ),
    m_max( max )
{
}

LinguisticVariable::LinguisticVariable( const LinguisticVariable & var )
:	m_name( var.m_name ),
	m_min(  var.m_min ),
	m_max(  var.m_max )
{
	// copy the fuzzy Sets
	FuzzySetMap::const_iterator iter = var.m_fuzzySets.begin();

	while( iter != var.m_fuzzySets.end() )
	{
		FuzzySetPair pair = (* iter);
		m_fuzzySets.insert( FuzzySetPair( pair.first, pair.second->copy() ) );

		++ iter;
	}
}

LinguisticVariable::~LinguisticVariable(void)
{
    while( m_fuzzySets.size() > 0 )
	{
        FuzzySetMap::iterator toDelete = (m_fuzzySets.begin());
        delete toDelete->second;
        m_fuzzySets.erase( toDelete );
    }
}

void LinguisticVariable::addFuzzySet( const string & label, const IFuzzyMemberFunctions & memberFunction )
{
	m_fuzzySets.insert( FuzzySetPair( label, memberFunction.copy() ) );
}

double LinguisticVariable::getMembership( const string & fuzzySet, double x ) const
{
    FuzzySetMap::const_iterator iter = m_fuzzySets.find( fuzzySet );
    if( iter == m_fuzzySets.end() )
    {
        throw std::exception( "LinguisticVarialbe.fire: the fuzzySet doesn't exist!" );
    }

    return iter->second->getMembership( x );
}

double LinguisticVariable::getMembership( const string & fuzzySet ) const
{
    return getMembership( fuzzySet, m_input );
}