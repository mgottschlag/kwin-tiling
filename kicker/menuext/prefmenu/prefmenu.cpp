/*****************************************************************

Copyright (c) 1996-2002 the kicker authors. See file AUTHORS.

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
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

//Added by qt3to4:
#include <QCursor>
#include <QMouseEvent>
#include <QMenuItem>
#include <QPixmap>
#include <QTimer>

#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kservice.h>
#include <kservicegroup.h>
#include <kstandarddirs.h>
#include <ksycoca.h>
#include <kurl.h>
#include <k3urldrag.h>
#include <ktoolinvocation.h>
#include <kworkspace.h>
#include "kickerSettings.h"
#include "utils.h"
#include "prefmenu.h"

K_EXPORT_KICKER_MENUEXT(prefmenu, PrefMenu)

const int idStart = 4242;

PrefMenu::PrefMenu(QWidget *parent,
                   const QStringList &/*args*/)
    : KPanelMenu(i18n("Settings"), parent),
      m_clearOnClose(false)
{
}

PrefMenu::PrefMenu(const QString& label,
                   const QString& root,
                   QWidget *parent)
    : KPanelMenu(label, parent),
      m_clearOnClose(false),
      m_root(root)
{
    connect(KSycoca::self(), SIGNAL(databaseChanged()),
            this, SLOT(clearOnClose()));

    connect(this, SIGNAL(aboutToHide()),
            this, SLOT(aboutToClose()));
}

PrefMenu::~PrefMenu()
{
}

void PrefMenu::insertMenuItem(KService::Ptr& s,
                              int nId,
                              int nIndex,
                              const QStringList *suppressGenericNames)
{
    QString serviceName = s->name();
    // add comment
    QString comment = s->genericName();
    if (!comment.isEmpty())
    {
        if (KickerSettings::menuEntryFormat() == KickerSettings::NameAndDescription)
        {
            if (!suppressGenericNames ||
                !suppressGenericNames->contains(s->untranslatedGenericName()))
            {
                serviceName = QString("%1 (%2)").arg(serviceName).arg(comment);
            }
        }
        else if (KickerSettings::menuEntryFormat() == KickerSettings::DescriptionAndName)
        {
            serviceName = QString("%1 (%2)").arg(comment).arg(serviceName);
        }
        else if (KickerSettings::menuEntryFormat() == KickerSettings::DescriptionOnly)
        {
            serviceName = comment;
        }
    }

    // restrict menu entries to a sane length
    if (serviceName.length() > 60)
    {
        serviceName.truncate(57);
        serviceName += "...";
    }

    // check for NoDisplay
    if (s->noDisplay())
    {
        return;
    }

    // ignore dotfiles.
    if ((serviceName.at(0) == '.'))
    {
        return;
    }

    // item names may contain ampersands. To avoid them being converted
    // to accelerators, replace them with two ampersands.
    serviceName.replace("&", "&&");

    QIcon iconset = Plasma::menuIconSet(s->icon());
    int newId = insertItem(iconset, serviceName, nId, nIndex);
    m_entryMap.insert(newId, KSycocaEntry::Ptr::staticCast(s));
}

void PrefMenu::mousePressEvent(QMouseEvent * ev)
{
    m_dragStartPos = ev->pos();
    KPanelMenu::mousePressEvent(ev);
}

void PrefMenu::mouseMoveEvent(QMouseEvent * ev)
{
    KPanelMenu::mouseMoveEvent(ev);

    if ((ev->state() & Qt::LeftButton) != Qt::LeftButton)
    {
        return;
    }

    QPoint p = ev->pos() - m_dragStartPos;
    if (p.manhattanLength() <= QApplication::startDragDistance())
    {
        return;
    }

    int id = static_cast<QMenuItem*>(actionAt(m_dragStartPos))->id();

    // Don't drag items we didn't create.
    if (id < idStart)
    {
        return;
    }

    if (!m_entryMap.contains(id))
    {
        kDebug(1210) << "Cannot find service with menu id " << id << endl;
        return;
    }

    KSycocaEntry::Ptr e = m_entryMap[id];

    QPixmap icon;
    KUrl url;

    switch (e->sycocaType())
    {
        case KST_KService:
        {
            KService::Ptr service(KService::Ptr::staticCast(e));
            icon = service->pixmap(K3Icon::Small);
            QString filePath = service->desktopEntryPath();
            if (filePath[0] != '/')
            {
                filePath = KStandardDirs::locate("apps", filePath);
            }
            url.setPath(filePath);
            break;
        }

        case KST_KServiceGroup:
        {
            KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(e);
            icon = KGlobal::iconLoader()->loadIcon(serviceGroup->icon(), K3Icon::Small);
            url = "programs:/" + serviceGroup->relPath();
            break;
        }

        default:
        {
            return;
            break;
        }
    }

    // If the path to the desktop file is relative, try to get the full
    // path from KStdDirs.
    K3URLDrag *d = new K3URLDrag(KUrl::List(url), this);
    connect(d, SIGNAL(destroyed()), this, SLOT(dragObjectDestroyed()));
    d->setPixmap(icon);
    d->dragCopy();

    // Set the startposition outside the panel, so there is no drag initiated
    // when we use drag and click to select items. A drag is only initiated when
    // you click to open the menu, and then press and drag an item.
    m_dragStartPos = QPoint(-1,-1);
}

void PrefMenu::initialize()
{
    if (initialized())
    {
        return;
    }

    // Set the startposition outside the panel, so there is no drag initiated
    // when we use drag and click to select items. A drag is only initiated when
    // you click to open the menu, and then press and drag an item.
    m_dragStartPos = QPoint(-1,-1);

    if (m_root.isEmpty())
    {
        insertItem(Plasma::menuIconSet("kcontrol"),
                   i18n("Control Center"),
                   this, SLOT(launchControlCenter()));
        addSeparator();
    }

    // We ask KSycoca to give us all services under Settings/
    KServiceGroup::Ptr root = KServiceGroup::group(m_root.isEmpty() ? "Settings/" : m_root);

    if (!root || !root->isValid())
    {
        return;
    }

    KServiceGroup::List list = root->entries(true, true, true,
                                             KickerSettings::menuEntryFormat() == KickerSettings:: NameAndDescription);

    if (list.isEmpty())
    {
        setItemEnabled(insertItem(i18n("No Entries")), false);
        return;
    }

    int id = idStart;

    QStringList suppressGenericNames = root->suppressGenericNames();

    foreach(const KSycocaEntry::Ptr &e, list)
    {
        if (e->isType(KST_KServiceGroup))
        {

            KServiceGroup::Ptr g(KServiceGroup::Ptr::staticCast(e));
            QString groupCaption = g->caption();

            // Avoid adding empty groups.
            KServiceGroup::Ptr subMenuRoot = KServiceGroup::group(g->relPath());
            if (subMenuRoot->childCount() == 0)
            {
                continue;
            }

            // Ignore dotfiles.
            if ((g->name().at(0) == '.'))
            {
                continue;
            }

            // Item names may contain ampersands. To avoid them being converted
            // to accelerators, replace each ampersand with two ampersands.
            groupCaption.replace("&", "&&");

            QIcon iconset = Plasma::menuIconSet(g->icon());
            PrefMenu* m = new PrefMenu(g->name(), g->relPath(), this);
            m->setIcon(iconset);
            m->setWindowTitle(groupCaption);
            addMenu(m);
        }
        else if (e->isType(KST_KService))
        {
            KService::Ptr s(KService::Ptr::staticCast(e));
            insertMenuItem(s, id++, -1, &suppressGenericNames);
        }
        else if (e->isType(KST_KServiceSeparator))
        {
            addSeparator();
        }
    }

    setInitialized(true);
}

void PrefMenu::slotExec(int id)
{
    if (!m_entryMap.contains(id))
    {
        return;
    }

    KWorkSpace::propagateSessionManager();
    KService::Ptr service(KService::Ptr::staticCast(m_entryMap[id]));
    KToolInvocation::startServiceByDesktopPath(service->desktopEntryPath(),
                                            QStringList(), 0, 0, 0, "", true);
    m_dragStartPos = QPoint(-1,-1);
}

void PrefMenu::clearOnClose()
{
    if (!initialized())
    {
        return;
    }

    m_clearOnClose = isVisible();
    if (!m_clearOnClose)
    {
        // we aren't visible right now so clear immediately
        slotClear();
    }
}

void PrefMenu::slotClear()
{
    if( isVisible())
    {
        // QPopupMenu's aboutToHide() is emitted before the popup is really hidden,
        // and also before a click in the menu is handled, so do the clearing
        // only after that has been handled
        QTimer::singleShot( 100, this, SLOT( slotClear()));
        return;
    }

    m_entryMap.clear();
    KPanelMenu::slotClear();
}

void PrefMenu::aboutToClose()
{
    if (m_clearOnClose)
    {
        m_clearOnClose = false;
        slotClear();
    }
}

void PrefMenu::launchControlCenter()
{
    KToolInvocation::startServiceByDesktopName("kcontrol", QStringList(),
                                            0, 0, 0, "", true);
}


void PrefMenu::dragObjectDestroyed()
{
    if (K3URLDrag::target() != this)
    {
        close();
    }
}

#include "prefmenu.moc"
