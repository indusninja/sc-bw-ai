#include <cstdio>

#include "LinguisticVariable.h"
#include "RuleStack.h"
#include "Clause.h"
#include "FuzzyMemberFunctions.h"
#include "FuzzySystem.h"

int main() {
	try{
		// linguistic labels (fuzzy sets) that compose the distances
		LinguisticVariable lvFront( "FrontalDistance", 0, 100 );
		lvFront.addFuzzySet( "Near", TriangleFunction( -50, 10 ) );
		lvFront.addFuzzySet( "Medium", TriangleFunction( 0, 100 ) );
		lvFront.addFuzzySet( "Far", TriangleFunction( 50, 150 ) );

		// linguistic labels (fuzzy sets) that compose the angle
		LinguisticVariable lvAngle( "Angle", 0, 40 );
		lvAngle.addFuzzySet( "Zero", TriangleFunction( -10, 10 ) );
		//lvAngle.addFuzzySet( "LittlePositive", TriangleFunction( 5, 25 ) );
		lvAngle.addFuzzySet( "Positive", TriangleFunction( 20, 40 ) );
		//lvAngle.addFuzzySet( "VeryPositive", TriangleFunction( 35, 45 ) );



		FuzzySystem IS;

		IS.addVariable( lvAngle );
		IS.addVariable( lvFront );


		/*
		// the database
		Database fuzzyDB = new Database( );
		fuzzyDB.AddVariable( lvFront );
		fuzzyDB.AddVariable( lvAngle );

		// creating the inference system
		InferenceSystem IS = new InferenceSystem( fuzzyDB, new CentroidDefuzzifier( 1
		*/    


		// RULES:
		RuleStack rule1Stack;
		rule1Stack.add( Clause( "FrontalDistance", "Far" ) );
		rule1Stack.setThen( Clause( "Angle", "Zero" ) );

		RuleStack rule2;
		rule2.add( Clause( "FrontalDistance", "Near" ) );
		rule2.setThen( Clause( "Angle", "Positive" ) );



		IS.newRule( "Rule 1", rule1Stack );
		IS.newRule( "Rule 2", rule2 );
		// going Straight
		//IS.newRule( "Rule 1", "IF FrontalDistance IS Far THEN Angle IS Zero" );
		// Turning Left
		//IS.newRule( "Rule 2", "IF FrontalDistance IS Near THEN Angle IS Positive" );

		// */

		IS.setInput( "FrontalDistance", 10 );


		double result = IS.evaluate( "Angle" );

		printf( "Result: %f\n", result );
	}
	catch( exception& e )
	{
		printf( "Exeption caught: %s", e.what() );
	}

}
