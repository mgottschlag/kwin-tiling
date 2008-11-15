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

#include <KMenu>
#include <KRun>

#include "quicklaunchApplet.h"

QuicklaunchIcon::QuicklaunchIcon(const KUrl & appUrl, const KIcon & icon, QuicklaunchApplet *parent)
  : Plasma::IconWidget(icon, QString(), parent),
    m_launcher(parent),
    m_appUrl(appUrl),
    m_removeAction(0)
{
    setAcceptDrops(true);
    connect(this, SIGNAL(clicked()), SLOT(execute()));
}

QuicklaunchIcon::~QuicklaunchIcon()
{
}

KUrl QuicklaunchIcon::url() const
{
    return m_appUrl;
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
