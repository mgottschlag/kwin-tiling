/*
  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
  Copyright (c) 2000 Matthias Elter <elter@kde.org>

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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

#include <iostream.h>

#include <stdlib.h>
#include <kapp.h>
#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kcmdlineargs.h>
#include <kservice.h>
#include <kdesktopfile.h>

#include "kcdialog.h"
#include "moduleinfo.h"
#include "modloader.h"

static KCmdLineOptions options[] =
{
    { "list", I18N_NOOP("List all possible modules"), 0},
    { "+module", I18N_NOOP("Configuration module to open."), 0 },
    KCmdLineLastOption
};

int main(int _argc, char *_argv[])
{
    KCmdLineArgs::init( _argc, _argv, "kcmshell", I18N_NOOP("A tool to start single kcontrol modules"), "2.0pre" );
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KLocale::setMainCatalogue("kcontrol");
    KApplication app;
    // It has to be unset, if not it will break modules like kcmlocale
    KLocale::setMainCatalogue(0);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("list")) {
	QStringList files;
	KGlobal::dirs()->findAllResources("apps",
					  "Settings/*.desktop",
					  true, true, files);
	QStringList modules;
	QStringList descriptions;
	uint maxwidth = 0;
	for (QStringList::ConstIterator it = files.begin(); it != files.end(); it++) {
	
	    if (KDesktopFile::isDesktopFile(*it)) {
		KDesktopFile file(*it, true);
		QString module = *it;
		if (module.left(9) == "Settings/")
		    module = module.mid(9);
		if (module.right(8) == ".desktop")
		    module.truncate(module.length() - 8);

		modules.append(module);
		if (module.length() > maxwidth)
		    maxwidth = module.length();
		descriptions.append(QString("%2 (%3)").arg(file.readName()).arg(file.readComment()));
	    }
	}
	
	QByteArray vl;
	vl.fill(' ', 80);
	QString verylong = vl;

	for (uint i = 0; i < modules.count(); i++) {
	    cout << (*modules.at(i)).latin1();
	    cout << verylong.left(maxwidth - (*modules.at(i)).length());
	    cout << " - ";
	    cout << (*descriptions.at(i)).local8Bit();
	    cout << endl;
	}

	return 0;
    }

    if (args->count() != 1) {
	args->usage();
	return -1;
    }

    QCString arg = args->arg(0);

    // locate the desktop file
    //QStringList files;
    if (arg[0] == '/')
    {
        kdDebug() << "Full path given to kcmshell - not supported yet" << endl;
        // (because of KService::findServiceByDesktopPath)
	//files.append(args->arg(0));
    }

    QCString path = "Settings/";
    path += arg;
    path += ".desktop";

    if (!KService::serviceByDesktopPath( path ))
    {
        // Path didn't work. Trying as a name
        KService::Ptr serv = KService::serviceByDesktopName( arg );
        if ( serv )
            path = serv->entryPath();
        else
        {
            cerr << i18n("Module %1 not found!").arg(arg) << endl;
            return -1;
        }
    }

    args->clear();

    // load the module
    ModuleInfo info(path);

    KCModule *module = ModuleLoader::loadModule(info);

    if (module) {
	// create the dialog
	KCDialog * dlg = new KCDialog(module, module->buttons(), info.docPath(), 0, 0, true);
	dlg->setCaption(info.name());
	
	// Needed for modules that use d'n'd (not really the right
	// solution for this though, I guess)
	dlg->setAcceptDrops(true);

	// run the dialog
	int ret = dlg->exec();
        delete dlg;
	ModuleLoader::unloadModule(info);
	return ret;
    }

    return 0;
}
