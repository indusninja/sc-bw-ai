#pragma once

#include "IEvaluateable.h"
#include "IStackable.h"

#include <string>

/// PREDECLARATIONS ////
class LinguisticVariable;

class Clause : public IEvaluateable, public IStackable
{
    friend class FuzzySystem;
    friend class NotClause;

protected:
	std::string		        m_linguisticVariableName;
	std::string		        m_fuzzySetName;

    const LinguisticVariable *    m_pLinguisticVariable;
    Clause( const LinguisticVariable * pLinguisticVariable, const std::string & fuzzySet );
	

    const LinguisticVariable * getLinguisticVariable( ) const
    { return m_pLinguisticVariable; }

    void setLinguisticVariable( const LinguisticVariable * var )
    { m_pLinguisticVariable = var; }

public:
	Clause( const std::string & linguisticVariable, const std::string & fuzzySet );
	virtual ~Clause(void);

	const std::string & getLinguisticVariableName( ) const
	{ return m_linguisticVariableName; }

	const std::string & getFuzzySetName( ) const
	{ return m_fuzzySetName; }

	// OVERWRITE: IEvaluateable
	virtual double evaluate() const;

	// OVERWRITE: IStackable
	virtual IStackable * copy() const;
};

class NotClause : public Clause
{
public:
	NotClause( const std::string & linguisticVariable, const std::string & fuzzySet );
    NotClause( const Clause & rClause );
	
	// OVERWRITE: IEvaluateable
	virtual double evaluate() const;

    // OVERWRITE: IStackable
	virtual IStackable * copy() const;

};
