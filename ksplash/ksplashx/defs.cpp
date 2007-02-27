#include "defs.h"
#include <string.h>

bool freadline( char* buf, int bufsize, FILE* datafile )
    {
    if( fgets( buf, bufsize, datafile ) == 0 )
        {
        buf[ 0 ] = '\0';
        return false;
        }
    char* nl = strchr( buf, '\n' );
    if( nl != NULL )
        *nl = '\0';
    return true;
    }

void strip_whitespace( char* line )
    {
    char* dst = line;
    char* src = line;
    while( *src == ' ' )
        ++src;
    while( ( *dst++ = *src++ ) != '\0' )
        ;
    --dst;
    while( dst >= line && *dst == ' ' )
        *dst-- = '\0';
    }

bool begins_with( const char* line, const char* str )
    {
    int len = strlen( str );
    return strncmp( line, str, len );
    }
