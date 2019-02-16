#pragma once


class Point 
{
public:
	double X;
	double Y;
};

class IFuzzyMemberFunctions
{
public:
	virtual double getMembership( double x ) = 0;
	virtual IFuzzyMemberFunctions * copy( void ) const = 0;
};

class PiecewiseLinearFunction : public IFuzzyMemberFunctions
{
protected:
	int			m_size;
	Point *		m_pPoints;

    virtual void checkConstruction( );
public:
	PiecewiseLinearFunction( const PiecewiseLinearFunction & ref ); // copy constructor
	PiecewiseLinearFunction( int size, Point* pPoints );
	~PiecewiseLinearFunction( );
	virtual double getMembership( double x );
	virtual IFuzzyMemberFunctions * copy( void ) const;

};

class TriangleFunction : public PiecewiseLinearFunction
{
public:
	TriangleFunction( double left, double right );
	//~TriangleFunction( );
};

class TrapezoidalFunction : public PiecewiseLinearFunction
{
public:
    /***************************************
    ----\                          /----
         \                        /    
          \                      /
    _______\____________________/_______
      RightEdge             LeftEdge 
    ****************************************/
	enum EEdgeType{ LeftEdge, RightEdge };
	TrapezoidalFunction( double m1, double m2, double m3, double m4 );
	TrapezoidalFunction( double m1, double m2, EEdgeType edge );
};