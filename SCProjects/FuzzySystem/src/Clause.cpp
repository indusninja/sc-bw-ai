#include "Clause.h"
#include "LinguisticVariable.h"

Clause::Clause( const std::string & linguisticVariable, 
				const std::string & fuzzySet )
:	m_linguisticVariableName( linguisticVariable ),
	m_fuzzySetName( fuzzySet ),
    m_pLinguisticVariable( 0 )
{
}

Clause::Clause( const LinguisticVariable * pLinguisticVariable, 
				const std::string & fuzzySet )
:	m_linguisticVariableName( pLinguisticVariable->getName() ),
	m_fuzzySetName( fuzzySet ),
    m_pLinguisticVariable( pLinguisticVariable )
{
}

Clause::~Clause(void)
{
}

double Clause::evaluate() const
{
    if( m_pLinguisticVariable == 0 )
    {
        throw std::exception( "Clause.evaluate: Pointer m_pLinguisticVariable is 0!" );
    }

    return m_pLinguisticVariable->getMembership( m_fuzzySetName );
}

IStackable * Clause::copy() const
{
    if( m_pLinguisticVariable == 0 )
    {
	    return new Clause( m_linguisticVariableName, m_fuzzySetName );
    }
    else
    {
        return new Clause( m_pLinguisticVariable, m_fuzzySetName );
    }
}

//-----------------------------------------------------------------------------

NotClause::NotClause( const std::string & linguisticVariable, const std::string & fuzzySet )
: Clause( linguisticVariable, fuzzySet )
{
}

NotClause::NotClause( const Clause & rClause )
: Clause( rClause.m_pLinguisticVariable, rClause.m_fuzzySetName )
{
}

double NotClause::evaluate() const
{
	return 1.0 - Clause::evaluate();
}

IStackable * NotClause::copy() const
{
    return new NotClause( *this );
}
