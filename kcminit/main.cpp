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

#include <qfile.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <klibloader.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kconfig.h>

#include <X11/Xlib.h>
#include <QX11Info>

static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List modules that are run at startup"), 0 },
    { "+module", I18N_NOOP("Configuration module to run"), 0 },
    KCmdLineLastOption
};

static bool runModule(const QString &libName, KLibLoader *loader, KService::Ptr service)
{
    KLibrary *lib = loader->library(QFile::encodeName(libName));
    if (lib) {
	// get the init_ function
	QString factory = QString("init_%1").arg(service->init());
	void *init = lib->symbol(factory.utf8());
	if (init) {
	    // initialize the module
	    kdDebug(1208) << "Initializing " << libName << ": " << factory << endl;
	    
	    void (*func)() = (void(*)())init;
	    func();
	    return true;
	}
	loader->unloadLibrary(QFile::encodeName(libName));
    }
    return false;
}

extern "C" KDE_EXPORT int kdemain(int argc, char *argv[])
{
  KLocale::setMainCatalog("kcontrol");
  KAboutData aboutData( "kcminit", I18N_NOOP("KCMInit"),
	"",
	I18N_NOOP("KCMInit - runs startups initialization for Control Modules."));

  KCmdLineArgs::init(argc, argv, &aboutData);
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication app;
  KLocale::setMainCatalog(0);

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  QByteArray arg;
  if (args->count() == 1) {
    arg = args->arg(0);
  }

  // get the library loader instance
  KLibLoader *loader = KLibLoader::self();

  KService::List list;

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
    return 0;
  }

  if (!arg.isEmpty()) {

    QString module = QFile::decodeName(arg);
    if (!module.endsWith(".desktop"))
       module += ".desktop";

    KService::Ptr serv = KService::serviceByStorageId( module );
    if ( !serv || serv->library().isEmpty() ||
	 serv->init().isEmpty()) {
      kdError(1208) << i18n("Module %1 not found!").arg(module) << endl;
      return -1;
    } else
      list.append(serv);

  } else {

    // locate the desktop files
    list = KService::allInitServices();

  }

  if ( !kapp->dcopClient()->isAttached() )
    kapp->dcopClient()->attach();

  // This key has no GUI apparently
  KConfig config("kcmdisplayrc", true );
  config.setGroup("X11");
  bool multihead = !config.readBoolEntry( "disableMultihead", false) &&
                    (ScreenCount(QX11Info::display()) > 1);
  // Pass env. var to kdeinit.
  QByteArray name = "KDE_MULTIHEAD";
  QByteArray value = multihead ? "true" : "false";
  QByteArray params;
  QDataStream stream(&params, QIODevice::WriteOnly);

  stream.setVersion(QDataStream::Qt_3_1);
  stream << name << value;
  kapp->dcopClient()->send("klauncher", "klauncher", "setLaunchEnv(QCString,QCString)", params);
  setenv( name, value, 1 ); // apply effect also to itself

  QStringList alreadyInitialized;

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

      QString libName = QString("kcm_%1").arg(library);

      // try to load the library
      if (! alreadyInitialized.contains( libName.ascii() )) {
	  if (!runModule(libName, loader, service)) {
	      libName = QString("libkcm_%1").arg(library);
	      if (! alreadyInitialized.contains( libName.ascii() )) {
		  runModule(libName, loader, service);
		  alreadyInitialized.append( libName.ascii() );
	      }
	  } else 
	      alreadyInitialized.append( libName.ascii() );
      }
  }

  kapp->dcopClient()->send( "ksplash", "", "upAndRunning(QString)",  QString("kcminit"));

  return 0;
}
