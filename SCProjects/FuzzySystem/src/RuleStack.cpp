#include "RuleStack.h"
#include "NormOperators.h"


RuleStack::RuleStack(void)
:   m_pThenClause( 0 )
{
    //m_stack.
}

RuleStack::RuleStack( const RuleStack & ref )
:   m_pThenClause( 0 )
{
    std::vector<IStackable *>::const_iterator iter = ref.m_stack.begin();

	while( iter != ref.m_stack.end() )
	{
		m_stack.push_back( (*iter)->copy() );
		++ iter;
	}

    //then clause
    if( ref.m_pThenClause )
    {
        m_pThenClause = (Clause *)ref.m_pThenClause->copy();
    }
	
}

RuleStack::~RuleStack(void)
{
    if( m_pThenClause != 0 )
	{
		delete m_pThenClause;
	}
    while( m_stack.size() > 0 )
	{
        IStackable * toDelete = m_stack.back();
        delete toDelete;
        m_stack.pop_back();
    }
	
}

void RuleStack::add( const IStackable & op )
{
    m_stack.push_back( op.copy() );
}

void RuleStack::setThen( const Clause & rClause )
{
	m_pThenClause = (Clause *)rClause.copy();
}


bool RuleStack::isThenRule( const std::string & var ) const
{
    return m_pThenClause->getLinguisticVariableName() == var;
}

double RuleStack::evaluate() const
{
	double nextValue;


	std::vector<IStackable *>::const_iterator iter = m_stack.begin();
	if( iter == m_stack.end() )
		return 0.0;

	IEvaluateable * pEvaluatable = dynamic_cast<IEvaluateable *>(*iter);
	if( !pEvaluatable )
	{
		printf( "Rulestack.evaluate: first element hast to be evaluateable!" );
		throw std::exception( "Rulestack.evaluate: first element hast to be evaluateable!" );
	}
	
	double value = pEvaluatable->evaluate();

	// next element 
	++ iter;

	while( iter != m_stack.end() )
	{
		// operator
        INormOperator * pNormOperator = dynamic_cast<INormOperator *>(*iter);
        if( pNormOperator )
        {
			// next element
            ++ iter;
            if( iter == m_stack.end() )
            {
				printf( "Rulestack.evaluate: after an NormOperator( OR/AND ) a Evaluatable is needed!" );
                throw std::exception( "Rulestack.evaluate: after an NormOperator( OR/AND ) a Evaluatable is needed!" );
            }            
        }

		pEvaluatable = dynamic_cast<IEvaluateable*>(*iter);

		if( pEvaluatable )
		{
			nextValue = pEvaluatable->evaluate();
		}

		// binary operator
		value = pNormOperator->apply( value, nextValue );

		// next element
		++ iter;
	}


	return value;
}


IStackable * RuleStack::copy( void ) const
{
	return new RuleStack( *this );
}