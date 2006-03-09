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

#include <qcursor.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <QDesktopWidget>

#include <QByteArray>
#include <QMenu>

#include <kapplication.h>
#include <dcopclient.h>

#include "client_mnu.h"
#include "container_extension.h"
#include "utils.h"
#include "k_mnu.h"
#include "kicker.h"

#include "menumanager.h"
#include "menumanager.moc"

// Why MenuManager doesn't use KStaticDeleter
// MenuManager gets created before the ExtensionManager
// So using KStaticDeleter results in MenuManager getting
// deleted before ExtensionManager, which means also the panels
// which means also the K Menu buttons. K Menu buttons call
// MenuManager in their dtor, so if MenuManager is already gone
// then every KButton will cause it to be reconstructed.
// So we rely on Kicker to delete MenuManager on the way out
// ensuring it's the last thing to go.
MenuManager* MenuManager::m_self = 0;

MenuManager* MenuManager::self()
{
    if (!m_self)
    {
        m_self = new MenuManager();
    }

    return m_self;
}

MenuManager::MenuManager(QObject *parent)
    : QObject(parent, "MenuManager"), DCOPObject("MenuManager")
{
    m_kmenu = new PanelKMenu;
    kapp->dcopClient()->setNotifications(true);
    connect(kapp->dcopClient(), SIGNAL(applicationRemoved(const QByteArray&)),
            this, SLOT(applicationRemoved(const QByteArray&)));
}

MenuManager::~MenuManager()
{
    if (this == m_self)
    {
        m_self = 0;
    }

    delete m_kmenu;
}

void MenuManager::slotSetKMenuItemActive()
{
    m_kmenu->selectFirstItem();
}

void MenuManager::showKMenu()
{
    m_kmenu->showMenu();
}

void MenuManager::popupKMenu(const QPoint &p)
{
//    kDebug(1210) << "popupKMenu()" << endl;
    if (m_kmenu->isVisible())
    {
        m_kmenu->hide();
    }
    else if (p.isNull())
    {
        m_kmenu->popup(QCursor::pos());
    }
    else
    {
        m_kmenu->popup( p );
    }
}

void MenuManager::registerKButton(PanelPopupButton *button)
{
    if (!button)
    {
        return;
    }

    m_kbuttons.append(button);
}

void MenuManager::unregisterKButton(PanelPopupButton *button)
{
    m_kbuttons.removeAll(button);
}

PanelPopupButton* MenuManager::findKButtonFor(QMenu* menu)
{
    KButtonList::const_iterator itEnd = m_kbuttons.constEnd();
    for (KButtonList::const_iterator it = m_kbuttons.constBegin(); it != itEnd; ++it)
    {
        if ((*it)->popup() == menu)
        {
            return *it;
        }
    }

    return 0;
}

void MenuManager::kmenuAccelActivated()
{
    if (m_kmenu->isVisible())
    {
        m_kmenu->hide();
        return;
    }

    m_kmenu->initialize();

    if (m_kbuttons.isEmpty())
    {
        // no button to use, make it behave like a desktop menu
        QPoint p;
        // Popup the K-menu at the center of the screen.
        QDesktopWidget* desktop = KApplication::desktop();
        QRect r = desktop->screenGeometry(desktop->screenNumber(QCursor::pos()));
        // kMenu->rect() is not valid before showing, use sizeHint()
        p = r.center() - QRect( QPoint( 0, 0 ), m_kmenu->sizeHint()).center();
        m_kmenu->popup(p);

        // when the cursor is in the area where the menu pops up,
        // the item under the cursor gets selected. The single shot
        // avoids this from happening by allowing the item to be selected
        // when the event loop is enterred, and then resetting it.
        QTimer::singleShot(0, this, SLOT(slotSetKMenuItemActive()));
    }
    else
    {
        // We need the kmenu's size to place it at the right position.
        // We cannot rely on the popup menu's current size(), if it wasn't
        // shown before, so we resize it here according to its sizeHint().
        const QSize size = m_kmenu->sizeHint();
        m_kmenu->resize(size.width(),size.height());

        PanelPopupButton* button = m_kbuttons.at(0);

        // let's unhide the panel while we're at it. traverse the widget
        // hierarchy until we find the panel, if any
        QObject* menuParent = button->parent();
        while (menuParent)
        {
            ExtensionContainer* ext = dynamic_cast<ExtensionContainer*>(menuParent);

            if (ext)
            {
                ext->unhideIfHidden();
                // make sure it's unhidden before we use it to figure out
                // where to popup
                qApp->processEvents();
                break;
            }

            menuParent= menuParent->parent();
        }

        m_kmenu->popup(Plasma::popupPosition(button->popupDirection(),
                                                m_kmenu, button));
    }
}

DCOPCString MenuManager::createMenu(QPixmap icon, QString text)
{
    static int menucount = 0;
    menucount++;
    QString name;
    name.sprintf("kickerclientmenu-%d", menucount );
    KickerClientMenu* p = new KickerClientMenu(0);
    p->setObjectName(name);
    clientmenus.append(p);
    m_kmenu->initialize();
    p->text = text;
    p->icon = icon;
    p->idInParentMenu = m_kmenu->insertClientMenu( p );
    p->createdBy = kapp->dcopClient()->senderId();
    m_kmenu->adjustSize();
    return name.toLatin1();
}

void MenuManager::removeMenu(DCOPCString menu)
{
    bool iterate = true;
    ClientMenuList::iterator it = clientmenus.begin();
    for (; it != clientmenus.end(); iterate ? ++it : it)
    {
        iterate = true;
        KickerClientMenu* m = *it;
        if (m->objId() == menu)
        {
            m_kmenu->removeClientMenu(m->idInParentMenu);
            it = clientmenus.erase(it);
            iterate = false;
        }
    }
    m_kmenu->adjustSize();
}


void MenuManager::applicationRemoved(const QByteArray& appRemoved)
{
    bool iterate = true;
    ClientMenuList::iterator it = clientmenus.begin();
    for (; it != clientmenus.end(); iterate ? ++it : it)
    {
        iterate = true;
        KickerClientMenu* m = *it;
        if (QByteArray(m->createdBy) == appRemoved)
        {
            m_kmenu->removeClientMenu(m->idInParentMenu);
            it = clientmenus.erase(it);
            iterate = false;
        }
    }
    m_kmenu->adjustSize();
}

bool MenuManager::process(const DCOPCString &fun, const QByteArray &data,
				DCOPCString &replyType, QByteArray &replyData)
{
    if ( fun == "createMenu(QPixmap,QString)" ) {
	QDataStream dataStream( data );
	dataStream.setVersion(QDataStream::Qt_3_1);
	QPixmap icon;
	QString text;
	dataStream >> icon >> text;
	QDataStream reply( replyData );
	reply << createMenu( icon, text );
	replyType = "QCString";
	return true;
    } else if ( fun == "removeMenu(QCString)" ) {
	QDataStream dataStream( data );
	dataStream.setVersion(QDataStream::Qt_3_1);
	DCOPCString menu;
	dataStream >> menu;
	removeMenu( menu );
	replyType = "void";
	return true;
    }
    return false;
}
