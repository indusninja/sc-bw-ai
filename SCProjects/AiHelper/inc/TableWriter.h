#pragma once

#include <string.h>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

#define MAX_MESSAGELENGTH 512



class TableWriter
{
public:
    class IPrinter
    {
    public:

        virtual void print( const char * msg ) = 0;
        virtual void print( const string & msg)
        {
            print( msg.c_str() );
        }
    };

protected:
    string      m_filename;
    ofstream    m_fStream;
    time_t      m_startTime;

    char *      m_columnTypes;
    int         m_columnNrs;
    //bool        m_bAppend;
    bool        m_bWriteHeaders;

    IPrinter *  m_pPrinter;

    bool fileExists( const char* fileName );

public:
    TableWriter( const string & filename, bool bAppend = true );
    virtual ~TableWriter(void);

    virtual void setColumnHeaders( const char * types, ... );
    //void setColumnTypes( string types );

    virtual void addRow( bool bPrint, ... );

    virtual void setPrinter( IPrinter * pPrinter )
    {
        m_pPrinter = pPrinter;
    }

    virtual IPrinter * getPrinter( ) const 
    {
        return m_pPrinter;
    }
};
