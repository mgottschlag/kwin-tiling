// Born as kdelibs/kio/kfile/kfilebookmarkhandler.h

#ifndef KONSOLEBOOKMARKHANDLER_H
#define KONSOLEBOOKMARKHANDLER_H

#include <kbookmarkmanager.h>
#include "konsolebookmarkmenu.h"
//Added by qt3to4:
#include <QTextStream>
#include <QByteArray>
#include <QMenu>

class QTextStream;
class KMenu;
class KonsoleBookmarkMenu;
class KonsoleMenu;

class KonsoleBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    KonsoleBookmarkHandler( KonsoleMenu *konsole, bool toplevel );

    // KBookmarkOwner interface:
    virtual void openBookmarkURL( const QString& url, const QString& title )
                                { emit openUrl( url, title ); }
    virtual QString currentURL() const;

    KMenu *menu() const { return m_menu; }

Q_SIGNALS:
    void openUrl( const QString& url, const QString& title );

private Q_SLOTS:
    // for importing
    void slotNewBookmark( const QString& text, const QByteArray& url,
                          const QString& additionalInfo );
    void slotNewFolder( const QString& text, bool open,
                        const QString& additionalInfo );
    void slotBookmarksChanged( const QString &, const QString & caller );
    void newSeparator();
    void endFolder();

private:
    void importOldBookmarks( const QString& path, const QString& destinationPath );

    KonsoleMenu *m_konsole;
    KMenu *m_menu;
    KonsoleBookmarkMenu *m_bookmarkMenu;
    QTextStream *m_importStream;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KonsoleBookmarkHandlerPrivate;
    KonsoleBookmarkHandlerPrivate *d;
};


#endif // KONSOLEBOOKMARKHANDLER_H
