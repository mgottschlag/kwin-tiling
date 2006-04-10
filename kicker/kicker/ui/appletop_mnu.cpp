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

#include <kguiitem.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kpanelapplet.h>
#include <kstdguiitem.h>

#include "kicker.h"
#include "appletop_mnu.h"
#include "container_button.h"
#include "containerarea.h"
//Added by qt3to4:
#include <QPixmap>
#include <QKeyEvent>
#include <QMenu>
#include <kauthorized.h>

PanelAppletOpMenu::PanelAppletOpMenu(int actions, QMenu *opMenu, const QMenu* appletsMenu,
                                     const QString & title, const QString &icon,
                                     QWidget *parent, const char *name)
  : QMenu(parent)
{
    setName(name);
    bool needSeparator = false;
    bool isButton = (parent && parent->inherits("ButtonContainer"));
    bool isMenu = false;
    QString titleText = title;
    titleText = titleText.replace('&', "&&");
    if (isButton)
    {
        isMenu = static_cast<ButtonContainer*>(parent)->isAMenu();
    }

    if (!Kicker::self()->isImmutable())
    {
        QString text = isButton ? (isMenu ? i18n("&Move %1 Menu", titleText) :
                                            i18n("&Move %1 Button", titleText)) :
                                   i18n("&Move %1", titleText);
        insertItem(SmallIconSet("move"), text, Move);

        // we look for a container area to see if we can add containers
        // this is part of the kiosk support in kicker, allowing
        // one to block users from adding new containers
        ContainerArea* area = 0;
        QObject* findTheArea = parent ? parent->parent() : 0;
        while (findTheArea)
        {
            area = dynamic_cast<ContainerArea*>(findTheArea);

            if (area)
            {
                break;
            }

            findTheArea = findTheArea->parent();
        }

        if  (!area || area->canAddContainers())
        {
            text = isButton ? (isMenu ? i18n("&Remove %1 Menu", titleText) :
                                        i18n("&Remove %1 Button", titleText)) :
                              i18n("&Remove %1", titleText);
            insertItem(SmallIconSet("remove"), text, Remove);
            needSeparator = true;
        }
    }

    if (actions & Plasma::ReportBug)
    {
        if (needSeparator)
        {
            insertSeparator();
            needSeparator = false;
        }

        insertItem(i18n("Report &Bug..."), ReportBug);
    }

    if (actions & Plasma::About)
    {
        if (needSeparator)
        {
            insertSeparator();
        }

        QPixmap iconPix(kapp->iconLoader()->loadIcon(icon,
                                                     K3Icon::Small, 0,
                                                     K3Icon::DefaultState,
                                                     0, true));
        insertItem(QIcon(iconPix), i18n("&About %1", titleText ), About);
        needSeparator = !(actions & Plasma::Help);
    }

    if (actions & Plasma::Help)
    {
        if (needSeparator)
        {
            insertSeparator();
        }

        insertItem(SmallIconSet("help"), KStdGuiItem::help().text(), Help);
        needSeparator = true;
    }

    if (!Kicker::self()->isImmutable() && (actions & Plasma::Preferences))
    {
        if (isButton)
        {
            insertItem(SmallIconSet("configure"),
                       i18n("&Configure %1 Button...", titleText), Preferences);
        }
        else
        {
            insertItem(SmallIconSet("configure"),
                       i18n("&Configure %1...", titleText), Preferences);
        }
        needSeparator = true;
    }

    if (appletsMenu)
    {
        if (needSeparator)
        {
            insertSeparator();
            needSeparator = false;
        }

        QString text = title.isEmpty() ? i18n("Applet Menu") :
                                         i18n("%1 Menu", titleText);

        // the 2 const_cast's below prevents const_cast'ing in multiple places
        // elsewhere in the kicker code base. it's ugly, but unavoidable
        // unless either QPopupMenu one day allows inserting const
        // QPopupMenu's or we uglify other bits of kicker's API,
        // notably Plasma::customMeu()
        if (icon.isEmpty())
        {
            insertItem(text, const_cast<QMenu*>(appletsMenu));
        }
        else
        {
            insertItem(SmallIconSet(icon), text,
                       const_cast<QMenu*>(appletsMenu));
        }
    }

    if ((actions & PanelAppletOpMenu::KMenuEditor) && KAuthorized::authorizeKAction("menuedit"))
    {
        if (needSeparator)
        {
            insertSeparator();
            needSeparator = false;
        }

        insertItem(SmallIconSet("kmenuedit"), i18n("&Menu Editor"), Preferences);
    }

    if ((actions & PanelAppletOpMenu::BookmarkEditor) &&
        KAuthorized::authorizeKAction("edit_bookmarks"))
    {
        if (needSeparator)
        {
            insertSeparator();
        }
        needSeparator = false;

        // NOTE: keditbookmarks is from konqueror. seeing as this is in kdebase
        //       as well this should be ok?
        insertItem(SmallIconSet("keditbookmarks"), 
                   i18n("&Edit Bookmarks"),
                   Preferences);
    }

    if (needSeparator)
    {
        insertSeparator();
    }

    insertItem(SmallIcon("panel"), i18n("Panel Menu"), opMenu);
    adjustSize();
}

void PanelAppletOpMenu::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        emit escapePressed();
    }

    QMenu::keyPressEvent(e);
}

#include "appletop_mnu.moc"
