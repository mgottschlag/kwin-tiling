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
#include <plasma/theme.h>
#include <plasma/widgets/pushbutton.h>
#include <Plasma/Label>
#include <Plasma/Frame>
#include <Plasma/IconWidget>

class NotificationWidgetPrivate
{
public:
    NotificationWidgetPrivate(NotificationWidget *q)
        : q(q),
          destroyOnClose(true),
          autoDelete(false),
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
    void buttonClicked();

    NotificationWidget *q;

    QWeakPointer<SystemTray::Notification> notification;
    bool destroyOnClose;
    bool autoDelete;

    QString message;
    Plasma::Label *body;
    Plasma::Label *image;
    Plasma::Label *title;
    Plasma::IconWidget *icon;
    QGraphicsLinearLayout *mainLayout;
    QGraphicsLinearLayout *labelLayout;
    QGraphicsWidget *actionsWidget;
    QHash<QString, QString> actions;
    QStringList actionOrder;

    QSignalMapper *signalMapper;
};

NotificationWidget::NotificationWidget(SystemTray::Notification *notification, QGraphicsWidget *parent)
    : Plasma::Frame(parent),
      d(new NotificationWidgetPrivate(this))
{
    setMinimumWidth(300);
    setPreferredWidth(400);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QGraphicsLinearLayout *titleLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->icon = new Plasma::IconWidget(this);
    d->icon->setMaximumSize(d->icon->sizeFromIconSize(KIconLoader::SizeSmall));
    d->icon->setMinimumSize(d->icon->maximumSize());
    d->title = new Plasma::Label(this);
    Plasma::IconWidget *closeButton = new Plasma::IconWidget(this);
    titleLayout->addItem(d->icon);
    titleLayout->addItem(d->title);
    titleLayout->addItem(closeButton);
    closeButton->setSvg("widgets/configuration-icons", "close");
    closeButton->setMaximumSize(closeButton->sizeFromIconSize(KIconLoader::SizeSmall));
    closeButton->setMinimumSize(closeButton->maximumSize());
    connect(closeButton, SIGNAL(clicked()), notification, SLOT(deleteLater()));

    d->mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    d->labelLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->body = new Plasma::Label(this);
    d->body->nativeWidget()->setTextFormat(Qt::RichText);

    d->mainLayout->addItem(titleLayout);
    d->labelLayout->addItem(d->body);
    d->mainLayout->addItem(d->labelLayout);


    d->notification = notification;

    connect(d->signalMapper, SIGNAL(mapped(const QString &)),
            d->notification.data(), SLOT(triggerAction(const QString &)));
    connect(notification, SIGNAL(changed()),
            this, SLOT(updateNotification()));
    connect(notification, SIGNAL(destroyed()),
            this, SLOT(destroy()));

    d->updateNotification();
}

NotificationWidget::~NotificationWidget()
{

    delete d;
}

void NotificationWidget::setAutoDelete(bool autoDelete)
{
    if (autoDelete != d->autoDelete) {
        if (autoDelete) {
            connect(d->notification.data(), SIGNAL(expired()),
                    this, SLOT(destroy()));
        } else {
            disconnect(d->notification.data(), SIGNAL(expired()),
                       this, SLOT(destroy()));
        }

        d->autoDelete = autoDelete;
    }
}

bool NotificationWidget::isAutoDelete() const
{
    return d->autoDelete;
}

void NotificationWidgetPrivate::setTextFields(const QString &applicationName,
                                                const QString &summary, const QString &message)
{
    if (!summary.isEmpty()) {
        title->setText(summary);
    } else {
        title->setText(i18n("Notification from %1", applicationName));
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
        q->connect(button, SIGNAL(clicked()), q, SLOT(buttonClicked()));
        signalMapper->setMapping(button, actionId);

        layout->addItem(button);
    }

    mainLayout->addItem(actionsWidget);
}

void NotificationWidgetPrivate::buttonClicked()
{
    //a decsion has already been taken
    if (actionsWidget) {
        actionsWidget->hide();
    }
}

void NotificationWidgetPrivate::updateNotification()
{
    if (!notification) {
        return;
    }

    //set text fields and icon
    setTextFields(notification.data()->applicationName(), notification.data()->summary(), notification.data()->message());
    icon->setIcon(notification.data()->applicationIcon());

    //set the actions provided
    actions = notification.data()->actions();
    actionOrder = notification.data()->actionOrder();
    updateActions();

    if (!notification.data()->image().isNull()) {
        if (!image) {
            image = new Plasma::Label(q);
            image->setScaledContents(true);
        }
        image->nativeWidget()->setPixmap(QPixmap::fromImage(notification.data()->image()));

        QSize imageSize = notification.data()->image().size();

        if (imageSize.width() > KIconLoader::SizeHuge || imageSize.height() > KIconLoader::SizeHuge) {
            imageSize.scale(KIconLoader::SizeHuge, KIconLoader::SizeHuge, Qt::KeepAspectRatio);
        }

        image->setMinimumSize(imageSize);
        image->setMaximumSize(imageSize);
        labelLayout->insertItem(0, image);
    } else {
        labelLayout->setContentsMargins(0, 0, 0, 0);
        if (image) {
            image->deleteLater();
            image = 0;
        }
    }


    //FIXME: this sounds wrong
    q->setPreferredHeight(mainLayout->effectiveSizeHint(Qt::MinimumSize).height());
}

void NotificationWidgetPrivate::destroy()
{
    q->deleteLater();
}

#include "notificationwidget.moc"
