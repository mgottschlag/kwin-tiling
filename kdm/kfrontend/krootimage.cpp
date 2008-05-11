/*

Copyright (C) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2002,2004 Oswald Buddenhagen <ossi@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#include "krootimage.h"

#include <kcmdlineargs.h>
#include <kcomponentdata.h>
#include <klocale.h>
#include <kconfig.h>

#include <QDesktopWidget>
#include <QFile>
#include <QX11Info>

#include <X11/Xlib.h>

#include <stdlib.h>

static const char description[] =
	I18N_NOOP( "Fancy desktop background for kdm" );

static const char version[] = "v2.0";


MyApplication::MyApplication( const char *conf, int argc, char **argv )
	: QApplication( argc, argv )
	, renderer( 0, KSharedConfig::openConfig( QFile::decodeName( conf ) ), true )
{
	connect( &timer, SIGNAL(timeout()), SLOT(slotTimeout()) );
	connect( &renderer, SIGNAL(imageDone( int )), this, SLOT(renderDone()) );
	renderer.enableTiling( true ); // optimize
	renderer.changeWallpaper(); // cannot do it when we're killed, so do it now
	timer.start( 60000 );
	renderer.start();
}


void
MyApplication::renderDone()
{
	QPalette palette;
	palette.setBrush( desktop()->backgroundRole(), QBrush( renderer.pixmap() ) );
	desktop()->setPalette( palette );
	desktop()->setAutoFillBackground( true );
	desktop()->setAttribute( Qt::WA_PaintOnScreen );
	desktop()->show();
	desktop()->repaint();

	renderer.saveCacheFile();
	renderer.cleanup();
	for (unsigned i = 0; i < renderer.numRenderers(); ++i) {
		KBackgroundRenderer *r = renderer.renderer( i );
		if (r->backgroundMode() == KBackgroundSettings::Program ||
		    (r->multiWallpaperMode() != KBackgroundSettings::NoMulti &&
		     r->multiWallpaperMode() != KBackgroundSettings::NoMultiRandom))
			return;
	}
	quit();
}

void
MyApplication::slotTimeout()
{
	bool change = false;

	if (renderer.needProgramUpdate()) {
		renderer.programUpdate();
		change = true;
	}

	if (renderer.needWallpaperChange()) {
		renderer.changeWallpaper();
		change = true;
	}

	if (change)
		renderer.start();
}

int
main( int argc, char *argv[] )
{
	KCmdLineArgs::init( argc, argv, "krootimage", "kdesktop",
	                    ki18n("KRootImage"), version, ki18n(description) );

	KCmdLineOptions options;
	options.add( "+config", ki18n("Name of the configuration file") );
	KCmdLineArgs::addCmdLineOptions( options );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if (!args->count())
		args->usage();
	KComponentData inst( KCmdLineArgs::aboutData() );
	MyApplication app( args->arg( 0 ).toLocal8Bit(),
	                   KCmdLineArgs::qtArgc(), KCmdLineArgs::qtArgv() );
	args->clear();

	app.exec();
	app.flush();

	// Keep color resources after termination
	XSetCloseDownMode( QX11Info::display(), RetainTemporary );

	return 0;
}

#include "krootimage.moc"
