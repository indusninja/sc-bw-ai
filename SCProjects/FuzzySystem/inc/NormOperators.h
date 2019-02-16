#pragma once

#include "IStackable.h"

class INormOperator : public IStackable
{
public:
   virtual double apply( double value1, double value2 ) const = 0;
   //virtual INormOperator * copy( void ) const = 0;
};

class ANDOperator : public INormOperator
{
public:
   virtual double apply( double value1, double value2 ) const;
   virtual IStackable * copy( void ) const;
};

class OROperator : public INormOperator
{
public:
   virtual double apply( double value1, double value2 ) const;
   virtual IStackable * copy( void ) const;
};
