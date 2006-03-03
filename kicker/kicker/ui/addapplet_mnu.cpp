/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

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

#include <kiconloader.h>

#include <QMenu>

#include "pluginmanager.h"
#include "containerarea.h"

#include "addapplet_mnu.h"
#include "addapplet_mnu.moc"


PanelAddAppletMenu::PanelAddAppletMenu(ContainerArea* cArea, QWidget *parent, const char *name)
    : QMenu(parent), containerArea(cArea)
{
    setName(name);
    connect(this, SIGNAL(activated(int)), SLOT(slotExec(int)));
    connect(this, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
}

void PanelAddAppletMenu::slotAboutToShow()
{
    clear();

    applets = PluginManager::applets();

    AppletInfo::List::const_iterator it = applets.constBegin();
    for (int i = 0; it != applets.constEnd(); ++it, ++i)
    {
        const AppletInfo& ai = (*it);
        if (ai.isHidden())
        {
            continue;
        }

        if (ai.icon().isEmpty() || ai.icon() == "unknown")
        {
            insertItem(ai.name().replace( "&", "&&" ), i);
        }
        else
        {
            insertItem(SmallIconSet(ai.icon()), ai.name().replace( "&", "&&" ), i);
        }

        if (ai.isUniqueApplet() && PluginManager::self()->hasInstance(ai))
        {
            setItemEnabled( i, false );
            setItemChecked( i, true );
        }
    }
}

void PanelAddAppletMenu::slotExec(int id)
{
    containerArea->addApplet( applets[id].desktopFile() );
}
