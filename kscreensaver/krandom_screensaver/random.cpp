//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1999
//
// This is an extremely simple program that starts a random screensaver.
//

#include <config-workspace.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <QLayout>
#include <QCheckBox>
#include <QWidget>
//Added by qt3to4:
#include <QGridLayout>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <krandomsequence.h>
#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kdialog.h>
#include <kconfig.h>
#include <kservice.h>
#include <kdeversion.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include "kscreensaver_vroot.h"
#include "random.h"
#include <QX11Info>
#include <QFrame>
#include <kservicetypetrader.h>

void usage(char *name)
{
	puts(i18n("Usage: %1 [-setup] [args]\n"
				"Starts a random screen saver.\n"
				"Any arguments (except -setup) are passed on to the screen saver.", name ).toLocal8Bit().data());
}

static const char appName[] = "random";

static const char description[] = I18N_NOOP("Start a random KDE screen saver");

static QString exeFromActionGroup(const QList<KServiceAction>& actions, const char* name)
{
    foreach(const KServiceAction& action, actions) {
        if (action.name() == name)
            return action.exec();
    }
    return QString();
}

//----------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	KCmdLineArgs::init(argc, argv, appName, "kscreensaver", ki18n("Random screen saver"), 
                KDE_VERSION_STRING, ki18n(description));


	KCmdLineOptions options;

	options.add("setup", ki18n("Setup screen saver"));

	options.add("window-id wid", ki18n("Run in the specified XWindow"));

	options.add("root", ki18n("Run in the root XWindow"));

	KCmdLineArgs::addCmdLineOptions(options);

	KApplication app;

	Window windowId = 0;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	if (args->isSet("setup"))
	{
		KRandomSetup setup;
		setup.exec();
		exit(0);
	}

	if (args->isSet("window-id"))
	{
		windowId = args->getOption("window-id").toInt();
	}

	if (args->isSet("root"))
	{
		QX11Info info;
		windowId = RootWindow(QX11Info::display(), info.screen());
	}
	args->clear();
	const KService::List lst = KServiceTypeTrader::self()->query( "ScreenSaver");
        KService::List availableSavers;

	KConfig type("krandom.kssrc", KConfig::NoGlobals);
        const KConfigGroup configGroup = type.group("Settings");
	const bool opengl = configGroup.readEntry("OpenGL", false);
	const bool manipulatescreen = configGroup.readEntry("ManipulateScreen", false);
        // TODO replace this with TryExec=fortune in the desktop files
        const bool fortune = !KStandardDirs::findExe("fortune").isEmpty();
        foreach( const KService::Ptr& service, lst ) {
            //QString file = KStandardDirs::locate("services", service->entryPath());
            //kDebug() << "Looking at " << file;
            const QString saverType = service->property("X-KDE-Type").toString();
            foreach (const QString &type, saverType.split(QLatin1Char(';'))) {
                //kDebug() << "saverTypes is "<< type;
                if (type == QLatin1String("ManipulateScreen")) {
                    if (!manipulatescreen)
                        goto fail;
                } else if (type == QLatin1String("OpenGL")) {
                    if (!opengl)
                        goto fail;
                } else if (type == QLatin1String("Fortune")) {
                    if (!fortune)
                        goto fail;
                }
            }
            availableSavers.append(service);
          fail: ;
        }

	KRandomSequence rnd;
	const int indx = rnd.getLong(availableSavers.count());
        const KService::Ptr service = availableSavers.at(indx);
        const QList<KServiceAction> actions = service->actions();

	QString cmd;
	if (windowId)
            cmd = exeFromActionGroup(actions, "InWindow");
        if (cmd.isEmpty() && windowId == 0)
            cmd = exeFromActionGroup(actions, "Root");
        if (cmd.isEmpty())
            cmd = service->exec();

    QHash<QChar, QString> keyMap;
    keyMap.insert('w', QString::number(windowId));
    const QStringList words = KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(cmd, keyMap));
    if (!words.isEmpty()) {
        QString exeFile = KStandardDirs::findExe(words.first());
        if (!exeFile.isEmpty()) {
            char **sargs = new char *[words.size() + 1];
            int i = 0;
            for (; i < words.size(); i++)
                sargs[i] = qstrdup(words[i].toLocal8Bit().constData());
            sargs[i] = 0;

            execv(exeFile.toLocal8Bit(), sargs);
        }
    }

	// If we end up here then we couldn't start a saver.
	// If we have been supplied a window id or root window then blank it.
	QX11Info info;
	Window win = windowId ? windowId : RootWindow(QX11Info::display(), info.screen());
	XSetWindowBackground(QX11Info::display(), win,
			BlackPixel(QX11Info::display(), info.screen()));
	XClearWindow(QX11Info::display(), win);
}


KRandomSetup::KRandomSetup( QWidget *parent, const char *name )
	: KDialog( parent )
{
  setObjectName( name );
  setModal( true );
  setCaption( i18n( "Setup Random Screen Saver" ) );
  setButtons( Ok | Cancel );

	QFrame *main = new QFrame( this );
  setMainWidget( main );
	QGridLayout *grid = new QGridLayout(main );
        grid->setSpacing( spacingHint() );

	openGL = new QCheckBox( i18n("Use OpenGL screen savers"), main );
	grid->addWidget(openGL, 0, 0);

	manipulateScreen = new QCheckBox(i18n("Use screen savers that manipulate the screen"), main);
	grid->addWidget(manipulateScreen, 1, 0);

	setMinimumSize( sizeHint() );

	KConfig config("krandom.kssrc", KConfig::NoGlobals);
        const KConfigGroup configGroup = config.group("Settings");
	openGL->setChecked(configGroup.readEntry("OpenGL", true));
	manipulateScreen->setChecked(configGroup.readEntry("ManipulateScreen", true));

  connect( this, SIGNAL(okClicked()), SLOT(slotOk()) );
}

void KRandomSetup::slotOk()
{
    KConfig config("krandom.kssrc");
    KConfigGroup configGroup = config.group("Settings");
    configGroup.writeEntry("OpenGL", openGL->isChecked());
    configGroup.writeEntry("ManipulateScreen", manipulateScreen->isChecked());

    accept();
}

#include "random.moc"
