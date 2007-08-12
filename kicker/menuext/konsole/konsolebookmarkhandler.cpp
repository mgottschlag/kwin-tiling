// Born as kdelibs/kio/kfile/kfilebookmarkhandler.cpp

#include <stdio.h>
#include <stdlib.h>

#include <QTextStream>
//Added by qt3to4:
#include <QByteArray>

#include <kbookmarkimporter.h>
#include <kmimetype.h>
#include <kmenu.h>
#include <ksavefile.h>
#include <kstandarddirs.h>
#include <kbookmarkmenu.h>

#include "konsole_mnu.h"
#include "konsolebookmarkhandler.h"

KonsoleBookmarkHandler::KonsoleBookmarkHandler( KonsoleMenu *konsole, bool )
    : QObject( konsole ),
      KBookmarkOwner(),
      m_konsole( konsole )
{
    setObjectName( "KonsoleBookmarkHandler" );
    m_menu = new KMenu(konsole);
    m_menu->setObjectName("bookmark menu");

    QString file = KStandardDirs::locate( "data", "kfile/bookmarks.xml" );
    if ( file.isEmpty() )
        file = KStandardDirs::locateLocal( "data", "kfile/bookmarks.xml" );

    KBookmarkManager *manager = KBookmarkManager::managerForFile( file, "kfile" );
    manager->setUpdate( true );

    m_bookmarkMenu = new KBookmarkMenu( manager, this, m_menu, 0 );
}

void KonsoleBookmarkHandler::openBookmark(const KBookmark & bm, Qt::MouseButtons, Qt::KeyboardModifiers )
{
    emit openUrl( bm.url().url(), bm.text() );
}

QString KonsoleBookmarkHandler::currentUrl() const
{
    return m_konsole->baseURL().url();
}

#include "konsolebookmarkhandler.moc"
