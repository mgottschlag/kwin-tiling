/*
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "SplashApp.h"

#include <iostream>
#include <X11/Xlib.h>

int main(int argc, char **argv)
{
    // lets fork and all that...

#ifndef SPLASH_TEST
    pid_t pid = fork();
    if (pid < -1) {
        return -1;
    }

    if (pid != 0) {
        // this is the parent process, returning pid of the fork
        // printf("%d\n", pid);
        return 0;
    }

    // close stdin,stdout,stderr, otherwise startkde will block
    close(0);
    close(1);
    close(2);
#endif

    Display * display = XOpenDisplay(NULL);

    SplashApp app(display, argc, argv);

    return app.exec();
}

