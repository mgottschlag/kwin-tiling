/***********************************************************************************
* Window List: Plasmoid to show list of opened windows.
* Copyright (C) 2009 Michal Dutkiewicz aka Emdek <emdeck@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
***********************************************************************************/

#include "WindowList.h"

#include <QApplication>

#include <KIcon>
#include <KMenu>
#include <KLocale>
#include <KIconLoader>
#include <KWindowSystem>

#include <Plasma/IconWidget>
#include <Plasma/ToolTipManager>

#include <taskmanager/taskitem.h>
#include <taskmanager/taskactions.h>
#include <taskmanager/taskmanager.h>
#include <taskmanager/groupmanager.h>

K_EXPORT_PLASMA_APPLET(windowlist, WindowList)

WindowList::WindowList(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args)
{
    setAspectRatioMode(Plasma::ConstrainedSquare);

    const int iconSize = IconSize(KIconLoader::Desktop);
    resize((iconSize * 2), (iconSize * 2));
}

WindowList::~WindowList()
{
    qDeleteAll(m_listMenu->actions());
    delete m_listMenu;
}

void WindowList::init()
{
    Plasma::IconWidget *icon = new Plasma::IconWidget(KIcon("preferences-system-windows"), QString(), this);

    m_listMenu = new KWindowListMenu;
    m_listMenu->installEventFilter(this);

    registerAsDragHandle(icon);

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addItem(icon);

    Plasma::ToolTipManager::self()->setContent(this, Plasma::ToolTipContent(i18n("Window list"), i18n("Show list of opened windows"), KIcon("preferences-system-windows").pixmap(IconSize(KIconLoader::Desktop))));

    connect(this, SIGNAL(activate()), this, SLOT(showMenu()));
    connect(this, SIGNAL(destroyed()), m_listMenu, SLOT(deleteLater()));
    connect(icon, SIGNAL(clicked()), this, SLOT(showMenu()));
    connect(m_listMenu, SIGNAL(triggered(QAction*)), this, SLOT(triggered(QAction*)));
}

void WindowList::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Applet::mousePressEvent(event);

    if (event->button() == Qt::MidButton) {
        event->accept();
    }
}

void WindowList::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Applet::mouseReleaseEvent(event);

    if (event->button() == Qt::MidButton) {
        showMenu(true);
    }
}

void WindowList::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QList<WId> windows = KWindowSystem::windows();

    if (windows.count() < 2) {
        return;
    }

    int position = windows.indexOf(KWindowSystem::activeWindow());

    if (event->delta() > 0) {
        ++position;

        if (position >= windows.count()) {
            position = 0;
        }
    } else {
        --position;

        if (position < 0) {
            position = (windows.count() - 1);
        }
    }

    KWindowSystem::activateWindow(windows.at(position));
}

bool WindowList::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        KMenu *menu = qobject_cast<KMenu*>(object);

        if (menu && menu->activeAction() && menu->activeAction()->data().type() == QVariant::ULongLong) {
            QContextMenuEvent *cmEvent = static_cast<QContextMenuEvent *>(event);
            QList<QAction*> actionList;
            TaskManager::TaskItem item(this, TaskManager::TaskManager::self()->findTask((WId)menu->activeAction()->data().toULongLong()));
            TaskManager::GroupManager groupManager(this);
            TaskManager::BasicMenu taskMenu(NULL, &item, &groupManager, actionList);
            if (taskMenu.exec(cmEvent->globalPos())) {
                m_listMenu->hide();
            }
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() != Qt::LeftButton) {
            return false;
        }

        KMenu *menu = static_cast<KMenu*>(object);

        if (menu && menu->activeAction() && menu->activeAction()->data().type() == QVariant::ULongLong) {
            m_dragStartPosition = mouseEvent->pos();
        }
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (!(mouseEvent->buttons() & Qt::LeftButton) || (mouseEvent->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance()) {
            return false;
        }

        KMenu *menu = static_cast<KMenu*>(object);

        if (menu && menu->activeAction() && menu->activeAction()->data().type() == QVariant::ULongLong) {
            QDrag *drag = new QDrag(menu);
            QMimeData *mimeData = new QMimeData;
            QByteArray data;
            WId window = (WId)menu->activeAction()->data().toULongLong();

            data.resize(sizeof(WId));

            memcpy(data.data(), &window, sizeof(WId));

            mimeData->setData("windowsystem/winid", data);

            drag->setMimeData(mimeData);
            drag->setPixmap(menu->activeAction()->icon().pixmap(32, 32));

            m_listMenu->hide();

            drag->exec();
            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

void WindowList::showMenu(bool onlyCurrentDesktop)
{
    QList<WId> windows = KWindowSystem::windows();
    QList<QAction*> actionList;
    QList< QList<QAction*> > windowList;
    int amount = 0;
    int number = 0;

    qDeleteAll(m_listMenu->actions());
    //m_listMenu->clear();

    if (!onlyCurrentDesktop) {
        m_listMenu->addTitle(i18n("Actions"));

        QAction *unclutterAction = m_listMenu->addAction(i18n("Unclutter Windows"));
        QAction *cascadeAction = m_listMenu->addAction(i18n("Cascade Windows"));

        connect(unclutterAction, SIGNAL(triggered()), m_listMenu, SLOT(slotUnclutterWindows()));
        connect(cascadeAction, SIGNAL(triggered()), m_listMenu, SLOT(slotCascadeWindows()));
    }

    for (int i = 0; i <= KWindowSystem::numberOfDesktops(); ++i) {
        windowList.append(QList<QAction*>());
    }

    for (int i = 0; i < windows.count(); ++i) {
        KWindowInfo window = KWindowSystem::windowInfo(windows.at(i), (NET::WMGeometry | NET::WMFrameExtents | NET::WMWindowType | NET::WMDesktop | NET::WMState | NET::XAWMState | NET::WMVisibleName));
        NET::WindowType type = window.windowType(NET::NormalMask | NET::DialogMask | NET::OverrideMask | NET::UtilityMask | NET::DesktopMask | NET::DockMask | NET::TopMenuMask | NET::SplashMask | NET::ToolbarMask | NET::MenuMask);

        if ((onlyCurrentDesktop && !window.isOnDesktop(KWindowSystem::currentDesktop())) || type == NET::Desktop || type == NET::Dock || type == NET::TopMenu || type == NET::Splash || type == NET::Menu || type == NET::Toolbar || window.hasState(NET::SkipPager)) {
            windows.removeAt(i);

            --i;

            continue;
        }

        ++amount;

        QAction *action = new QAction(QIcon(KWindowSystem::icon(windows.at(i))), window.visibleName(), this);
        action->setData((unsigned long long) windows.at(i));

        QString window_title = QString(action->text());
        window_title.truncate(55);
        action->setText(window_title);

        QFont font = QFont(action->font());

        if (window.isMinimized()) {
            font.setItalic(true);
        } else if (KWindowSystem::activeWindow() == windows.at(i)) {
            font.setUnderline(true);
            font.setBold(true);
        }

        action->setFont(font);

        number = ((onlyCurrentDesktop || window.onAllDesktops()) ? 0 : window.desktop());

        QList<QAction*> subList = windowList.value(number);
        subList.append(action);

        windowList.replace(number, subList);
    }

    const bool useSubMenus = (!onlyCurrentDesktop && KWindowSystem::numberOfDesktops() > 1 && (amount / KWindowSystem::numberOfDesktops()) > 5);

    if (amount && useSubMenus) {
        m_listMenu->addTitle(i18n("Desktops"));
    }

    for (int i = 0; i <= KWindowSystem::numberOfDesktops(); ++i) {
        if (windowList.value(i).isEmpty()) {
            continue;
        }

        KMenu *subMenu = NULL;
        QAction *subMenuAction = NULL;
        QString title = (i ? KWindowSystem::desktopName(i) : (onlyCurrentDesktop ? i18n("Current desktop") : i18n("On all desktops")));

        if (useSubMenus) {
            subMenuAction = m_listMenu->addAction(title);

            subMenu = new KMenu(m_listMenu);
            subMenu->installEventFilter(this);
        } else {
            m_listMenu->addTitle(title);
        }

        for (int j = 0; j < windowList.value(i).count(); ++j) {
            if (useSubMenus) {
                subMenu->addAction(windowList.value(i).value(j));
            } else {
                m_listMenu->addAction(windowList.value(i).value(j));
            }
        }

        if (useSubMenus) {
            subMenuAction->setMenu(subMenu);
        }
    }

    if (!amount) {
        qDeleteAll(m_listMenu->actions());

        m_listMenu->clear();

        QAction *noWindows = m_listMenu->addAction(i18n("No windows"));
        noWindows->setEnabled(false);
    }

    if (formFactor() == Plasma::Vertical || formFactor() == Plasma::Horizontal) {
        m_listMenu->popup(popupPosition(m_listMenu->sizeHint()));
    } else {
        m_listMenu->popup(QCursor::pos());
    }
}

void WindowList::triggered(QAction *action)
{
    if (action->data().type() == QVariant::ULongLong) {
        if (KWindowSystem::activeWindow() == (WId)action->data().toULongLong()) {
            KWindowSystem::minimizeWindow((WId)action->data().toULongLong());
        } else {
            KWindowSystem::activateWindow((WId)action->data().toULongLong());
        }
    }
}

#include "WindowList.moc"
