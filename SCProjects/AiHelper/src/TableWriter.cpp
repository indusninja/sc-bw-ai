#include "..\inc\TableWriter.h"
#include <stdarg.h>

TableWriter::TableWriter( const string & filename, bool bAppend ):
m_columnTypes( 0 ),
m_columnNrs( 0 ),
m_pPrinter( 0 )
//m_bAppend( bAppend )
{
    m_filename = filename;

    m_bWriteHeaders = !fileExists( filename.c_str() ) || !bAppend;

    m_fStream.open( filename.c_str(),
        bAppend 
            ? ( ios_base::in | ios_base::out | ios_base::app ) 
            : ( ios_base::out | ios_base::trunc ) );
    //m_fStream.seekp( ios_base::end ); */


    m_startTime = time( NULL );
}

TableWriter::~TableWriter(void)
{
    if( m_fStream.is_open() )
    {
        m_fStream.close();
    }

    if( m_columnTypes )
    {
        delete m_columnTypes;
    }
}

void TableWriter::setColumnHeaders(  const char * types, ... )
{
    //char buffer[MAX_MESSAGELENGTH];

	m_columnNrs = strlen( types );
    m_columnTypes = new char[ m_columnNrs+1 ];
    strcpy_s( m_columnTypes, m_columnNrs+1, types );
    
 
    if( m_bWriteHeaders )
    {
        va_list vl;
        va_start( vl, types );

        // first column is time
        m_fStream << "time (seconds); ";

        for( int i = 0; i < m_columnNrs ; ++ i ) 
        {
           char * s = va_arg( vl, char * );
           m_fStream << s << "; ";
        }

        m_fStream << "\n";
        m_fStream.flush();

        va_end( vl );
    }
}


void TableWriter::addRow( bool bPrint, ... )
{
    char buffer[MAX_MESSAGELENGTH];
    string line;

    sprintf_s( buffer, MAX_MESSAGELENGTH,
                    "%ld; ",  time( NULL ) );
    // first column time
    line.append( buffer );

   

    va_list vl;
    va_start( vl, bPrint );

    // Step through the list.
    for( int i = 0; i < m_columnNrs ; ++ i ) 
    {
        union Printable_t {
         int     i;
         double   f;
         char   *s;
        } Printable;

        switch( m_columnTypes[i] ) {   // Type to expect.
            case 'i':
                Printable.i = va_arg( vl, int );
                sprintf_s( buffer, MAX_MESSAGELENGTH,
                    "%d", Printable.i );
            break;

            case 'f':
                 Printable.f = va_arg( vl, double );
                 sprintf_s( buffer, MAX_MESSAGELENGTH,
                     "%f", Printable.f );
            break;

            case 's':
                Printable.s = va_arg( vl, char * );
                sprintf_s( buffer, MAX_MESSAGELENGTH,
                    "\"%s\"", Printable.s );
            break;

            default:
            break;
        }

        //m_fStream << buffer << "; ";
        line.append( buffer );
        line.append( "; " );
    }

    m_fStream << line.c_str() << "\n";
    m_fStream.flush();

    if( bPrint && m_pPrinter != 0 )
    {
        m_pPrinter->print( line );
    }

    va_end( vl );
}

bool TableWriter::fileExists( const char* fileName )
{
    FILE* fp = NULL;

    if( fopen_s( &fp, fileName, "rb" ) == 0 )
    {
        fclose( fp );
        return true;
    }

    return false;
}

