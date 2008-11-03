/***************************************************************************
 *   Copyright (C) 2008 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


#include "tasksmenu.h"

//Qt
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QApplication>
#include <QBitmap>

//KDE
#include <KIconLoader>

//Plasma
#include <plasma/applet.h>
#include <plasma/framesvg.h>
#include <plasma/theme.h>

namespace TaskManager
{


TasksMenu::TasksMenu(QWidget *parent, TaskGroup *group, GroupManager *groupManager, Plasma::Applet *applet)
    :  GroupPopupMenu(parent, group, groupManager),
       m_applet(applet)
{
    setAttribute(Qt::WA_NoSystemBackground);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("dialogs/background");

    m_itemBackground = new Plasma::FrameSvg(this);
    m_itemBackground->setImagePath("widgets/tasks");
    m_itemBackground->setElementPrefix("hover");

    //since the thing gets destroyed on close we can set this just one time for now
    const int topHeight = m_background->marginSize(Plasma::TopMargin);
    const int leftWidth = m_background->marginSize(Plasma::LeftMargin);
    const int rightWidth = m_background->marginSize(Plasma::RightMargin);
    const int bottomHeight = m_background->marginSize(Plasma::BottomMargin);

    switch (m_applet->location()) {
    case Plasma::BottomEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::TopBorder
                                                                     | Plasma::FrameSvg::RightBorder);
        setContentsMargins(leftWidth, topHeight, rightWidth, 0);
        break;
    case Plasma::TopEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::BottomBorder
                                                                     | Plasma::FrameSvg::RightBorder);

        setContentsMargins(leftWidth, 0, rightWidth, bottomHeight);
        break;
    case Plasma::LeftEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder
                                                                    | Plasma::FrameSvg::RightBorder);

        setContentsMargins(0, topHeight, rightWidth, bottomHeight);
        break;
    case Plasma::RightEdge:
        m_background->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::BottomBorder
                                                                    | Plasma::FrameSvg::LeftBorder);

        setContentsMargins(leftWidth, topHeight, 0, bottomHeight);
        break;
    default:
        m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
        setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    }
}

TasksMenu::~TasksMenu()
{}

void TasksMenu::paintEvent(QPaintEvent *event)
{
    //Q_UNUSED(event)

    QPainter painter(this);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(event->rect(), Qt::transparent);
    m_background->paintFrame(&painter);

    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);


    foreach (QAction *a, actions()) {
        QRect actionRect(actionGeometry(a));
        QRect iconRect(QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignLeft | Qt::AlignVCenter,
                              QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall), actionRect));
        QRect textRect(QStyle::alignedRect(QApplication::layoutDirection(), Qt::AlignRight | Qt::AlignVCenter,
                              QSize(actionRect.width()-iconRect.width()-3, actionRect.height()), actionRect));

        if (activeAction() == a) {
            m_itemBackground->resizeFrame(actionRect.size() + QSize(4,4));
            m_itemBackground->paintFrame(&painter, actionRect.topLeft() - QPoint(2,2));
        }

        painter.drawPixmap(iconRect, a->icon().pixmap(iconRect.size()));
        painter.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        painter.drawText(textRect, Qt::AlignLeft|Qt::AlignVCenter, a->text());
    }

}

void TasksMenu::resizeEvent(QResizeEvent *event)
{
    m_background->resizeFrame(event->size());
    setMask(m_background->mask());
}

};

#include "tasksmenu.moc"


