/*
    KTip, the KDE Tip Of the Day

	Copyright (c) 2000, Matthias Hoelzer-Kluepfel
	Copyright (c) 2002 Tobias Koenig <tokoe82@yahoo.de>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktip.h>
#include <kuniqueapplication.h>
#include <kwin.h>
#include <stdlib.h>
#include <QtDBus/QtDBus>

static const char description[] = I18N_NOOP("Useful tips");

int main(int argc, char *argv[])
{
	KAboutData aboutData("ktip", I18N_NOOP("KTip"),
				"0.3", description, KAboutData::License_GPL,
				"(c) 1998-2002, KDE Developers");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KUniqueApplication::addCmdLineOptions();

	if (!KUniqueApplication::start())
		exit(-1);

	KUniqueApplication app;

	KTipDialog *tipDialog = new KTipDialog(new KTipDatabase(KStandardDirs::locate("data", QString("kdewizard/tips"))));
	Q_CHECK_PTR(tipDialog);
#ifdef Q_WS_X11
	KWin::setState(tipDialog->winId(), NET::StaysOnTop);
#endif
	tipDialog->setCaption(i18n("Useful Tips"));
	QDBusInterface ksplash("org.kde.ksplash", "/KSplash", "org.kde.ksplash.KSplash");
	ksplash.call("close"); // Close splash screen
	tipDialog->show();

	QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

        return app.exec();
}
