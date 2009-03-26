/***************************************************************************
 *   Copyright (C) 2008 by Lukas Appelhans                                 *
 *   l.appelhans@gmx.de                                                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "quicklaunchIcon.h"

#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneDragDropEvent>
#include <QPainter>

#include <KMenu>
#include <KRun>

#include <Plasma/ToolTipManager>

#include "quicklaunchApplet.h"

QuicklaunchIcon::QuicklaunchIcon(const KUrl & appUrl, const QString & text, const KIcon & icon, const QString & genericName, QuicklaunchApplet *parent)
  : Plasma::IconWidget(icon, QString(), parent),
    m_launcher(parent),
    m_appUrl(appUrl),
    m_text(text),
    m_genericName(genericName),
    m_removeAction(0),
    m_iconSize(0)
{
    setAcceptDrops(true);
    Plasma::ToolTipManager::self()->registerWidget(this);
    connect(this, SIGNAL(clicked()), SLOT(execute()));
}

QuicklaunchIcon::~QuicklaunchIcon()
{
    Plasma::ToolTipManager::self()->unregisterWidget(this);
}

KUrl QuicklaunchIcon::url() const
{
    return m_appUrl;
}

void QuicklaunchIcon::setIconSize(int px)
{
    m_iconSize = px;
}

int QuicklaunchIcon::iconSize() const
{
    return m_iconSize;
}

void QuicklaunchIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setClipRect(option->rect);
    QRect rect = option->rect;
    rect.setSize(QSize(m_iconSize, m_iconSize));
    //rect.moveCenter(option->rect.center());
    rect.moveCenter(QPoint(option->rect.width() / 2, option->rect.height() / 2));
    //rect.setLeft((option->rect.width() - m_iconSize) / 2);
    //rect.setTop((option->rect.height() - m_iconSize) / 2);
    //QStyleOptionGraphicsItem opt = *option;
    //opt.rect = rect;
    //kDebug() << "Paint to:" << rect << "Original rect was:" << option->rect;

    painter->drawPixmap(rect, icon().pixmap(m_iconSize));
    //IconWidget::paint(painter, &opt, widget);
}

void QuicklaunchIcon::execute()
{
    new KRun(m_appUrl, 0);
}

void QuicklaunchIcon::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    KMenu m;
    m.addAction(m_launcher->action("configure"));
    m.addSeparator();
    m.addActions(m_launcher->contextActions(this));
    m.addSeparator();
    m.addAction(m_launcher->action("remove"));
    m.exec(event->screenPos());
}

void QuicklaunchIcon::toolTipAboutToShow()
{
  Plasma::ToolTipContent toolTip;
  toolTip.setMainText(m_text);
  toolTip.setSubText(m_genericName);
  toolTip.setImage(icon().pixmap(m_iconSize));

  Plasma::ToolTipManager::self()->setContent(this, toolTip);
}

void QuicklaunchIcon::toolTipHidden()
{
    Plasma::ToolTipManager::self()->clearContent(this);
}

/*bool QuicklaunchIcon::eventFilter(QObject * object, QEvent * event)
{
    if (event->type() != QEvent::GraphicsSceneMouseMove) {
        return Plasma::IconWidget::eventFilter(object, event);
    }

    QDrag * drag = new QDrag(static_cast<QGraphicsSceneMouseEvent*>(event)->widget());
    QMimeData * data = new QMimeData;
    KUrl::List urls;
    urls << m_appUrl;
    urls.populateMimeData(data);
    drag->setMimeData(data);
    drag->exec();
    event->accept();
    return true;
}*/
