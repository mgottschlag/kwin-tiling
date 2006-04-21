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

#include <qregexp.h>
#include <q3tl.h>

#include <QList>
#include <QMenu>
#include <kiconloader.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include "panelbutton.h"
#include "pluginmanager.h"
#include "containerarea.h"
#include "container_button.h"

#include "panelmenuiteminfo.h"
#include "removebutton_mnu.h"
#include "removebutton_mnu.moc"

PanelRemoveButtonMenu::PanelRemoveButtonMenu( ContainerArea* cArea,
                                              QWidget *parent )
    : QMenu( parent ), containerArea( cArea )
{
    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

void PanelRemoveButtonMenu::addToContainers(const QString& type)
{
    BaseContainer::List list = containerArea->containers(type);
    for (BaseContainer::Iterator it = list.begin();
         it != list.end();
         ++it)
    {
        if ((*it)->isImmutable())
        {
            continue;
        }
        containers.append(*it);
    }
}

void PanelRemoveButtonMenu::slotAboutToShow()
{
    clear();
    containers.clear();

    addToContainers("URLButton");
    addToContainers("ServiceButton");
    addToContainers("ServiceMenuButton");
    addToContainers("ExecButton");

    int id = 0;
    PanelMenuItemInfo::List items;
    for (BaseContainer::Iterator it = containers.begin(); it != containers.end(); ++it)
    {
        items.append(PanelMenuItemInfo((*it)->icon(), (*it)->visibleName(), id));
        id++;
    }

    qHeapSort(items);

    for (PanelMenuItemInfo::List::iterator it = items.begin();
         it != items.end();
         ++it)
    {
        (*it).plug(this);
    }

    if (containers.count() > 1)
    {
        addSeparator();
        insertItem(i18n("All"), this, SLOT(slotRemoveAll()), 0, id);
    }
}

void PanelRemoveButtonMenu::slotExec( int id )
{
    if (containers.at(id) != containers.back())
    {
        containerArea->removeContainer(containers.at(id));
    }
}

PanelRemoveButtonMenu::~PanelRemoveButtonMenu()
{
}

void PanelRemoveButtonMenu::slotRemoveAll()
{
    containerArea->removeContainers(containers);
}
