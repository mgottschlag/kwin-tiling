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
    void openBookmark( const KBookmark &, Qt::MouseButtons, Qt::KeyboardModifiers );

private:

    KonsoleMenu *m_konsole;
    KMenu *m_menu;
    KBookmarkMenu *m_bookmarkMenu;

private:
    class KonsoleBookmarkHandlerPrivate;
    KonsoleBookmarkHandlerPrivate *d;
};


#endif // KONSOLEBOOKMARKHANDLER_H
