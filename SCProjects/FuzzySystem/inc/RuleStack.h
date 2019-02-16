#pragma once

#include <vector>
#include "IStackable.h"
#include "IEvaluateable.h"
#include "Clause.h"


/// PREDECLARATIONS ///
class FuzzySystem;

class RuleStack : public IStackable, public IEvaluateable
{
	friend class FuzzySystem;
private:
	static const int			TYPE = 0;
	std::vector<IStackable *>	m_stack;
	Clause *					m_pThenClause;

public:
	RuleStack(void);
    RuleStack( const RuleStack & ref );
	virtual ~RuleStack(void);
    virtual void add( const IStackable & op );
	virtual void setThen( const Clause & rClause );

    bool isThenRule( const std::string & var ) const;

    //virtual bool checkConsistency( FuzzySystem 

	//virtual bool isType( const IStackable& obj ) const { return obj.getType() == TYPE; }

	// OVERWRITE IEvaluateable
	virtual double evaluate() const;

	// OVERWRITE IStackable
	virtual IStackable * copy( void ) const;
};
