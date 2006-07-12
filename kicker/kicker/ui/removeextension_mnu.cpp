/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

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

#include <klocale.h>
#include <kglobal.h>

#include "kicker.h"
#include "extensionmanager.h"
#include "pluginmanager.h"

#include "panelmenuiteminfo.h"
#include "removeextension_mnu.h"
#include "removeextension_mnu.moc"
#include <q3tl.h>
#include <QList>
#include <QMenu>

static const int REMOVEALLID = 1000;

PanelRemoveExtensionMenu::PanelRemoveExtensionMenu( QWidget *parent, const char *name )
    : QMenu( parent )
{
    setObjectName(name);
    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

PanelRemoveExtensionMenu::PanelRemoveExtensionMenu()
{
}

void PanelRemoveExtensionMenu::slotAboutToShow()
{
    int id = 0;

    clear();
    m_containers = ExtensionManager::self()->containers();
    PanelMenuItemInfo::List items;

    ExtensionContainer::List::iterator itEnd = m_containers.end();
    foreach (const ExtensionContainer* container, m_containers)
    {
        const AppletInfo info = container->info();
        QString name = info.name().replace("&", "&&");
        switch (container->position())
        {
            case Plasma::Top:
                name = i18n("%1 (Top)", name);
            break;
            case Plasma::Right:
                name = i18n("%1 (Right)", name);
            break;
            case Plasma::Bottom:
                name = i18n("%1 (Bottom)", name);
            break;
            case Plasma::Left:
                name = i18n("%1 (Left)", name);
            break;
            case Plasma::Floating:
                name = i18n("%1 (Floating)", name);
            break;
         }
        items.append(PanelMenuItemInfo(QString(), name, id));
        ++id;
    }

    qHeapSort(items);
    foreach (PanelMenuItemInfo item, items)
    {
        item.plug(this);
    }

    if (m_containers.count() > 1)
    {
        insertSeparator();
        insertItem(i18n("All"), REMOVEALLID);
    }
}

void PanelRemoveExtensionMenu::slotExec( int id )
{
    if (id == REMOVEALLID)
    {
        ExtensionManager::self()->removeAllContainers();
    }
    else if (m_containers.at(id) != m_containers.back())
    {
        ExtensionManager::self()->removeContainer(m_containers.at(id));
    }
}

