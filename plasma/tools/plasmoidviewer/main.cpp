/*
 * Copyright 2007 Frerich Raabe <raabe@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "fullview.h"

#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/widgets/layout.h>

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KIconLoader>
#include <KLocale>
#include <KMessageBox>
#include <KStandardDirs>

#include <QIcon>

using namespace Plasma;

static const char description[] = I18N_NOOP( "Run Plasma applets in their own window" );
static const char version[] = "1.0";

int main(int argc, char **argv)
{
    KAboutData aboutData( "plasmoidviewer", 0, ki18n( "Plasma Applet Viewer" ),
                          version, ki18n( description ), KAboutData::License_BSD,
                         ki18n( "(C) 2007, The KDE Team" ) );
    aboutData.addAuthor( ki18n( "Frerich Raabe" ),
                         ki18n( "Original author" ),
                        "raabe@kde.org" );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add( "+applet", ki18n( "Name of applet to add" ) );
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs() ;
    if ( args->count() == 0 ) {
        return 1;
    }

    Corona corona;
    corona.setBackgroundBrush( QPixmap( KStandardDirs::locate( "appdata", "checker.png" ) ) );
    Containment *containment = corona.addContainment( "null" );

    Applet *applet = containment->addApplet( args->arg( 0 ) );
    if (applet->failedToLaunch()) {
        // XXX Can we give a better error message somehow?
        applet->setFailedToLaunch(true, i18n( "Failed to load applet '%1'.", args->arg(0)));
    }

    // An Applet::setPosition call which takes the border width (if any) into
    // account and then calls QGraphicsItem::setPos would be nice here.
    const qreal borderWidth = ( applet->sizeHint().width() - applet->contentSizeHint().width() ) / 2;
    applet->setPos( borderWidth, borderWidth );
    applet->setFlag( QGraphicsItem::ItemIsMovable, false );

    FullView view( &corona );
    view.setWindowTitle( applet->name() );
    view.setWindowIcon( SmallIcon( applet->icon() ) );
    view.show();

    return app.exec();
}

