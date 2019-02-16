#include "TableWriter.h"
#include "LogWriter.h"
#include "StdPrinter.h"

int main() {
   
    StdPrinter printer;

    TableWriter table("testFile.csv" );
    table.setPrinter( &printer );

    table.setColumnHeaders( "sifi", "TestString", "Integer1", "Float", "Integer2" );

    table.addRow( true, "Eine Nachricht", 123, 456.78f, 90 );
    table.addRow( true, "Eine Nachricht", 123, 456.78, 90 );
    table.addRow( true, "Eine Nachricht", 123, 456.78f, 90 );
    table.addRow( true, "Eine Nachricht", 123, 456.78, 90 );


    LogWriter log( "testFile.log" );
    log.setPrinter( &printer );

    log.addLine( "TestZeile, Hier kann ein ziemlich langer Text stehen! Sorry guys, just a habit to write texts in German. :D" );
    log.addLine( "TestZeile, Hier kann ein ziemlich langer Text stehen! Sorry guys, just a habit to write texts in German. :D" );
    log.addLine( "TestZeile, Hier kann ein ziemlich langer Text stehen! Sorry guys, just a habit to write texts in German. :D" );


}