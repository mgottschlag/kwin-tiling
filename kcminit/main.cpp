/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <config.h>

#include "main.h"

#include <unistd.h>

#include <QFile>
#include <QTimer>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <klibloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <klauncher_iface.h>
#include <QtDBus/QtDBus>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <QX11Info>
#endif


static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List modules that are run at startup"), 0 },
    { "+module", I18N_NOOP("Configuration module to run"), 0 },
    KCmdLineLastOption
};

static int ready[ 2 ];
static bool startup = false;

static void sendReady()
{
  if( ready[ 1 ] == -1 )
    return;
  char c = 0;
  write( ready[ 1 ], &c, 1 );
  close( ready[ 1 ] );
  ready[ 1 ] = -1;
}

static void waitForReady()
{
  char c = 1;
  close( ready[ 1 ] );
  read( ready[ 0 ], &c, 1 );
  close( ready[ 0 ] );
}

bool KCMInit::runModule(const QString &libName, KLibLoader *loader, KService::Ptr service)
{
    KLibrary *lib = loader->library(QFile::encodeName(libName));
    if (lib) {
	// get the init_ function
	QString factory = QString("init_%1").arg(service->init());
	void *init = lib->symbol(factory.toUtf8());
	if (init) {
	    // initialize the module
	    kDebug(1208) << "Initializing " << libName << ": " << factory << endl;

	    void (*func)() = (void(*)())init;
	    func();
	    return true;
	}
	loader->unloadLibrary(QFile::encodeName(libName));
    }
    return false;
}

void KCMInit::runModules( int phase )
{
  KLibLoader *loader = KLibLoader::self();
  // look for X-KDE-Init=... entries
  for(KService::List::Iterator it = list.begin();
      it != list.end();
      ++it) {
      KService::Ptr service = (*it);

      QString library = service->property("X-KDE-Init-Library", QVariant::String).toString();
      if (library.isEmpty())
        library = service->library();

      if (library.isEmpty() || service->init().isEmpty())
	continue; // Skip

      // see ksmserver's README for the description of the phases
      QVariant vphase = service->property("X-KDE-Init-Phase", QVariant::Int );
      int libphase = 1;
      if( vphase.isValid() )
          libphase = vphase.toInt();

      if( phase != -1 && libphase != phase )
          continue;

      QString libName = QString("kcm_%1").arg(library);

      // try to load the library
      if (! alreadyInitialized.contains( libName.toAscii() )) {
	  if (!runModule(libName, loader, service)) {
	      libName = QString("libkcm_%1").arg(library);
	      if (! alreadyInitialized.contains( libName.toAscii() )) {
		  runModule(libName, loader, service);
		  alreadyInitialized.append( libName.toAscii() );
	      }
	  } else
	      alreadyInitialized.append( libName.toAscii() );
      }
  }
}

KCMInit::KCMInit( KCmdLineArgs* args )
{
  QDBus::sessionBus().registerObject("/kcminit", this, QDBusConnection::ExportSlots);
  QByteArray arg;
  if (args->count() == 1) {
    arg = args->arg(0);
  }

  if (args->isSet("list"))
  {
    list = KService::allInitServices();

    for(KService::List::Iterator it = list.begin();
        it != list.end();
        ++it)
    {
      KService::Ptr service = (*it);
      if (service->library().isEmpty() || service->init().isEmpty())
	continue; // Skip
      printf("%s\n", QFile::encodeName(service->desktopEntryName()).data());
    }
    return;
  }

  if (!arg.isEmpty()) {

    QString module = QFile::decodeName(arg);
    if (!module.endsWith(".desktop"))
       module += ".desktop";

    KService::Ptr serv = KService::serviceByStorageId( module );
    if ( !serv || serv->library().isEmpty() ||
	 serv->init().isEmpty()) {
      kError(1208) << i18n("Module %1 not found", module) << endl;
      return;
    } else
      list.append(serv);

  } else {

    // locate the desktop files
    list = KService::allInitServices();

  }
  // This key has no GUI apparently
  KConfig config("kcmdisplayrc", true );
  config.setGroup("X11");
#ifdef Q_WS_X11
  bool multihead = !config.readEntry( "disableMultihead", QVariant(false)).toBool() &&
                    (ScreenCount(QX11Info::display()) > 1);
#else
  bool multihead = false;
#endif
  // Pass env. var to kdeinit.
  QString name = "KDE_MULTIHEAD";
  QString value = multihead ? "true" : "false";
  KToolInvocation::klauncher()->setLaunchEnv(name, value);
  setenv( name.toLatin1().constData(), value.toLatin1().constData(), 1 ); // apply effect also to itself

  if( startup )
  {
     runModules( 0 );
     QDBusInterface ksplash("org.kde.ksplash", "/KSplash", "org.kde.ksplash.KSplash");
     ksplash.call( "upAndRunning", "kcminit" );
     sendReady();
     QTimer::singleShot( 300 * 1000, qApp, SLOT( quit())); // just in case
     qApp->exec(); // wait for runPhase1() and runPhase2()
  }
  else
     runModules( -1 ); // all phases
}

KCMInit::~KCMInit()
{
  sendReady();
}

void KCMInit::runPhase1()
{
  runModules( 1 );
  emit phase1Done();
}

void KCMInit::runPhase2()
{
  runModules( 2 );
  emit phase2Done();
  qApp->exit( 0 );
}

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
  // kdeinit waits for kcminit to finish, but during KDE startup
  // only important kcm's are started very early in the login process,
  // the rest is delayed, so fork and make parent return after the initial phase
  pipe( ready );
  if( fork() != 0 )
  {
      waitForReady();
      return 0;
  }
  close( ready[ 0 ] );

  startup = ( strcmp( argv[ 0 ], "kcminit_startup" ) == 0 ); // started from startkde?

  KLocale::setMainCatalog("kcontrol");
  KAboutData aboutData( "kcminit", I18N_NOOP("KCMInit"),
	"",
	I18N_NOOP("KCMInit - runs startups initialization for Control Modules."));

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;
  KLocale::setMainCatalog(0);
  KCMInit kcminit( KCmdLineArgs::parsedArgs());
  return 0;
}

#include "main.moc"
