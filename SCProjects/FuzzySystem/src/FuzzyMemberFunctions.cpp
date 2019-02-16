#include <string.h>
#include <stdexcept>
#include "..\inc\FuzzyMemberFunctions.h"

//using namespace std; 

PiecewiseLinearFunction::PiecewiseLinearFunction( const PiecewiseLinearFunction & ref )
{
	m_size = ref.m_size;
	m_pPoints = new Point[m_size];
	if( m_pPoints != 0 )
	{
		memcpy_s(	m_pPoints,	sizeof(Point) * m_size,
					ref.m_pPoints,	sizeof(Point) * m_size );
	}
}

PiecewiseLinearFunction::PiecewiseLinearFunction( int size, Point* pPoints )
{
	m_size = size;
	m_pPoints = new Point[size];
	if( pPoints != 0 )
	{
		memcpy_s(	m_pPoints,	sizeof(Point)*size,
					pPoints,	sizeof(Point) *size );

        checkConstruction( );
    }
}


PiecewiseLinearFunction::~PiecewiseLinearFunction( )
{
	delete m_pPoints;
}

void PiecewiseLinearFunction::checkConstruction( )
{
    // check if X points are in a sequence and if Y values are in [0..1] range
    for ( int i = 0, n = m_size; i < n; i++ )
    {
        if ( ( m_pPoints[i].Y < 0 ) || ( m_pPoints[i].Y > 1 ) )
			throw std::exception( "Y value of points must be in the range of [0, 1]." );

        if ( i == 0 )
            continue;

        if ( m_pPoints[i - 1].X > m_pPoints[i].X )
			throw std::exception( "Points must be in crescent order on X axis." );
    }
}


double PiecewiseLinearFunction::getMembership( double x )
{
    // no values belong to the fuzzy set, if there are no points in the piecewise function
    if ( m_size == 0 )
        return 0.0;

    // if X value is less than the first point, so first point's Y will be returned as membership
    if ( x < m_pPoints[0].X )
        return m_pPoints[0].Y;

    // looking for the line that contais the X value
    for ( int i = 1, n = m_size; i < n; i++ )
    {
        // the line with X value starts in points[i-1].X and ends at points[i].X
        if ( x < m_pPoints[i].X )
        {
            // points to calculate line's equation
            double y1 = m_pPoints[i].Y;
            double y0 = m_pPoints[i - 1].Y;
            double x1 = m_pPoints[i].X;
            double x0 = m_pPoints[i - 1].X;
            // angular coefficient
            double m = ( y1 - y0 ) / ( x1 - x0 );
            // returning the membership - the Y value for this X
            return m * ( x - x0 ) + y0;
        }
    }

    // X value is more than last point, so last point Y will be returned as membership
    return m_pPoints[ m_size - 1].Y;
}

IFuzzyMemberFunctions * PiecewiseLinearFunction::copy( void ) const
{
	return new PiecewiseLinearFunction( *this );
}


TriangleFunction::TriangleFunction( double left, double right )
: PiecewiseLinearFunction( 3, 0 )
{	
	m_pPoints[0].X = left;	
	m_pPoints[0].Y = 0;

	m_pPoints[1].X = (left+right)/2;	
	m_pPoints[1].Y = 1.0;

	m_pPoints[2].X = right;	
	m_pPoints[2].Y = 0;


    // check consistency within PieceWiseLinearFunction
    checkConstruction( );

}


TrapezoidalFunction::TrapezoidalFunction( double m1, double m2, double m3, double m4 )
: PiecewiseLinearFunction( 4, 0 )
{
	m_pPoints[0].X = m1;	
	m_pPoints[0].Y = 0;

	m_pPoints[1].X = m2;	
	m_pPoints[1].Y = 1.0;

	m_pPoints[2].X = m3;	
	m_pPoints[2].Y = 1.0;

	m_pPoints[3].X = m4;	
	m_pPoints[3].Y = 0;


    // check consistency within PieceWiseLinearFunction
    checkConstruction( );
}
TrapezoidalFunction::TrapezoidalFunction( double m1, double m2, EEdgeType edge )
: PiecewiseLinearFunction( 2, 0 )
{
	if( edge == LeftEdge )
	{
		m_pPoints[0].X = m1;	
		m_pPoints[0].Y = 0;

		m_pPoints[1].X = m2;	
		m_pPoints[1].Y = 1.0;
	}
	else
    {
        m_pPoints[0].X = m1;	
		m_pPoints[0].Y = 1.0;

		m_pPoints[1].X = m2;	
		m_pPoints[1].Y = 0.0;
    }

	// check consistency within PieceWiseLinearFunction
    checkConstruction( );
}
