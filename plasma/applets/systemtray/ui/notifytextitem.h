/***************************************************************************
 *   notifytextitem.h                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>                 *
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#ifndef NOTIFYTEXTITEM_H
#define NOTIFYTEXTITEM_H

#include <QtCore/QObject>

#include <QtGui/QGraphicsItem>


/**
 * Graphics item for holding single notification text and its actions
 */
class NotifyTextItem : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    NotifyTextItem(QGraphicsItem *parent = 0);
    ~NotifyTextItem();

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    void setSize(qreal width, qreal height);
    void setBody(const QString &body);
    void setActions(const QHash<QString, QString> &actions, const QStringList &actionOrder);
    void clearActions();

signals:
    void actionInvoked(const QString &action);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:
    struct ActionInfo;
    class Private;
    Private* const d;
};

#endif
