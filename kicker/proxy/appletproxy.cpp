/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTDHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdlib.h>

#include <qstring.h>
#include <qfile.h>
#include <qobject.h>
#include <QPixmap>

#include <kapplication.h>
#include <kglobal.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kpanelapplet.h>
#include <kaboutdata.h>
#include <qfileinfo.h>
#include <dcopclient.h>
#include <kwin.h>

#include "appletinfo.h"

#include "appletproxy.h"
#include "kickerSettings.h"
#include "appletproxy.moc"

#include <X11/Xlib.h>
#include <QX11Info>

static KCmdLineOptions options[] =
{
  { "+desktopfile", I18N_NOOP("The applet's desktop file"), 0 },
  { "configfile <file>", I18N_NOOP("The config file to be used"), 0 },
  KCmdLineLastOption
};

extern "C" KDE_EXPORT int kdemain( int argc, char ** argv )
{
    KAboutData aboutData( "kicker", I18N_NOOP("Panel applet proxy.")
                          , "v0.1.0"
                          ,I18N_NOOP("Panel applet proxy.")
                          , KAboutData::License_BSD
                          , "(c) 2000, The KDE Developers");
    KCmdLineArgs::init(argc, argv, &aboutData );
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    KCmdLineArgs::addStdCmdLineOptions();
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

    KApplication a;
    a.disableSessionManagement();

    KGlobal::dirs()->addResourceType("applets", KStandardDirs::kde_default("data") +
				     "kicker/applets");

    // setup proxy object
    AppletProxy proxy(0, "appletproxywidget");

    // parse cmdline args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() == 0 )
        KCmdLineArgs::usage(i18n("No desktop file specified") );

    // Perhaps we should use a konsole-like solution here (shell, list of args...)
    QString desktopfile( args->arg(0) );

    // load applet DSO
    if ( !QFile::exists( desktopfile ) &&
         !desktopfile.endsWith( ".desktop" ) )
        desktopfile.append( ".desktop" );

    if ( !QFile::exists( desktopfile ) )
        desktopfile = locate( "applets", desktopfile ).latin1();

    proxy.loadApplet( desktopfile, args->getOption("configfile"));

    // dock into our applet container
    proxy.showStandalone();

    return a.exec();
}

AppletProxy::AppletProxy(QObject* parent, const char* name)
  : QObject(parent, name)
  , DCOPObject("AppletProxy")
  , _info(0)
  , _applet(0)
{
    // try to attach to DCOP server
    if (!kapp->dcopClient()->attach()) {
	kError() << "Failed to attach to DCOP server." << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not be started due to DCOP communication problems."),
                           i18n("Applet Loading Error"));
	exit(0);
    }

    if (kapp->dcopClient()->registerAs("applet_proxy", true).isEmpty()) {
	kError() << "Failed to register at DCOP server." << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not be started due to DCOP registration problems."),
                           i18n("Applet Loading Error"));
	exit(0);
    }

    _bg = QPixmap();
}

AppletProxy::~AppletProxy()
{
    kapp->dcopClient()->detach();
    delete _info;
    delete _applet;
}

void AppletProxy::loadApplet(const QString& desktopFile, const QString& configFile)
{
    QString df;
    KickerSettings::instance("kickerrc");

    // try simple path first
    QFileInfo finfo( desktopFile );
    if ( finfo.exists() ) {
	df = finfo.absoluteFilePath();
    } else {
	// locate desktop file
	df = KGlobal::dirs()->findResource("applets", desktopFile);
    }

    QFile file(df);
    // does the config file exist?
    if (df.isNull() || !file.exists()) {
	kError() << "Failed to locate applet desktop file: " << desktopFile << endl;
        KMessageBox::error(0,
                           i18n("The applet proxy could not load the applet information from %1.").arg(desktopFile), 
                           i18n("Applet Loading Error"));
	exit(0);
    }

    // create AppletInfo instance
    delete _info;
    _info = new AppletInfo(df);

    // set the config file
    if (!configFile.isNull())
	_info->setConfigFile(configFile);

    // load applet DSO
    _applet = loadApplet(*_info);

    // sanity check
    if (!_applet)
    {
	kError() << "Failed to load applet: " << _info->library() << endl;
        KMessageBox::error(0,
                           i18n("The applet %1 could not be loaded via the applet proxy.").arg(_info->name()), 
                           i18n("Applet Loading Error"));
	exit(0);
    }
}

KPanelApplet* AppletProxy::loadApplet(const AppletInfo& info)
{
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library(QFile::encodeName(info.library()));

    if (!lib)
    {
        kWarning() << "cannot open applet: " << info.library()
                    << " because of " << loader->lastErrorMessage() << endl;
        return 0;
    }

    KPanelApplet* (*init_ptr)(QWidget *, const QString&);
    init_ptr = (KPanelApplet* (*)(QWidget *, const QString&))lib->symbol( "init" );

    if (!init_ptr)
    {
        kWarning() << info.library() << " is not a kicker plugin!" << endl;
        return 0;
    }

    return init_ptr(0, info.configFile());
}

void AppletProxy::repaintApplet(QWidget* widget) 
{
    widget->repaint();
 
    const QObjectList children = widget->children();

    foreach (QObject* object, children)
    {
        QWidget *w = dynamic_cast<QWidget*>(object);
        if (w)
        {
            repaintApplet(w);
        }
    }
}

bool AppletProxy::process(const DCOPCString &fun, const QByteArray &data,
                          DCOPCString& replyType, QByteArray &replyData)
{
    if ( fun == "widthForHeight(int)" )
	{
	    QDataStream dataStream( data );
	    int height;
	    dataStream >> height;
	    QDataStream reply( &replyData, QIODevice::WriteOnly );

	    reply.setVersion(QDataStream::Qt_3_1);
	    replyType = "int";

	    if (!_applet)
		reply << height;
	    else
		reply << _applet->widthForHeight(height);

	    return true;
	}
    else if ( fun == "heightForWidth(int)" )
	{
	    QDataStream dataStream( data );
	    int width;
	    dataStream >> width;
	    QDataStream reply( &replyData, QIODevice::WriteOnly );

	    reply.setVersion(QDataStream::Qt_3_1);
	    replyType = "int";

	    if(!_applet)
		reply << width;
	    else
		reply << _applet->heightForWidth(width);

	    return true;
	}
    else if ( fun == "setDirection(int)" )
	{
	    QDataStream dataStream( data );
	    int dir;
	    dataStream >> dir;

	    if(_applet) {
		_applet->setPosition( (Plasma::Position)dir );
	    }
	    return true;
	}
    else if ( fun == "setAlignment(int)" )
	{
	    QDataStream dataStream( data );
	    int alignment;
	    dataStream >> alignment;

	    if(_applet) {
		_applet->setAlignment( (Plasma::Alignment)alignment );
	    }
	    return true;
	}
    else if ( fun == "removedFromPanel()" )
	{
            delete _applet;
            _applet = 0;
            exit(0);
	    return true;
	}
    else if ( fun == "about()" )
	{
	    if(_applet) _applet->action( Plasma::About );
	    return true;
	}
    else if ( fun == "help()" )
	{
	    if(_applet) _applet->action( Plasma::Help );
	    return true;
	}
    else if ( fun == "preferences()" )
	{
	    if(_applet) _applet->action( Plasma::Preferences );
	    return true;
	}
    else if (fun == "reportBug()" )
  {
      if(_applet) _applet->action( Plasma::ReportBug );
      return true;
  }
    else if ( fun == "actions()" )
	{
	    QDataStream reply( &replyData, QIODevice::WriteOnly );

	    reply.setVersion(QDataStream::Qt_3_1);
	    int actions = 0;
	    if(_applet) actions = _applet->actions();
	    reply << actions;
	    replyType = "int";
	    return true;
	}
    else if ( fun == "type()" )
	{
	    QDataStream reply( &replyData, QIODevice::WriteOnly );

	    reply.setVersion(QDataStream::Qt_3_1);
	    int type = 0;
	    if (_applet) type = static_cast<int>(_applet->type());
	    reply << type;
	    replyType = "int";
	    return true;
	}
    else if ( fun == "setBackground(QPixmap)" )
        {
            QDataStream dataStream( data ); 
            dataStream >> _bg;
            if(_applet)
                if ( _bg.isNull() ) { // no transparency
		    _applet->unsetPalette();
		    _applet->repaint();
		}
                else { //transparency
		    _applet->blockSignals(true);
		    _applet->setBackgroundMode(Qt::FixedPixmap);
		    _applet->setPaletteBackgroundPixmap(_bg);
		    repaintApplet(_applet);
		    _applet->blockSignals(false);
                }
            return true;
        }
    return false;
}

void AppletProxy::showStandalone()
{
    if (!_applet)
    {
        return;
    }

    _applet->resize( _applet->widthForHeight( 48 ), 48 );
    _applet->setMinimumSize( _applet->size() );
    _applet->setWindowTitle( _info->name() );
    kapp->setMainWidget( _applet );
    _applet->show();
}

