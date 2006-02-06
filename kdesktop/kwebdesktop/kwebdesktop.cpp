/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Idea by Gael Duval
// Implementation by David Faure

#include <config.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kurifilter.h>
#include <kio/job.h>
#include <khtmlview.h>

#include <Q3ScrollView>
#include <QFile>

#include "kwebdesktop.h"
#include <kmimetype.h>
#include <kparts/componentfactory.h>
#include "kwebdesktopsettings.h"

#include "kwebdesktop.moc"

static KCmdLineOptions options[] =
{
  { "+width", I18N_NOOP("Width of the image to create"), 0 },
  { "+height", I18N_NOOP("Height of the image to create"), 0 },
  { "+file", I18N_NOOP("File sname where to dump the output in png format"), 0 },
  { "+[URL]", I18N_NOOP("URL to open (if not specified, it is read from kwebdesktoprc)"), 0 },
  KCmdLineLastOption
};

KWebDesktopRun::KWebDesktopRun( KWebDesktop* webDesktop, const KUrl & url )
    : m_webDesktop(webDesktop), m_url(url)
{
    kDebug() << "KWebDesktopRun::KWebDesktopRun starting get" << endl;
    KIO::Job * job = KIO::get(m_url, false, false);
    connect( job, SIGNAL( result( KIO::Job *)),
             this, SLOT( slotFinished(KIO::Job *)));
    connect( job, SIGNAL( mimetype( KIO::Job *, const QString &)),
             this, SLOT( slotMimetype(KIO::Job *, const QString &)));
}

void KWebDesktopRun::slotMimetype( KIO::Job *job, const QString &_type )
{
    KIO::SimpleJob *sjob = static_cast<KIO::SimpleJob *>(job);
    // Update our URL in case of a redirection
    m_url = sjob->url();
    QString type = _type; // necessary copy if we plan to use it
    sjob->putOnHold();
    kDebug() << "slotMimetype : " << type << endl;

    KParts::ReadOnlyPart* part = m_webDesktop->createPart( type );
    // Now open the URL in the part
    if ( part )
        part->openURL( m_url );
}

void KWebDesktopRun::slotFinished( KIO::Job * job )
{
    // The whole point of all this is to abort silently on error
    if (job->error())
    {
        kDebug() << job->errorString() << endl;
        kapp->quit();
    }
}


int main( int argc, char **argv )
{
    KAboutData data( "kwebdesktop", I18N_NOOP("KDE Web Desktop"),
                     VERSION,
                     I18N_NOOP("Displays an HTML page as the background of the desktop"),
                     KAboutData::License_GPL,
                     "(c) 2000, David Faure <faure@kde.org>" );
    data.addAuthor( "David Faure", I18N_NOOP("developer and maintainer"), "faure@kde.org" );

    KCmdLineArgs::init( argc, argv, &data );

    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() != 3 && args->count() != 4 )
    {
       args->usage();
       return 1;
    }
    const int width      = QByteArray(args->arg(0)).toInt();
    const int height     = QByteArray(args->arg(1)).toInt();
    QByteArray imageFile = args->arg(2);
    QString url;
    if (args->count() == 4)
        url = QString::fromLocal8Bit(args->arg(3));

    KWebDesktop *webDesktop = new KWebDesktop( 0, imageFile, width, height );

    if (url.isEmpty())
      url = KWebDesktopSettings::uRL();
    // Apply uri filter
    KURIFilterData uridata = url;
    KURIFilter::self()->filterURI( uridata );
    KUrl u = uridata.uri();

    // Now start getting, to ensure mimetype and possible connection
    KWebDesktopRun * run = new KWebDesktopRun( webDesktop, u );

    int ret = app.exec();

    KIO::SimpleJob::removeOnHold(); // Kill any slave that was put on hold.
    delete webDesktop;
    delete run;
    //khtml::Cache::clear();

    return ret;
}

void KWebDesktop::slotCompleted()
{
    kDebug() << "KWebDesktop::slotCompleted" << endl;
    // Dump image to m_imageFile
    QPixmap snapshot = QPixmap::grabWidget( m_part->widget() );
    snapshot.save( QFile::decodeName(m_imageFile), "PNG" );
    // And terminate the app.
    kapp->quit();
}

KParts::ReadOnlyPart* KWebDesktop::createPart( const QString& mimeType )
{
    delete m_part;
    m_part = 0;

    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
    if ( !mime || mime == KMimeType::defaultMimeTypePtr() )
        return 0;
    if ( mime->is( "text/html" ) || mime->is( "text/xml" ) || mime->is( "application/xhtml+xml" ) )
    {
        KHTMLPart* htmlPart = new KHTMLPart;
        htmlPart->widget()->resize(m_width,m_height);

        htmlPart->setMetaRefreshEnabled(false);
        htmlPart->setJScriptEnabled(false);
        htmlPart->setJavaEnabled(false);

        htmlPart->view()->setHScrollBarMode( Q3ScrollView::AlwaysOff );
        htmlPart->view()->setVScrollBarMode( Q3ScrollView::AlwaysOff );

        connect( htmlPart->widget(), SIGNAL( finishedLayout() ),
                 this, SLOT( slotCompleted() ) );
        m_part = htmlPart;
    } else {
        // Try to find an appropriate viewer component
        m_part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>
                 ( mimeType, QString(), 0, 0, this, 0 );
        if ( !m_part )
            kWarning() << "No handler found for " << mimeType << endl;
        else {
            kDebug() << "Loaded " << m_part->className() << endl;
            connect( m_part, SIGNAL( completed() ),
                     this, SLOT( slotCompleted() ) );
        }
    }
    if ( m_part ) {
        connect( m_part, SIGNAL( canceled(const QString &) ),
                 this, SLOT( slotCompleted() ) );
    }
    return m_part;
}

KWebDesktop::~KWebDesktop()
{
    delete m_part;
}
