/*

Copyright (C) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2002,2004 Oswald Buddenhagen <ossi@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public
License version 2 as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.	 If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.

*/

#include <config.h>

#include <kcmdlineargs.h>
#include <ksimpleconfig.h>
#include <qdesktopwidget.h>
#include <klocale.h>

#include <qfile.h>

#include "krootimage.h"

#include <X11/Xlib.h>

#include <stdlib.h>
#include <QX11Info>

static const char description[] =
	I18N_NOOP( "Fancy desktop background for kdm" );

static const char version[] = "v2.0";

static KCmdLineOptions options[] = {
	{ "+config", I18N_NOOP( "Name of the configuration file" ), 0 },
	KCmdLineLastOption
};


MyApplication::MyApplication( const char *conf )
	: KApplication(),
	  renderer( 0, new KSimpleConfig( QFile::decodeName( conf ) ) )
{
	connect( &timer, SIGNAL(timeout()), SLOT(slotTimeout()) );
	connect( &renderer, SIGNAL(imageDone( int )), this, SLOT(renderDone()) );
	timer.start( 60000 );
	renderer.start();
}


void
MyApplication::renderDone()
{
	desktop()->setBackgroundPixmap( *renderer.pixmap() );
	desktop()->repaint( true );
	for (unsigned i=0; i<renderer.numRenderers(); ++i)
	{
		KBackgroundRenderer * r = renderer.renderer(i);
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
	KApplication::disableAutoDcopRegistration();

	KLocale::setMainCatalog( "kdesktop" );
	KCmdLineArgs::init( argc, argv, "krootimage", I18N_NOOP( "KRootImage" ), description, version );
	KCmdLineArgs::addCmdLineOptions( options );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if (!args->count())
		args->usage();
	MyApplication app( args->arg( 0 ) );
	args->clear();

	app.exec();

	app.flush();

	// Keep color resources after termination
	XSetCloseDownMode( QX11Info::display(), RetainTemporary );

	return 0;
}

#include "krootimage.moc"
