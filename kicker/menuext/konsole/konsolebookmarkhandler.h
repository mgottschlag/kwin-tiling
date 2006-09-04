// Born as kdelibs/kio/kfile/kfilebookmarkhandler.h

#ifndef KONSOLEBOOKMARKHANDLER_H
#define KONSOLEBOOKMARKHANDLER_H

#include <kbookmarkmanager.h>
//Added by qt3to4:
#include <QTextStream>
#include <QByteArray>
#include <QMenu>

class QTextStream;
class KMenu;
class KBookmarkMenu;
class KonsoleMenu;

class KonsoleBookmarkHandler : public QObject, public KBookmarkOwner
{
    Q_OBJECT

public:
    KonsoleBookmarkHandler( KonsoleMenu *konsole, bool toplevel );
    virtual QString currentUrl() const;

    KMenu *menu() const { return m_menu; }

Q_SIGNALS:
    void openUrl( const QString& url, const QString& title );

private Q_SLOTS:
    // for importing
    void openBookmark( KBookmark, Qt::MouseButtons, Qt::KeyboardModifiers );
    void slotNewBookmark( const QString& text, const QByteArray& url,
                          const QString& additionalInfo );
    void slotNewFolder( const QString& text, bool open,
                        const QString& additionalInfo );
    void newSeparator();
    void endFolder();

private:
    void importOldBookmarks( const QString& path, const QString& destinationPath );

    KonsoleMenu *m_konsole;
    KMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;
    QTextStream *m_importStream;

private:
    class KonsoleBookmarkHandlerPrivate;
    KonsoleBookmarkHandlerPrivate *d;
};


#endif // KONSOLEBOOKMARKHANDLER_H
