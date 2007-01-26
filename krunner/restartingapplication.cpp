/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <QTimer>

#include <KCrash>

#include <restartingapplication.h>

RestartingApplication::RestartingApplication(Display *display,
                                             Qt::HANDLE visual,
                                             Qt::HANDLE colormap)
    : KUniqueApplication(display, visual, colormap)
{
    if (KCrash::crashHandler() == 0 )
    {
        // this means we've most likely crashed once. so let's see if we
        // stay up for more than 2 minutes time, and if so reset the
        // crash handler since the crash isn't a frequent offender
        QTimer::singleShot(120000, this, SLOT(setCrashHandler()));
    }
    else
    {
        // See if a crash handler was installed. It was if the -nocrashhandler
        // argument was given, but the app eats the kde options so we can't
        // check that directly. If it wasn't, don't install our handler either.
        setCrashHandler();
    }
}

void RestartingApplication::setCrashHandler()
{
    KCrash::setEmergencySaveFunction(RestartingApplication::crashHandler);
}

void RestartingApplication::crashHandler(int)
{
    fprintf(stderr, "krunner: crashHandler called\n");

    sleep(1);
    system("krunner --nocrashhandler &"); // try to restart
}

#include "restartingapplication.moc"
