#ifndef KONSOLEBOOKMARKMENU_H
#define KONSOLEBOOKMARKMENU_H

#include <q3ptrlist.h>
#include <q3ptrstack.h>
#include <QObject>
#include <sys/types.h>
#include <kbookmark.h>
#include <kbookmarkmenu.h>

#include "konsolebookmarkhandler.h"


class QString;
class KBookmark;
class KAction;
class KActionMenu;
class KActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KMenu;
class KonsoleBookmarkMenu;

class KonsoleBookmarkMenu : public KBookmarkMenu
{
    Q_OBJECT

public:
    KonsoleBookmarkMenu( KBookmarkManager* mgr,
                         KonsoleBookmarkHandler * _owner, KMenu * _parentMenu,
                         KActionCollection *collec, bool _isRoot,
                         bool _add = true, const QString & parentAddress = "");

    void fillBookmarkMenu();

public Q_SLOTS:

Q_SIGNALS:

private Q_SLOTS:

private:
    KonsoleBookmarkHandler * m_kOwner;

protected Q_SLOTS:
    void slotAboutToShow2();
    void slotBookmarkSelected();
    void slotNSBookmarkSelected();

protected:
    void refill();

private:
    class KonsoleBookmarkMenuPrivate;
    KonsoleBookmarkMenuPrivate *d;
};

#endif // KONSOLEBOOKMARKMENU_H
