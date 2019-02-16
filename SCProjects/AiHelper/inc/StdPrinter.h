#pragma once

#include "TableWriter.h"

class StdPrinter : public TableWriter::IPrinter
    {
    public:

        virtual void print( const char * msg )
        {
            printf( "%s\n", msg );
        }
    };
