#include <algorithm>

#include "NormOperators.h"


double ANDOperator::apply( double value1, double value2 ) const
{
    return std::min<double>( value1, value2 );
}

IStackable * ANDOperator::copy( void ) const
{
    return new ANDOperator();
}


double OROperator::apply( double value1, double value2 ) const
{
    return std::max<double>( value1, value2 );
}

IStackable * OROperator::copy( void ) const
{
    return new OROperator();
}
