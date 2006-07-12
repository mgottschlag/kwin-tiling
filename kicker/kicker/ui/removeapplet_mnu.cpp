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

#include <q3tl.h>

#include "pluginmanager.h"
#include "containerarea.h"
#include "container_applet.h"

#include "panelmenuiteminfo.h"
#include "removeapplet_mnu.h"
#include "removeapplet_mnu.moc"

PanelRemoveAppletMenu::PanelRemoveAppletMenu(ContainerArea* cArea,
                                             QWidget *parent,
                                             const char *name)
    : QMenu(parent), m_containerArea(cArea)
{
    setObjectName(name);
    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

void PanelRemoveAppletMenu::slotAboutToShow()
{
    int id = 0;

    clear();
    m_containers = m_containerArea->containers("Applet") +
                   m_containerArea->containers("Special Button");

    PanelMenuItemInfo::List items;

    for (BaseContainer::List::iterator it = m_containers.begin();
         it != m_containers.end();)
    {
        BaseContainer* container = *it;
        if (container->isImmutable())
        {
            ++it;
            m_containers.erase(it);
            continue;
        }

        items.append(PanelMenuItemInfo(container->icon(),
                                       container->visibleName().replace("&", "&&"),
                                       id));
        ++id;
        ++it;
    }

    qHeapSort(items);

    for (PanelMenuItemInfo::List::iterator it = items.begin();
         it != items.end();
         ++it)
    {
        (*it).plug(this);
    }

    if (m_containers.count() > 1)
    {
        insertSeparator();
        insertItem(i18n("All"), this, SLOT(slotRemoveAll()), 0, id);
    }
}

void PanelRemoveAppletMenu::slotExec(int id)
{
    if (m_containers.at(id) != m_containers.back())
    {
        m_containerArea->removeContainer(m_containers.at(id));
    }
}

void PanelRemoveAppletMenu::slotRemoveAll()
{
    m_containerArea->removeContainers(m_containers);
}
