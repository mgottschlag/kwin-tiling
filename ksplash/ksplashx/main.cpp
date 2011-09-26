/********************************************************************

Copyright (C) 2007 Lubos Lunak <l.lunak@kde.org>

Please see file LICENSE for the licensing terms of ksplashx as a whole.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "pixmap.h"
#include "splash.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <math.h>

int screen_number = 0;
int number_of_screens = 1;

static void usage( char* name )
    {
    fprintf( stderr, "Usage: %s <theme> [--test] [--pid]\n", name );
    exit( 1 );
    }

int main( int argc, char* argv[] )
    {
    if( argc != 2 && argc != 3 && argc != 4 )
        usage( argv[ 0 ] );
    bool test = false;
    bool print_pid = false;
    int* cpid;
    for( int i = 2; // 1 is the theme
         i < argc;
         ++i )
        {
        if( strcmp( argv[ i ], "--test" ) == 0 )
            test = true;
        else if( strcmp( argv[ i ], "--pid" ) == 0 )
            print_pid = true;
        else
            usage( argv[ 0 ] );
        }
    const char* theme = argv[ 1 ];
    if( strcmp( theme, "DefaultFullscreen" ) == 0 )
        theme = "Default"; // these are now the same
    int parent_pipe = -1;
    if( print_pid )
        {
        int pipes[ 2 ];
        if( pipe( pipes ) < 0 )
            {
            perror( "pipe()" );
            abort();
            }
        pid_t pid = fork();
        if( pid < 0 )
            {
            perror( "fork()" );
            abort();
            }
        if( pid == 0 )
            { // child
            close( pipes[ 0 ] );
            parent_pipe = pipes[ 1 ];
            close( 0 ); // close stdin,stdout,stderr, otherwise startkde will block
            close( 1 );
            close( 2 );
            Display* dpy = XOpenDisplay(NULL);
            if (!dpy)
                {
                exit(1);
                }
            number_of_screens = ScreenCount(dpy);
            if (number_of_screens > 1)
                {
                cpid = (int*)calloc(number_of_screens, sizeof(int));
                screen_number = DefaultScreen(dpy);
                char* display_name;
                display_name = XDisplayString(dpy);
                int breakpos;
                for (int i = strlen(display_name) - 1; i >= 0; i--)
                    {
                    if (display_name[i] == '.')
                        {
                        breakpos = i;
                        break;
                        }
                    }
                // Calculate the array size: part before the dot + length of the screen
                // string (which is log10 + 1) + 1 for the dot itself + 8 for "DISPLAY=" + \0
                const int envir_len = breakpos + log10(static_cast<double>(number_of_screens)) + 11;
                char *envir = new char[envir_len];
                char *server_name = new char[breakpos + 1];
                strncpy(server_name, display_name, breakpos);
                server_name[breakpos] = '\0';

                XCloseDisplay(dpy);
                dpy = 0;
                for (int i = 0; i < number_of_screens; i++)
                    {
                    if (i != screen_number)
                        {
                        cpid[i] = fork();
                        if (cpid[i] == 0)
                            {
                            screen_number = i;
                            // Break here because we are the child process, we don't
                            // want to fork() anymore
                            break;
                            }
                        }
                    }
                snprintf(envir, envir_len, "DISPLAY=%s.%d", server_name, screen_number);
                putenv(strdup(envir));
                delete[] envir;
                delete[] server_name;
                }
            else
                XCloseDisplay(dpy);
            }
        else
            { // parent
            close( pipes[ 1 ] );
            char buf;
            while( read( pipes[ 0 ], &buf, 1 ) < 0
                && ( errno == EINTR || errno == EAGAIN || errno == ECHILD ))
                ;
            if( print_pid )
                printf( "%d\n", pid );
            return 0;
            }
        }
    if( !openDisplay())
        return 2;
    if( test )
        XSynchronize( qt_xdisplay(), True );
    runSplash( theme, test, parent_pipe );
    closeDisplay();
    if (number_of_screens > 1)
        {
        for (int i = 1; i < number_of_screens; i++)
            kill(cpid[i], SIGTERM);
        }
    }
