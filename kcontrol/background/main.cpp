/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmbackground.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
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

#include <qobject.h>
#include <qlayout.h>
#include <qstring.h>
//#include <qstringlist.h>
//#include <qpushbutton.h>
#include <qdragobject.h>
#include <qevent.h>
#include <qwhatsthis.h>
//#include <qhbuttongroup.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kimageio.h>
#include <kgenericfactory.h>

#include <bgdefaults.h>
#include <bgsettings.h>
#include <backgnd.h>
#include <main.h>

/* as late as possible, as it includes some X headers without protecting them */
#include <kwin.h>
#include <X11/Xlib.h>

/**** DLL Interface ****/
typedef KGenericFactory<KBackground, QWidget> KBackGndFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_background, KBackGndFactory("kcmbackground"));

/**** KBackground ****/
KBackground::~KBackground( )
{
}

KBackground::KBackground(QWidget *parent, const char *name, const QStringList &/* */)
    : KCModule(KBackGndFactory::instance(), parent, name)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_base = new Backgnd(this);
    m_base->show();
    layout->add(m_base);


    kdDebug() << "KBackground\n";
    KImageIO::registerFormats();

    int screen_number = 0;
    if (qt_xdisplay())
	screen_number = DefaultScreen(qt_xdisplay());
    QCString configname;
    if (screen_number == 0)
	configname = "kdesktoprc";
    else
	configname.sprintf("kdesktop-screen-%drc", screen_number);

    // reparenting that is done.
    setAcceptDrops(true);

    connect(m_base, SIGNAL(changed(bool)), SLOT(slotChildChanged(bool)));

    //m_base->setWidgets();

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
    if (qt_xdisplay())
	screen_number = DefaultScreen(qt_xdisplay());
    QCString appname;
    if (screen_number == 0)
	appname = "kdesktop";
    else
	appname.sprintf("kdesktop-screen-%d", screen_number);

    client->send(appname, "KBackgroundIface", "configure()", "");

    emit changed(false);
}

void KBackground::defaults()
{
    m_base->defaults();
}

void KBackground::slotChildChanged(bool different)
{
   emit changed(different);
}


QString KBackground::quickHelp() const
{
    return m_base->quickHelp();
}

const KAboutData* KBackground::aboutData() const
{

    KAboutData *about =
    new KAboutData(I18N_NOOP("kcmbackground"), I18N_NOOP("KDE Background Control Module"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 1997-2002 Martin R. Jones"));

    about->addAuthor("George Staikos", 0, "staikos@kde.org");
    about->addAuthor("Martin R. Jones", 0, "jones@kde.org");
    about->addAuthor("Matthias Hoelzer-Kluepfel", 0, "mhk@kde.org");
    about->addAuthor("Stephan Kulow", 0, "coolo@kde.org");
    about->addAuthor("Mark Donohoe", 0, 0);
    about->addAuthor("Matej Koss", 0 , 0);

    return about;
}



#include "main.moc"
