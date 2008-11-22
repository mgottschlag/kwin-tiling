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

#include <QSignalMapper>

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QTextDocument>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QPainter>

#include <KColorScheme>
#include <KPushButton>
#include <KGlobalSettings>
#include <KIcon>
#include <KLocalizedString>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/theme.h>
#include <plasma/widgets/pushbutton.h>

class NotificationWidgetPrivate
{
public:
    NotificationWidgetPrivate(NotificationWidget *q)
        : q(q),
          notification(0),
          destroyOnClose(true),
          body(new QGraphicsTextItem(q)),
          actionsWidget(0),
          signalMapper(new QSignalMapper(q))
    {
    }

    void setTextFields(const QString &applicationName, const QString &summary, const QString &message);
    void completeDetach();
    void updateActions();
    void updateNotification();
    void destroy();

    NotificationWidget *q;

    SystemTray::Notification *notification;
    bool destroyOnClose;

    QString message;
    QGraphicsTextItem *body;
    QGraphicsWidget *actionsWidget;
    QHash<QString, QString> actions;
    QStringList actionOrder;

    QSignalMapper *signalMapper;
};

NotificationWidget::NotificationWidget(SystemTray::Notification *notification, Plasma::ExtenderItem *extenderItem)
    : QGraphicsWidget(extenderItem),
      d(new NotificationWidgetPrivate(this))
{
    setMinimumWidth(300);
    setPreferredWidth(400);

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    d->body->setFont(theme->font(Plasma::Theme::DefaultFont));
    d->body->setDefaultTextColor(theme->color(Plasma::Theme::TextColor));

    QTextOption option = d->body->document()->defaultTextOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    d->body->document()->setDefaultTextOption(option);

    if (notification) {
        d->notification = notification;

        connect(d->signalMapper, SIGNAL(mapped(const QString &)),
                d->notification, SLOT(triggerAction(const QString &)));
        connect(notification, SIGNAL(changed()),
                this, SLOT(updateNotification()));
        connect(notification, SIGNAL(destroyed()),
                this, SLOT(destroy()));

        d->updateNotification();
    } else {
        d->setTextFields(extenderItem->config().readEntry("applicationName", ""),
                         extenderItem->config().readEntry("summary", ""),
                         extenderItem->config().readEntry("message", ""));
        qreal bodyHeight = d->body->boundingRect().height();
        setPreferredHeight(bodyHeight);
        extenderItem->showCloseButton();
    }
}

NotificationWidget::~NotificationWidget()
{
    delete d;
}

void NotificationWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->body->setTextWidth(event->newSize().width());
    d->body->setPos(0, 0);
    if (d->actionsWidget) {
        d->actionsWidget->setPos(event->newSize().width() - d->actionsWidget->size().width(),
                                 event->newSize().height() - d->actionsWidget->size().height());
    }
}

void NotificationWidgetPrivate::setTextFields(const QString &applicationName,
                                                const QString &summary, const QString &message)
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem*>(q->parentWidget());

    if (!summary.isEmpty()) {
        extenderItem->setTitle(summary);
    } else {
        extenderItem->setTitle(i18n("Notification from %1", applicationName));
    }

    body->setHtml(message);
}

void NotificationWidgetPrivate::completeDetach()
{
    actions.clear();
    actionOrder.clear();

    delete actionsWidget;
    actionsWidget = 0;
}

void NotificationWidgetPrivate::updateActions()
{
    if (actions.isEmpty() || actionsWidget) {
        return;
    }

    actionsWidget = new QGraphicsWidget(q);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(actionsWidget);
    layout->setOrientation(Qt::Horizontal);
    actionsWidget->setContentsMargins(0, 0, 0, 0);

    foreach (const QString &actionId, actionOrder) {
        Plasma::PushButton *button = new Plasma::PushButton(actionsWidget);
        QString &action = actions[actionId];
        button->setText(action);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        //TODO: we need smaller buttons but I don't like this method of accomplishing it.
        button->setPreferredHeight(button->minimumHeight() - 6);

        q->connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(button, actionId);

        layout->addItem(button);
    }

    layout->updateGeometry();
    actionsWidget->setPos(q->size().width() - actionsWidget->size().width(),
                          q->size().width() - actionsWidget->size().height());
}

void NotificationWidgetPrivate::updateNotification()
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem*>(q->parentWidget());

    //store the notification
    extenderItem->config().writeEntry("applicationName", notification->applicationName());
    extenderItem->config().writeEntry("summary", notification->summary());
    extenderItem->config().writeEntry("message", notification->message());

    //set text fields and icon
    setTextFields(notification->applicationName(), notification->summary(), notification->message());
    extenderItem->setIcon(notification->applicationIcon());

    //set the actions provided
    actions = notification->actions();
    actionOrder = notification->actionOrder();
    updateActions();

    //set the correct size hint and display a close action if no actions are provided by the
    //notification
    qreal bodyHeight = body->boundingRect().height();
    if (actionsWidget) {
        extenderItem->hideCloseButton();
        q->setPreferredHeight(bodyHeight + actionsWidget->size().height());
    } else {
        extenderItem->showCloseButton();
        q->setPreferredHeight(bodyHeight);
    }
}

void NotificationWidgetPrivate::destroy()
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem *>(q->parentItem());

    if (extenderItem->isDetached()) {
        completeDetach();
    } else {
        completeDetach();
        extenderItem->destroy();
    }

    notification = 0;
}

#include "notificationwidget.moc"
