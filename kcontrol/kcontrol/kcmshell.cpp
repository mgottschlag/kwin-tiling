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
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/



#include <iostream.h>

#include <stdlib.h>
#include <kapp.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kcmodule.h>
#include <kcmdlineargs.h>
#include <kdesktopfile.h>

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

    // locate the desktop file
    QStringList files;
    if (args->arg(0)[0] == '/')
	files.append(args->arg(0));
    else
	files = KGlobal::dirs()->
	    findAllResources("apps",
			     QString("Settings/%1.desktop").arg(args->arg(0)),
			     true);

    // check the matches
    if (files.count() > 1)
	cerr << i18n("Module name not unique. Taking the first match.") << endl;
    if (files.count() <= 0) {
	cerr << i18n("Module not found!") << endl;
	return -1;
    }

    args->clear();

    // load the module
    ModuleInfo info(files[0]);
    KCModule *module = ModuleLoader::module(info, 0);

    if (module) {
	// create the dialog
	KCDialog dlg(module, info.docPath(), 0, 0, true);
	dlg.setCaption(info.name());
	
	// Needed for modules that use d'n'd (not really the right
	// solution for this though, I guess)
	dlg.setAcceptDrops(true);

	// run the dialog
	return dlg.exec();
    }

    return 0;
}
