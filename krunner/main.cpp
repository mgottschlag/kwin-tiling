/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2006 Zack Rusin <zack@kde.org>
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

#include <KAboutData>
#include <KCmdLineArgs>
#include <kdebug.h>
#include <KLocale>

#include "krunnerapp.h"
#include "interface.h"
#include "saverengine.h"
#include "startupid.h"
#include "kscreensaversettings.h" // contains screen saver config
#include "klaunchsettings.h" // contains startup config


#include <X11/extensions/Xrender.h>

static const char description[] = I18N_NOOP("KDE run command interface");
static const char version[] = "0.1";

int main(int argc, char* argv[])
{
    // thanks to zack rusin and frederik for pointing me in the right direction
    // for the following bits of X11 code
    Display *dpy = XOpenDisplay(0); // open default display
    if (!dpy)
    {
        qWarning("Cannot connect to the X server");
        exit(1);
    }

    bool argbVisual = false ;
    bool haveCompManager = !XGetSelectionOwner(dpy,
                                               XInternAtom(dpy,
                                                           "_NET_WM_CM_S0",
                                                           false)
                                               );
    Colormap colormap = 0;
    Visual *visual = 0;

    kDebug() << "haveCompManager: " << haveCompManager << endl;

    if (haveCompManager)
    {
        int screen = DefaultScreen(dpy);
        int eventBase, errorBase;

        if (XRenderQueryExtension(dpy, &eventBase, &errorBase))
        {
            int nvi;
            XVisualInfo templ;
            templ.screen  = screen;
            templ.depth   = 32;
            templ.c_class = TrueColor;
            XVisualInfo *xvi = XGetVisualInfo(dpy, VisualScreenMask |
                                                   VisualDepthMask |
                                                   VisualClassMask,
                                              &templ, &nvi);
            for (int i = 0; i < nvi; ++i)
            {
                XRenderPictFormat *format = XRenderFindVisualFormat(dpy,
                                                                    xvi[i].visual);
                if (format->type == PictTypeDirect && format->direct.alphaMask)
                {
                    visual = xvi[i].visual;
                    colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
                                               visual, AllocNone);
                    argbVisual = true;
                    break;
                }
            }
        }
    }

    KAboutData aboutData( "krunner", I18N_NOOP("Run Command Interface"),
                          version, description, KAboutData::License_GPL,
                          I18N_NOOP("(c) 2006, Aaron Seigo") );
    aboutData.addAuthor("Aaron J. Seigo",
                        I18N_NOOP("Author and maintainer"),
                        "aseigo@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KRunnerApp app(dpy, Qt::HANDLE(visual), Qt::HANDLE(colormap));

    kDebug() << "Attempting to create KScreenSaverSettings::instance()" << endl;
    KScreenSaverSettings::instance("kscreensaverrc");
    kDebug() << "Created KScreenSaverSettings::instance()" << endl;

    // LOCKING
    SaverEngine saver;

    // Startup stuff ported from kdesktop
    KLaunchSettings::self()->readConfig();
    StartupId *startup_id( NULL );
    if( !KLaunchSettings::busyCursor() )
    {
        delete startup_id;
        startup_id = NULL;
    }
    else
    {
        if( startup_id == NULL )
            startup_id = new StartupId;
        startup_id->configure();
    }

    Interface interface;

    QObject::connect( &app, SIGNAL( showInterface() ), &interface, SLOT( display() ) );

    return app.exec();
}
