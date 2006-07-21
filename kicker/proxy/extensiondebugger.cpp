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

#include <QFile>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QString>

#include <kapplication.h>
#include <kglobal.h>
#include <klibloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kpanelextension.h>
#include <kaboutdata.h>
#include <QFileInfo>

#include "appletinfo.h"
#include "extensiondebugger.h"
#include "extensiondebugger.moc"



static KCmdLineOptions options[] =
{
  { "+desktopfile", I18N_NOOP("The extensions desktop file"), 0 },
  KCmdLineLastOption
};

KPanelExtension* loadExtension(const AppletInfo& info)
{
    KLibLoader* loader = KLibLoader::self();
    KLibrary* lib = loader->library(QFile::encodeName(info.library()));

    if (!lib)
    {
        kWarning() << "cannot open extension: " << info.library()
                    << " because of " << loader->lastErrorMessage() << endl;
        return 0;
    }

    KPanelExtension* (*init_ptr)(QWidget *, const QString&);
    init_ptr = (KPanelExtension* (*)(QWidget *, const QString&))lib->symbol( "init" );

    if (!init_ptr)
    {
        kWarning() << info.library() << " is not a kicker extension!" << endl;
        return 0;
    }

    return init_ptr(0, info.configFile());
}

int main( int argc, char ** argv )
{
    KAboutData aboutData( "extensionproxy", I18N_NOOP("Panel extension proxy.")
                          , "v0.1.0"
                          ,I18N_NOOP("Panel extension proxy.")
                          , KAboutData::License_BSD
                          , "(c) 2000, The KDE Developers");
    KCmdLineArgs::init(argc, argv, &aboutData );
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    KCmdLineArgs::addStdCmdLineOptions();
    KCmdLineArgs::addCmdLineOptions(options); // Add our own options.

    KApplication a;
    a.disableSessionManagement();

    KGlobal::dirs()->addResourceType("extensions", KStandardDirs::kde_default("data") +
				     "kicker/extensions");

    QString df;

    // parse cmdline args
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // sanity check
    if ( args->count() == 0 )
        KCmdLineArgs::usage(i18n("No desktop file specified") );


    QString desktopFile = QString( args->arg(0) );

    // try simple path first
    QFileInfo finfo( desktopFile );
    if ( finfo.exists() ) {
	df = finfo.absoluteFilePath();
    } else {
	// locate desktop file
	df = KGlobal::dirs()->findResource("extensions", desktopFile);
    }

    // does the config file exist?
    if (!QFile::exists(df)) {
	kError() << "Failed to locate extension desktop file: " << desktopFile << endl;
	return 1;
    }

    AppletInfo info( df );

    KPanelExtension *extension = loadExtension(info);
    if ( !extension ) {
	kError() << "Failed to load extension: " << info.library() << endl;
	return 1;
    }

    ExtensionContainer *container = new ExtensionContainer( extension );
    container->show();

    QObject::connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );

    int result = a.exec();

    delete extension;
    return result;
}

ExtensionContainer::ExtensionContainer( KPanelExtension *extension, QWidget *parent )
    : QWidget( parent ), m_extension( extension )
{
    ( new QVBoxLayout( this ) )->setAutoAdd( true );

    QPushButton *configButton = new QPushButton( i18n( "Configure..." ), this );
    connect( configButton, SIGNAL( clicked() ),
             this, SLOT( showPreferences() ) );

    m_extension->setParent( this);
    m_extension->move(0,0);
}

void ExtensionContainer::resizeEvent( QResizeEvent * )
{
    m_extension->setGeometry( 0, 0, width(), height() );
}

void ExtensionContainer::showPreferences()
{
    m_extension->action( Plasma::Preferences );
}
