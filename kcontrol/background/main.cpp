/* vi: ts=8 sts=4 sw=4
 * This file is part of the KDE project, module kcmbackground.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * Copyright (C) 2003 Waldo Bastian <bastian@kde.org>
 *
 * Based on old backgnd.cpp:
 *
 * Copyright (c)  Martin R. Jones 1996
 * Converted to a kcc module by Matthias Hoelzer 1997
 * Gradient backgrounds by Mark Donohoe 1997
 * Pattern backgrounds by Stephan Kulow 1998
 * Randomizing & dnd & new display modes by Matej Koss 1998
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <QLayout>
#include <QVBoxLayout>
#include <QByteArray>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include "bgdialog.h"

#include <QtDBus/QtDBus>
#include "main.h"

/* as late as possible, as it includes some X headers without protecting them */
#include <X11/Xlib.h>
#include <QX11Info>
#include <KPluginFactory>
#include <KPluginLoader>

/**** DLL Interface ****/
K_PLUGIN_FACTORY(KBackGndFactory,
        registerPlugin<KBackground>();
        )
K_EXPORT_PLUGIN(KBackGndFactory("kcmbackground"))

/**** KBackground ****/
KBackground::~KBackground( )
{
}

KBackground::KBackground(QWidget *parent, const QVariantList &args)
    : KCModule(KBackGndFactory::componentData(), parent, args)
{
    int screen_number = 0;
    if (QX11Info::display())
	screen_number = DefaultScreen(QX11Info::display());
    QString configname;
    if (screen_number == 0)
	configname = "kdesktoprc";
    else
	configname.sprintf("kdesktop-screen-%drc", screen_number);
    m_pConfig = KSharedConfig::openConfig(configname, KConfig::NoGlobals);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_base = new BGDialog(this, m_pConfig, false);
    setQuickHelp( m_base->quickHelp());
    layout->addWidget(m_base);
    layout->addStretch();

    // reparenting that is done.
    setAcceptDrops(true);

    connect(m_base, SIGNAL(changed(bool)), SIGNAL(changed(bool)));

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmbackground"), 0, ki18n("KDE Background Control Module"),
                  0, KLocalizedString(), KAboutData::License_GPL,
                  ki18n("(c) 1997-2002 Martin R. Jones"));

    about->addAuthor(ki18n("Waldo Bastian"), KLocalizedString(), "bastian@kde.org");
    about->addAuthor(ki18n("George Staikos"), KLocalizedString(), "staikos@kde.org");
    about->addAuthor(ki18n("Martin R. Jones"), KLocalizedString(), "jones@kde.org");
    about->addAuthor(ki18n("Matthias Hoelzer-Kluepfel"), KLocalizedString(), "mhk@kde.org");
    about->addAuthor(ki18n("Stephan Kulow"), KLocalizedString(), "coolo@kde.org");
    about->addAuthor(ki18n("Mark Donohoe"));
    about->addAuthor(ki18n("Matej Koss"));

    setAboutData( about );
}


void KBackground::load()
{
    m_base->load();
}


void KBackground::save()
{
    m_base->save();

    int screen_number = 0;
    if (QX11Info::display())
	screen_number = DefaultScreen(QX11Info::display());
    QString appname;
    if (screen_number == 0)
	appname = "org.kde.kdesktop";
    else 
	appname.sprintf("org.kde.kdesktop-screen-%d", screen_number);
    QDBusInterface kdesktop(appname, "/Background", "org.kde.kdesktop.Background");
    kdesktop.call("configure");
}

void KBackground::defaults()
{
    m_base->defaults();
}

#include "main.moc"
