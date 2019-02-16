#pragma once
#include "Tablewriter.h"

class LogWriter :
    protected TableWriter
{
private:
    bool    m_bPrint;
public:
    LogWriter( const string & filename );
    virtual ~LogWriter(void);

    void addLine( const string & message, bool bPrint = true );
    void addLine( const char * msg, bool bPrint = true );

    virtual void setPrinter( IPrinter * pPrinter )
    {
        TableWriter::setPrinter( pPrinter );
    }

    virtual IPrinter * getPrinter( ) const 
    {
        return TableWriter::getPrinter();
    }
};
