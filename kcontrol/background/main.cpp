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

#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <Q3CString>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kimageio.h>
#include <kgenericfactory.h>
#include <kdialog.h>
#include "bgdialog.h"

#include "main.h"

/* as late as possible, as it includes some X headers without protecting them */
#include <X11/Xlib.h>
#include <QX11Info>

/**** DLL Interface ****/
typedef KGenericFactory<KBackground, QWidget> KBackGndFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_background, KBackGndFactory("kcmbackground"))

/**** KBackground ****/
KBackground::~KBackground( )
{
    delete m_pConfig;
}

KBackground::KBackground(QWidget *parent, const char *name, const QStringList &args)
    : KCModule(KBackGndFactory::instance(), parent, args)
{
    int screen_number = 0;
    if (QX11Info::display())
	screen_number = DefaultScreen(QX11Info::display());
    Q3CString configname;
    if (screen_number == 0)
	configname = "kdesktoprc";
    else
	configname.sprintf("kdesktop-screen-%drc", screen_number);
    m_pConfig = new KConfig(configname, false, false);

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_base = new BGDialog(this, m_pConfig);
    setQuickHelp( m_base->quickHelp());
    layout->add(m_base);
    layout->addStretch();

    KImageIO::registerFormats();

    // reparenting that is done.
    setAcceptDrops(true);

    connect(m_base, SIGNAL(changed(bool)), SIGNAL(changed(bool)));

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmbackground"), I18N_NOOP("KDE Background Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1997-2002 Martin R. Jones"));

    about->addAuthor("Waldo Bastian", 0, "bastian@kde.org");
    about->addAuthor("George Staikos", 0, "staikos@kde.org");
    about->addAuthor("Martin R. Jones", 0, "jones@kde.org");
    about->addAuthor("Matthias Hoelzer-Kluepfel", 0, "mhk@kde.org");
    about->addAuthor("Stephan Kulow", 0, "coolo@kde.org");
    about->addAuthor("Mark Donohoe", 0, 0);
    about->addAuthor("Matej Koss", 0 , 0);

    setAboutData( about );
}


void KBackground::load()
{
    m_base->load();
}


void KBackground::save()
{
    m_base->save();

    // reconfigure kdesktop. kdesktop will notify all clients
    DCOPClient *client = kapp->dcopClient();
    if (!client->isAttached())
	client->attach();

    int screen_number = 0;
    if (QX11Info::display())
	screen_number = DefaultScreen(QX11Info::display());
    QString appname;
    if (screen_number == 0)
	appname = "kdesktop";
    else 
	appname.sprintf("kdesktop-screen-%d", screen_number);

    client->send(appname.latin1(), "KBackgroundIface", "configure()", QByteArray());
}

void KBackground::defaults()
{
    m_base->defaults();
}

#include "main.moc"
