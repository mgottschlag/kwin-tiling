/* This file is part of the KDE project
   Copyright (C) ???
   Copyright (C) 2002 Oswald Buddenhagen <ossi@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kcmdlineargs.h>
#include <ksimpleconfig.h>
#include <klocale.h>

#include <qfile.h>

#include "krootimage.h"

#include <X11/Xlib.h>  

#include <stdlib.h>

static const char *description = 
	I18N_NOOP("Fancy desktop background for kdm");

static const char *version = "v1.5";

static KCmdLineOptions options[] = {
    { "+config", I18N_NOOP("Name of the configuration file"), 0 }
};


MyApplication::MyApplication( const char *conf )
  : KApplication(),
    renderer( 0, new KSimpleConfig( QFile::decodeName( conf ) ) )
{
  connect( &renderer, SIGNAL(imageDone(int)), this, SLOT(renderDone()) );
  renderer.start();
}


void MyApplication::renderDone()
{
  desktop()->setBackgroundPixmap( *renderer.pixmap() );
  desktop()->repaint( true );
  quit();
}


int main(int argc, char *argv[])
{
  if (!getenv( "HOME" ))
    setenv( "HOME", "/tmp", 1 );	/* for QSettings */
  KApplication::disableAutoDcopRegistration();

  KLocale::setMainCatalogue( "kdesktop" );
  KCmdLineArgs::init( argc, argv, "krootimage", description, version );
  KCmdLineArgs::addCmdLineOptions( options );

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (!args->count())
    args->usage();
  MyApplication app( args->arg( 0 ) );
  args->clear();
  
  // Keep color resources after termination
  XSetCloseDownMode( qt_xdisplay(), RetainTemporary );

  app.exec();

  app.flushX();
  
  return 0;
}

#include "krootimage.moc"
