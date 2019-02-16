#include "..\inc\LogWriter.h"

LogWriter::LogWriter( const string & filename )
:   TableWriter( filename, true ),
    m_bPrint( true )
{
    setColumnHeaders( "s", "Text" );
}

LogWriter::~LogWriter(void)
{
}

void LogWriter::addLine( const std::string & message, bool bPrint )
{
    addRow( bPrint, message.c_str() );
}

void LogWriter::addLine( const char * message, bool bPrint )
{
    addRow( bPrint, message );
}

