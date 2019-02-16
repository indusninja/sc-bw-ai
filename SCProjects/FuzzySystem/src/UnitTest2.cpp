#include <cstdio>

#include "LinguisticVariable.h"
#include "RuleStack.h"
#include "Clause.h"
#include "FuzzyMemberFunctions.h"
#include "FuzzySystem.h"
#include "NormOperators.h"

int main() {
	try{

		FuzzySystem IS;
		{
			/*

			 FuzzySet fsNear = new FuzzySet( "Near", new TrapezoidalFunction( 15, 50, TrapezoidalFunction.EdgeType.Right ) );
				FuzzySet fsMedium = new FuzzySet( "Medium", new TrapezoidalFunction( 15, 50, 60, 100 ) );
				FuzzySet fsFar = new FuzzySet( "Far", new TrapezoidalFunction( 60, 100, TrapezoidalFunction.EdgeType.Left ) );
				*/

			// linguistic labels (fuzzy sets) that compose the distances
			LinguisticVariable lvFront( "FrontalDistance", 0, 120 );
			lvFront.addFuzzySet( "Near",	TrapezoidalFunction( 15, 50, TrapezoidalFunction::RightEdge ) );
			lvFront.addFuzzySet( "Medium",	TrapezoidalFunction( 15, 50, 60, 100 ) );
			lvFront.addFuzzySet( "Far",		TrapezoidalFunction( 60, 100, TrapezoidalFunction::LeftEdge ) );

			LinguisticVariable lvRight( "RightDistance", 0, 120 );
			lvRight.addFuzzySet( "Near",	TrapezoidalFunction( 15, 50, TrapezoidalFunction::RightEdge ) );
			lvRight.addFuzzySet( "Medium",	TrapezoidalFunction( 15, 50, 60, 100 ) );
			lvRight.addFuzzySet( "Far",		TrapezoidalFunction( 60, 100, TrapezoidalFunction::LeftEdge ) );

			LinguisticVariable lvLeft( "LeftDistance", 0, 120 );
			lvLeft.addFuzzySet( "Near",	    TrapezoidalFunction( 15, 50, TrapezoidalFunction::RightEdge ) );
			lvLeft.addFuzzySet( "Medium",	TrapezoidalFunction( 15, 50, 60, 100 ) );
			lvLeft.addFuzzySet( "Far",		TrapezoidalFunction( 60, 100, TrapezoidalFunction::LeftEdge ) );


			/*
			// Linguistic labels (fuzzy sets) that compose the angle
				FuzzySet fsVN = new FuzzySet( "VeryNegative", new TrapezoidalFunction( -40, -35, TrapezoidalFunction.EdgeType.Right ) );
				FuzzySet fsN = new FuzzySet( "Negative", new TrapezoidalFunction( -40, -35, -25, -20 ) );
				FuzzySet fsLN = new FuzzySet( "LittleNegative", new TrapezoidalFunction( -25, -20, -10, -5 ) );
				FuzzySet fsZero = new FuzzySet( "Zero", new TrapezoidalFunction( -10, 5, 5, 10 ) );
				FuzzySet fsLP = new FuzzySet( "LittlePositive", new TrapezoidalFunction( 5, 10, 20, 25 ) );
				FuzzySet fsP = new FuzzySet( "Positive", new TrapezoidalFunction( 20, 25, 35, 40 ) );
				FuzzySet fsVP = new FuzzySet( "VeryPositive", new TrapezoidalFunction( 35, 40, TrapezoidalFunction.EdgeType.Left ) );
			*/

			// linguistic labels (fuzzy sets) that compose the angle
			LinguisticVariable lvAngle( "Angle", -50, 50 );
			lvAngle.addFuzzySet( "VeryNegative", TrapezoidalFunction( -40, -35, TrapezoidalFunction::RightEdge ) );
			lvAngle.addFuzzySet( "Negative", TrapezoidalFunction( -40, -35, -25, -20 ) );
			lvAngle.addFuzzySet( "LittleNegative", TrapezoidalFunction( -25, -20, -10, -5 ) );
			lvAngle.addFuzzySet( "Zero", TrapezoidalFunction( -10, 5, 5, 10 ) );
			lvAngle.addFuzzySet( "LittlePositive", TrapezoidalFunction( 5, 10, 20, 25 ) );
			lvAngle.addFuzzySet( "Positive", TrapezoidalFunction( 20, 25, 35, 40 ) );
			lvAngle.addFuzzySet( "VeryPositive", TrapezoidalFunction( 35, 40, TrapezoidalFunction::LeftEdge ) );

			IS.addVariable( lvAngle );
			IS.addVariable( lvFront );
			IS.addVariable( lvLeft );
			IS.addVariable( lvRight );
		}


		/*
		 // Going Straight
            IS.NewRule( "Rule 1", "IF FrontalDistance IS Far THEN Angle IS Zero" );
            // Going Straight (if can go anywhere)
            IS.NewRule( "Rule 2", "IF FrontalDistance IS Far AND RightDistance IS Far AND LeftDistance IS Far THEN Angle IS Zero" );
            // Near right wall
            IS.NewRule( "Rule 3", "IF RightDistance IS Near AND LeftDistance IS Not Near THEN Angle IS LittleNegative" );
            // Near left wall
            IS.NewRule("Rule 4", "IF RightDistance IS Not Near AND LeftDistance IS Near THEN Angle IS LittlePositive");
            // Near front wall - room at right
		*/   

		{
			// RULES:
			RuleStack rule1;
			rule1.add( Clause( "FrontalDistance", "Far" ) );
			rule1.setThen( Clause( "Angle", "Zero" ) );

			RuleStack rule2;
			rule2.add( Clause( "FrontalDistance", "Far" ) );
			rule2.add( ANDOperator() );
			rule2.add( Clause( "RightDistance", "Far" ) );
			rule2.add( ANDOperator() );
			rule2.add( Clause( "LeftDistance", "Far" ) );
			rule2.setThen( Clause( "Angle", "Zero" ) );

			RuleStack rule3;
			rule3.add( Clause( "RightDistance", "Near" ) );
			rule3.add( ANDOperator() );
			rule3.add( NotClause( "LeftDistance", "Near" ) );
			rule3.setThen( Clause( "Angle", "LittleNegative" ) );

			//"IF RightDistance IS Not Near AND LeftDistance IS Near THEN Angle IS LittlePositive"
			RuleStack rule4;
			rule4.add( Clause( "LeftDistance", "Near" ) );
			rule4.add( ANDOperator() );
			rule4.add( NotClause( "RightDistance", "Near" ) );
			rule4.setThen( Clause( "Angle", "LittlePositive" ) );

			/*
				IS.NewRule( "Rule 5", "IF RightDistance IS Far AND FrontalDistance IS Near THEN Angle IS Positive" );
				// Near front wall - room at left
				IS.NewRule( "Rule 6", "IF LeftDistance IS Far AND FrontalDistance IS Near THEN Angle IS Negative" );
				// Near front wall - room at both sides - go right
				IS.NewRule( "Rule 7", "IF RightDistance IS Far AND LeftDistance IS Far AND FrontalDistance IS Near THEN Angle IS Positive" );
			*/ 
			RuleStack rule5;
			rule5.add( Clause( "RightDistance", "Far" ) );
			rule5.add( ANDOperator() );
			rule5.add( Clause( "FrontalDistance", "Near" ) );
			rule5.setThen( Clause( "Angle", "Positive" ) );

			RuleStack rule6;
			rule6.add( Clause( "LeftDistance", "Far" ) );
			rule6.add( ANDOperator() );
			rule6.add( Clause( "FrontalDistance", "Near" ) );
			rule6.setThen( Clause( "Angle", "Negative" ) );

			RuleStack rule7;
			rule7.add( Clause( "RightDistance", "Far" ) );
			rule7.add( ANDOperator() );
			rule7.add( Clause( "LeftDistance", "Far" ) );
			rule7.add( ANDOperator() );
			rule7.add( Clause( "FrontalDistance", "Near" ) );
			rule7.setThen( Clause( "Angle", "Positive" ) );


	        
			IS.newRule( "Rule 1", rule1 );
			IS.newRule( "Rule 2", rule2 );
			IS.newRule( "Rule 3", rule3 );
			IS.newRule( "Rule 4", rule4 );
			IS.newRule( "Rule 5", rule5 ); // */
			IS.newRule( "Rule 6", rule6 );
			IS.newRule( "Rule 7", rule7 );
		}
		// going Straight
		//IS.newRule( "Rule 1", "IF FrontalDistance IS Far THEN Angle IS Zero" );
		// Turning Left
		//IS.newRule( "Rule 2", "IF FrontalDistance IS Near THEN Angle IS Positive" );

		/*
		front: 433; left: 50; right: 422; -> angle: 1,67
		front: 429; left: 50; right: 422; -> angle: 1,67
		front: 426; left: 50; right: 423; -> angle: 1,67
		front: 424; left: 51; right: 416; -> angle: 1,67
		front: 306; left: 50; right: 392; -> angle: 1,67

		// */
        double inputs[] = { 433, 50, 422 };

		IS.setInput( "FrontalDistance",   inputs[0] );
		IS.setInput( "LeftDistance",      inputs[1] );
		IS.setInput( "RightDistance",     inputs[2] );

		double result = IS.evaluate( "Angle" );

		printf( "front: %f; left: %f; right: %f; -> angle: %f\n", inputs[0], inputs[1], inputs[2], result );
	}
	catch( exception& e )
	{
		printf( "Exeption caught: %s", e.what() );
	}
	// */

}


/**** some results 

	frontal: 429
left: 50
right: 422
-> angle: 1,67

----------------

frontal: 52
left: 63
right: 68
-> angle: 0,18

----------------

frontal: 76
left: 55
right: 33
-> angle: -15,00

----------------

frontal: 50
left: 59
right: 67
-> angle: 10,00

----------------

frontal: 135
left: 35
right: 193
-> angle: 7,81



=====================================
just tests with rule nr 6
=====================================
frontal:     39
left:        74
right:       26
->angle:    -30

*****/
