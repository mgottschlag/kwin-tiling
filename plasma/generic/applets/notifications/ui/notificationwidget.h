/***************************************************************************
 *   notificationwidget.h                                                  *
 *                                                                         *
 *   Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>                 *
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <Plasma/Frame>

#include "../core/notification.h"

class NotificationWidgetPrivate;

/**
 *  A graphics item, representing notification message.
 */
class NotificationWidget : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal bodyHeight READ bodyHeight WRITE setBodyHeight)

public:
    NotificationWidget(Notification *notification, QGraphicsWidget *parent);
    ~NotificationWidget();

    void setCollapsed(bool collapse, bool animate = true);
    bool isCollapsed() const;

    void setTitleBarVisible(bool visible);
    bool isTitleBarVisible() const;

    qreal bodyHeight() const;
    void setBodyHeight(const qreal height);

    Notification *notification() const;

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);

Q_SIGNALS:
    void actionTriggered(Notification *);

private:
    friend class NotificationWidgetPrivate;
    NotificationWidgetPrivate* const d;

    Q_PRIVATE_SLOT(d, void updateNotification())
    Q_PRIVATE_SLOT(d, void buttonClicked())
    Q_PRIVATE_SLOT(d, void hideFinished())
};

#endif // NOTIFICATIONWIDGET_H
