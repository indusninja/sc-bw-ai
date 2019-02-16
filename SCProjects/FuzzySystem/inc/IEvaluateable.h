#pragma once

class IEvaluateable
{
public:
	virtual double evaluate() const = 0;
    virtual ~IEvaluateable(){}
};