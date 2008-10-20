/***************************************************************************
 *   notificationwidget.cpp                                                *
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

#include "notificationwidget.h"
#include "notifytextitem.h"

#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QPainter>

#include <KGlobalSettings>
#include <KIcon>
#include <KLocalizedString>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>


static const int ICON_SIZE = 48;
static const int HOR_SPACING = 5;


int NotificationWidget::desiredMinimumHeight()
{
    QFont smallFont = KGlobalSettings::smallestReadableFont();
    QFontMetrics fm(smallFont);

    const int MARGIN = 4;

    return MARGIN * 2 + fm.height() * 3;
}


class NotificationWidget::Private
{
public:
    Private(NotificationWidget *q)
        : q(q),
          notification(0),
          textWidget(0),
          destroyOnClose(true)
    {
    }

    void updateLayout(const QSizeF &newSize);
    void setTextFields(const QString &applicationName, const QString &summary, const QString &message);
    void completeDetach();

    NotificationWidget *q;

    KIcon applicationIcon;
    SystemTray::Notification *notification;
    NotifyTextItem *textWidget;
    bool destroyOnClose;
};


NotificationWidget::NotificationWidget(SystemTray::Notification *notification, Plasma::ExtenderItem *extenderItem)
    : QGraphicsWidget(extenderItem),
      d(new Private(this))
{
    extenderItem->setIcon("preferences-desktop-notification");
    setMinimumSize(QSizeF(275, desiredMinimumHeight()));
    d->textWidget = new NotifyTextItem(this);

    if (notification) {
        d->notification = notification;

        connect(notification, SIGNAL(changed()),
                this, SLOT(updateNotification()));
        connect(notification, SIGNAL(destroyed()),
                this, SLOT(destroy()));
        connect(d->textWidget, SIGNAL(actionInvoked(const QString&)),
                this, SLOT(checkAction(const QString&)));
        connect(extenderItem->extender(), SIGNAL(itemDetached(Plasma::ExtenderItem*)),
                this, SLOT(removeCloseActionIfSelf(Plasma::ExtenderItem*)));

        updateNotification();
    } else {
        d->setTextFields(extenderItem->config().readEntry("applicationName", ""),
                         extenderItem->config().readEntry("summary", ""),
                         extenderItem->config().readEntry("message", ""));
    }
}


NotificationWidget::~NotificationWidget()
{
    delete d;
}


void NotificationWidget::updateNotification()
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem*>(parentWidget());

    extenderItem->config().writeEntry("applicationName", d->notification->applicationName());
    extenderItem->config().writeEntry("summary", d->notification->summary());
    extenderItem->config().writeEntry("message", d->notification->message());

    d->setTextFields(d->notification->applicationName(), d->notification->summary(), d->notification->message());
    d->applicationIcon = KIcon(d->notification->applicationIcon());

    if (!d->notification->actions().isEmpty() || !d->destroyOnClose) {
        d->textWidget->setActions(d->notification->actions(), d->notification->actionOrder());
    } else {
        QHash<QString, QString> actions;
        QStringList actionOrder;
        actions.insert("close", i18n("Close"));
        actionOrder.append("close");
        d->textWidget->setActions(actions, actionOrder);
    }

    d->updateLayout(size());
}


void NotificationWidget::Private::setTextFields(const QString &applicationName, const QString &summary, const QString &message)
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem*>(q->parentWidget());

    if (!summary.isEmpty()) {
        extenderItem->setTitle(summary);
    } else {
        extenderItem->setTitle(i18n("Notification from %1", applicationName));
    }

    textWidget->setBody(message);
}


void NotificationWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawPixmap(0, 5, d->applicationIcon.pixmap(ICON_SIZE));
}


void NotificationWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->updateLayout(event->newSize());
}


void NotificationWidget::Private::updateLayout(const QSizeF &newSize)
{
    int bodyRectWidth = newSize.width() - ICON_SIZE - HOR_SPACING;
    int bodyRectHeight = newSize.height();

    if (textWidget) {
        textWidget->setSize(bodyRectWidth, bodyRectHeight);
        textWidget->setPos(ICON_SIZE + HOR_SPACING, 0);
    }

    q->update();
}


void NotificationWidget::destroy()
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem *>(parentItem());

    if (d->destroyOnClose) {
        extenderItem->destroy();
    } else {
        d->completeDetach();
    }

    d->notification = 0;
}


void NotificationWidget::checkAction(const QString &actionId)
{
    if (d->notification->actions().contains(actionId)) {
        d->notification->triggerAction(actionId);
    } else {
        destroy();
    }
}


void NotificationWidget::removeCloseActionIfSelf(Plasma::ExtenderItem *extenderItem)
{
    if (extenderItem != parentWidget()) {
        return;
    }

    d->destroyOnClose = false;
    if (d->notification->actions().isEmpty()) {
        d->completeDetach();
    }
}


void NotificationWidget::Private::completeDetach()
{
    textWidget->clearActions();
    applicationIcon = KIcon();
    q->update();
}


#include "notificationwidget.moc"
