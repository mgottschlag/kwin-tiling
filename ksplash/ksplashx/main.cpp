#include "pixmap.h"
#include "splash.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>

static void usage( char* name )
    {
    fprintf( stderr, "Usage: %s <theme> [--test]\n", name );
    exit( 1 );
    }

int main( int argc, char* argv[] )
    {
    if( argc != 2 && argc != 3 )
        usage( argv[ 0 ] );
    bool test = false;
    if( argc == 3 )
        {
        if( strcmp( argv[ 2 ], "--test" ) == 0 )
            test = true;
        else
            usage( argv[ 0 ] );
        }
    char* theme = argv[ 1 ];
    int parent_pipe = -1;
    if( !test )
        {
        int pipes[ 2 ];
        if( pipe( pipes ) < 0 )
            {
            perror( "pipe()" );
            abort();
            }
        int f = fork();
        if( f < 0 )
            {
            perror( "fork()" );
            abort();
            }
        if( f == 0 )
            { // child
            close( pipes[ 0 ] );
            parent_pipe = pipes[ 1 ];
            }
        else
            {
            close( pipes[ 1 ] );
            char buf;
            while( read( pipes[ 0 ], &buf, 1 ) < 0
                && ( errno == EINTR || errno == EAGAIN || errno == ECHILD ))
                ;
            return 0;
            }
        }
    if( !openDisplay())
        return 2;
    if( test )
        XSynchronize( qt_xdisplay(), True );
    runSplash( theme, test, parent_pipe );
    closeDisplay();
    }
