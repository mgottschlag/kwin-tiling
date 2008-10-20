/***************************************************************************
 *   notifytextitem.h                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>                 *
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

#include "notifytextitem.h"

#include <QtCore/QHash>

#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QCursor>
#include <QtGui/QGraphicsSceneHoverEvent>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>

#include <KGlobalSettings>

#include <plasma/theme.h>


struct NotifyTextItem::ActionInfo
{
    QString name;
    QRectF rect;
};


class NotifyTextItem::Private
{
public:
    Private(NotifyTextItem *q)
        : q(q),
          size(30, 30)
    {
    }

    void updateActionRects();

    NotifyTextItem *q;

    QTextDocument body;
    QHash<QString, ActionInfo> actions;
    QStringList actionOrder;

    QSizeF size;
    QString hoveredAction;
    QString clickedAction;
    int actionHeight;
};


NotifyTextItem::NotifyTextItem(QGraphicsItem* parent)
    : QGraphicsItem(parent),
      d(new Private(this))
{
    d->body.setDefaultFont(KGlobalSettings::smallestReadableFont());
}


NotifyTextItem::~NotifyTextItem()
{
    delete d;
}


QRectF NotifyTextItem::boundingRect() const
{
    return QRectF(QPointF(0, 0), d->size);
}


void NotifyTextItem::setSize(qreal width, qreal height)
{
    d->size = QSizeF(width, height);
    d->body.setTextWidth(width);

    d->updateActionRects();
}


void NotifyTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    const QColor highlightColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::HighlightColor);
    const int fadeHeight = 20;

    painter->setRenderHint(QPainter::Antialiasing, true);
    QRectF bodyRect = boundingRect();

    if (!d->actions.isEmpty()) {
        bodyRect.setHeight(bodyRect.height() - d->actionHeight);

        painter->save();
        painter->setFont(KGlobalSettings::smallestReadableFont());
        painter->setPen(highlightColor.darker());

        QHashIterator<QString, ActionInfo> i(d->actions);
        while (i.hasNext()) {
            i.next();

            if (i.key() == d->hoveredAction) {
                painter->setBrush(highlightColor.lighter(350));
            } else {
                painter->setBrush(highlightColor.lighter(280));
            }

            const ActionInfo &action = i.value();
            painter->drawRoundedRect(action.rect, 3, 3);
            painter->drawText(action.rect, Qt::AlignCenter, action.name);
        }

        painter->restore();
    }

    QAbstractTextDocumentLayout::PaintContext paintContext;
    paintContext.palette.setColor(QPalette::Text, textColor);
    paintContext.clip = bodyRect;

    if (d->body.size().height() <= bodyRect.height()) {
        d->body.documentLayout()->draw(painter, paintContext);
        return;
    }

    QPixmap pixmap(bodyRect.size().toSize());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(painter->pen());
    p.setFont(painter->font());

    d->body.documentLayout()->draw(&p, paintContext);

    QLinearGradient alphaGradient(0, 0, 0, 1);
    alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
    alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));

    QRectF fadeOutRect = bodyRect;
    fadeOutRect.setTop(fadeOutRect.bottom() - fadeHeight);

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(fadeOutRect, alphaGradient);

    p.end();

    painter->drawPixmap(bodyRect.topLeft(), pixmap);
}


void NotifyTextItem::setBody(const QString &body)
{
    d->body.setHtml(body);
}


void NotifyTextItem::setActions(const QHash<QString, QString> &actions, const QStringList &actionOrder)
{
    setAcceptHoverEvents(!actions.isEmpty());
    d->hoveredAction = QString();

    d->actions.clear();
    d->actionOrder.clear();

    foreach (const QString &actionId, actionOrder) {
        if (!actions.contains(actionId)) {
            continue;
        }
        ActionInfo action;
        action.name = actions[actionId];
        d->actions.insert(actionId, action);
        d->actionOrder.append(actionId);
    }

    d->updateActionRects();
}


void NotifyTextItem::clearActions()
{
    setActions(QHash<QString, QString>(), QStringList());
}


void NotifyTextItem::Private::updateActionRects()
{
    QFont smallFont = KGlobalSettings::smallestReadableFont();
    QFontMetrics fm(smallFont);
    actionHeight = fm.height();

    QRectF rect = q->boundingRect();
    rect.setLeft(rect.right());
    rect.setTop(rect.bottom() - actionHeight - 2);

    foreach (const QString &actionId, actionOrder) {
        ActionInfo &action = actions[actionId];
        int width = fm.width(action.name) + 4;
        rect.setLeft(rect.left() - width - 4);
        rect.setWidth(width);
        action.rect = rect;
    }

    q->update();
}


void NotifyTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    d->clickedAction = d->hoveredAction;
}


void NotifyTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    if (d->hoveredAction == d->clickedAction && !d->clickedAction.isEmpty()) {
        emit actionInvoked(d->clickedAction);
    }
}


void NotifyTextItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QString hoveredAction;

    QHashIterator<QString, ActionInfo> i(d->actions);
    while (i.hasNext()) {
        i.next();
        if (i.value().rect.contains(event->pos())) {
            hoveredAction = i.key();
            break;
        }
    }

    if (hoveredAction == d->hoveredAction) {
        return;
    }

    d->hoveredAction = hoveredAction;
    if (hoveredAction.isNull()) {
        unsetCursor();
    } else {
        setCursor(Qt::PointingHandCursor);
    }

    update();
}

#include "notifytextitem.moc"
