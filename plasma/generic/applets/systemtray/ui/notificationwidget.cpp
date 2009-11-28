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
#include <QAction>
#include <QLabel>

#include <KColorScheme>
#include <KPushButton>

#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/theme.h>
#include <plasma/widgets/pushbutton.h>
#include <Plasma/Label>

class NotificationWidgetPrivate
{
public:
    NotificationWidgetPrivate(NotificationWidget *q)
        : q(q),
          notification(0),
          destroyOnClose(true),
          autoHide(true),
          image(0),
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
    bool autoHide;

    QString message;
    Plasma::Label *body;
    Plasma::Label *image;
    QGraphicsLinearLayout *mainLayout;
    QGraphicsLinearLayout *labelLayout;
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
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    d->mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    d->labelLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->body = new Plasma::Label(this);

    d->labelLayout->addItem(d->body);
    d->mainLayout->addItem(d->labelLayout);

    if (notification) {
        d->notification = notification;

        connect(d->signalMapper, SIGNAL(mapped(const QString &)),
                d->notification, SLOT(triggerAction(const QString &)));
        connect(notification, SIGNAL(changed()),
                this, SLOT(updateNotification()));
        connect(notification, SIGNAL(destroyed()),
                this, SLOT(destroy()));
        connect(notification, SIGNAL(expired()),
                this, SLOT(destroy()));

        extenderItem->showCloseButton();
        QAction *closeAction = extenderItem->action("close");
        if (closeAction) {
            connect(closeAction, SIGNAL(triggered()),
                    notification, SLOT(deleteLater()));
        }



        d->updateNotification();
    } else {
        d->setTextFields(extenderItem->config().readEntry("applicationName", ""),
                         extenderItem->config().readEntry("summary", ""),
                         extenderItem->config().readEntry("message", ""));
        extenderItem->showCloseButton();
    }
}

NotificationWidget::~NotificationWidget()
{
    if (d->notification) {
        // we were destroyed by the user, and the notification still exists
        //d->notification->remove();
    }

    delete d;
}

void NotificationWidget::setAutoHide(bool autoHide)
{
    if (autoHide != d->autoHide) {
        if (autoHide) {
            connect(d->notification, SIGNAL(expired()),
                    this, SLOT(destroy()));
        } else {
            disconnect(d->notification, SIGNAL(expired()),
                       this, SLOT(destroy()));
        }
        d->autoHide = autoHide;
    }
}

bool NotificationWidget::autoHide() const
{
    return d->autoHide;
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

    //Don't show more than 8 lines
    //in the end it could be more than 8 lines depending on how much \n characters will be there
    QString processed = message.trimmed();
    QFontMetricsF fm(body->font());
    int totalWidth = qMax((qreal)200, body->boundingRect().width()) * 8;
    if (fm.width(processed) > totalWidth) {
        processed = fm.elidedText(processed, Qt::ElideRight, totalWidth);
    }

    processed.replace('\n', "<br>");
    body->setText(processed);
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
    layout->addStretch();
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

    mainLayout->addItem(actionsWidget);
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

    if (!notification->image().isNull()) {
        if (!image) {
            image = new Plasma::Label(q);
        }
        image->nativeWidget()->setPixmap(QPixmap::fromImage(notification->image()));
        image->setMinimumSize(notification->image().size());
        image->setMaximumSize(notification->image().size());
        labelLayout->insertItem(0, image);
    } else {
        labelLayout->setContentsMargins(0, 0, 0, 0);
        if (image) {
            image->deleteLater();
            image = 0;
        }
    }

    extenderItem->showCloseButton();

    //FIXME: this sounds wrong
    q->setPreferredHeight(mainLayout->effectiveSizeHint(Qt::MinimumSize).height());
}

void NotificationWidgetPrivate::destroy()
{
    Plasma::ExtenderItem *extenderItem = dynamic_cast<Plasma::ExtenderItem *>(q->parentItem());
    notification = 0;

    if (extenderItem->isDetached()) {
        completeDetach();
    } else {
        completeDetach();
        extenderItem->destroy();
    }
}

#include "notificationwidget.moc"
