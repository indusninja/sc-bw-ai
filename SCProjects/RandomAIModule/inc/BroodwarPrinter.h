#pragma once
#include <BWAPI.h>

#include "TableWriter.h"

class BroodwarPrinter : public TableWriter::IPrinter 
{
public:
	virtual void print( const char * msg )
	{
		BWAPI::Broodwar->sendText( msg );
	}
};