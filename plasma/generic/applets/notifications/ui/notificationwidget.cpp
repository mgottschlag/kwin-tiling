/***************************************************************************
 *   notificationwidget.cpp                                                *
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

#include "notificationwidget.h"

#include <QSignalMapper>

#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QTextDocument>
#include <QtGui/QFontMetrics>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QPainter>
#include <QAction>
#include <QLabel>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include <KColorScheme>
#include <KPushButton>

#include <plasma/extender.h>
#include <plasma/theme.h>
#include <plasma/widgets/pushbutton.h>
#include <Plasma/Animator>
#include <Plasma/Animation>
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
          collapsed(false),
          backgroundVisible(true),
          icon(0),
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
    void hideFinished();
    QRectF bigIconRect() const;

    NotificationWidget *q;

    QWeakPointer<Notification> notification;
    bool destroyOnClose;
    bool autoDelete;
    bool collapsed;
    bool backgroundVisible;

    QString message;
    Plasma::Label *messageLabel;
    Plasma::Label *title;
    Plasma::IconWidget *icon;
    QGraphicsLinearLayout *titleLayout;
    QGraphicsLinearLayout *mainLayout;
    QGraphicsLinearLayout *bodyLayout;
    QGraphicsWidget *body;
    QGraphicsWidget *iconPlaceSmall;
    QGraphicsWidget *iconPlaceBig;
    QGraphicsWidget *actionsWidget;
    QHash<QString, QString> actions;
    QStringList actionOrder;
    QPropertyAnimation *hideAnimation;
    QPropertyAnimation *iconAnimation;
    QParallelAnimationGroup *animationGroup;

    QSignalMapper *signalMapper;
};

NotificationWidget::NotificationWidget(Notification *notification, QGraphicsWidget *parent)
    : Plasma::Frame(parent),
      d(new NotificationWidgetPrivate(this))
{
    setMinimumWidth(350);
    setPreferredWidth(400);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    d->iconPlaceSmall = new QGraphicsWidget(this);
    d->iconPlaceSmall->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    d->iconPlaceSmall->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    d->icon = new Plasma::IconWidget(this);
    d->icon->setAcceptHoverEvents(false);
    d->icon->setAcceptedMouseButtons(Qt::NoButton);

    d->title = new Plasma::Label(this);
    d->title->setWordWrap(false);
    d->title->setAlignment(Qt::AlignCenter);

    Plasma::IconWidget *closeButton = new Plasma::IconWidget(this);
    closeButton->setSvg("widgets/configuration-icons", "close");
    closeButton->setMaximumSize(closeButton->sizeFromIconSize(KIconLoader::SizeSmall));
    closeButton->setMinimumSize(closeButton->maximumSize());
    connect(closeButton, SIGNAL(clicked()), notification, SLOT(deleteLater()));

    d->titleLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->titleLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->titleLayout->addItem(d->iconPlaceSmall);
    d->titleLayout->addItem(d->title);
    d->titleLayout->addItem(closeButton);



    d->body = new QGraphicsWidget(this);
    d->body->setContentsMargins(0,0,0,0);
    d->body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->bodyLayout = new QGraphicsLinearLayout(Qt::Horizontal, d->body);
    d->bodyLayout->setContentsMargins(0,0,0,0);

    d->messageLabel = new Plasma::Label(d->body);
    d->messageLabel->nativeWidget()->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    d->messageLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    connect(d->messageLabel, SIGNAL(linkActivated(const QString &)),
            notification, SLOT(linkActivated(const QString &)));
    d->messageLabel->nativeWidget()->setTextFormat(Qt::RichText);

    d->iconPlaceBig = new QGraphicsWidget(this);
    d->iconPlaceBig->setMaximumWidth(KIconLoader::SizeHuge);
    d->iconPlaceBig->setMinimumWidth(KIconLoader::SizeHuge);
    d->iconPlaceBig->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    d->bodyLayout->addItem(d->iconPlaceBig);
    d->bodyLayout->addItem(d->messageLabel);

    d->mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    d->mainLayout->setSpacing(0);
    d->mainLayout->addItem(d->titleLayout);
    d->mainLayout->addItem(d->body);


    d->notification = notification;

    connect(d->signalMapper, SIGNAL(mapped(const QString &)),
            d->notification.data(), SLOT(triggerAction(const QString &)));
    connect(notification, SIGNAL(changed()),
            this, SLOT(updateNotification()));
    connect(notification, SIGNAL(destroyed()),
            this, SLOT(destroy()));

    d->hideAnimation = new QPropertyAnimation(this, "bodyHeight", this);
    d->hideAnimation->setDuration(250);
    connect(d->hideAnimation, SIGNAL(finished()), this, SLOT(hideFinished()));

    d->iconAnimation = new QPropertyAnimation(d->icon, "geometry", d->icon);
    d->iconAnimation->setDuration(250);

    d->animationGroup = new QParallelAnimationGroup(this);
    d->animationGroup->addAnimation(d->hideAnimation);
    d->animationGroup->addAnimation(d->iconAnimation);

    d->updateNotification();

    d->mainLayout->activate();
    updateGeometry();
}

NotificationWidget::~NotificationWidget()
{
    delete d;
}

void NotificationWidget::setCollapsed(bool collapse, bool animate)
{
    if (collapse == d->collapsed) {
        return;
    }

    //use this weird way to make easy to animate
    if (animate) {
        setMinimumHeight(-1);
        if (collapse) {
            d->hideAnimation->setStartValue(d->body->size().height());
            d->hideAnimation->setEndValue(0);

            d->iconAnimation->setStartValue(d->icon->geometry());
            d->iconAnimation->setEndValue(d->iconPlaceSmall->geometry());
            d->animationGroup->start();
        } else {
            d->body->setVisible(true);
            d->hideAnimation->setStartValue(d->body->size().height());
            d->body->setMaximumHeight(-1);
            d->hideAnimation->setEndValue(d->body->effectiveSizeHint(Qt::PreferredSize).height());

            d->iconAnimation->setStartValue(d->icon->geometry());
            d->iconAnimation->setEndValue(d->bigIconRect());
            d->animationGroup->start();
        }
    } else {
        if (collapse) {
            //setMaximumHeight(d->titleLayout->geometry().bottom());
            d->body->setMinimumHeight(0);
            d->body->setMaximumHeight(0);
            d->body->hide();
            setMinimumHeight(-1);
            d->icon->setGeometry(d->iconPlaceSmall->geometry());
        } else {
            d->body->show();
            d->body->setMaximumHeight(-1);
            d->body->setMinimumHeight(-1);
            d->body->setMaximumHeight(d->body->effectiveSizeHint(Qt::PreferredSize).height());
            d->body->setMinimumHeight(d->body->maximumHeight());
            updateGeometry();
            d->mainLayout->invalidate();
            setMinimumHeight(sizeHint(Qt::PreferredSize, QSizeF()).height());

            d->icon->setGeometry(d->bigIconRect());
        }
    }

    d->body->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    d->collapsed = collapse;
}

qreal NotificationWidget::bodyHeight() const
{
    return d->body->maximumHeight();
}

void NotificationWidget::setBodyHeight(const qreal height)
{
    d->body->setMaximumHeight(height);
    d->body->setMinimumHeight(height);
    updateGeometry();
    d->mainLayout->invalidate();
    setMinimumHeight(sizeHint(Qt::PreferredSize, QSizeF()).height());
}

bool NotificationWidget::isCollapsed() const
{
    return d->collapsed;
}

void NotificationWidget::setBackgroundVisible(bool visible)
{
    d->backgroundVisible = visible;
    update();
}

bool NotificationWidget::isBackgroundVisible() const
{
    return d->backgroundVisible;
}

void NotificationWidget::paint(QPainter *painter,
                               const QStyleOptionGraphicsItem *option,
                               QWidget *widget)
{
    if (d->backgroundVisible) {
        Plasma::Frame::paint(painter, option, widget);
    }
}

void NotificationWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Frame::resizeEvent(event);
    if (d->icon && !d->collapsed) {
        d->icon->setGeometry(d->bigIconRect());
    }
}

QRectF NotificationWidgetPrivate::bigIconRect() const
{

    QRectF rect = q->mapFromScene(iconPlaceSmall->mapToScene(iconPlaceSmall->boundingRect())).boundingRect();

    return rect.united(q->mapFromScene(iconPlaceBig->mapToScene(iconPlaceBig->boundingRect())).boundingRect());
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
    QFontMetricsF fm(messageLabel->font());
    int totalWidth = qMax((qreal)200, messageLabel->boundingRect().width()) * 8;
    if (fm.width(processed) > totalWidth) {
        processed = fm.elidedText(processed, Qt::ElideRight, totalWidth);
    }

    //Step 2: elide too long words, like urls or whatever it can be here
    /*the meaning of this ugly regular expression is: all long words not quoted
      (like tag parameters) are cut at the first 20 characters. it would be better
      to do it with font metrics but wouldn't be possible to do it with a single
      regular expression. It would have to be tokenizen by hand, with some html
      parsing too (like, are we in a tag?)*/
    processed = processed.replace(QRegExp("([^\"])([^ ]{20})([^ ]+)([^\"])"), "\\1\\2...");

    /*if there is a < that is not closed as a tag, replace it with an entity*/
    processed = processed.replace(QRegExp("<([^>]*($|<))"), "&lt;\\1");

    processed.replace('\n', "<br>");
    messageLabel->setText(processed);

    if (!collapsed) {
        icon->setGeometry(bigIconRect());
    }
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

    actionsWidget = new QGraphicsWidget(body);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(actionsWidget);
    layout->setOrientation(Qt::Vertical);
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

    bodyLayout->addItem(actionsWidget);
}

void NotificationWidgetPrivate::buttonClicked()
{
    //a decsion has already been taken
    if (actionsWidget) {
        actionsWidget->hide();
    }
    emit q->actionTriggered(notification.data());
}

void NotificationWidgetPrivate::updateNotification()
{
    if (!notification) {
        return;
    }


    //set text fields and icon
    setTextFields(notification.data()->applicationName(), notification.data()->summary(), notification.data()->message());
    icon->setIcon(notification.data()->applicationIcon());
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    //set the actions provided
    actions = notification.data()->actions();
    actionOrder = notification.data()->actionOrder();
    updateActions();

    if (!notification.data()->image().isNull()) {

        icon->setIcon(QPixmap::fromImage(notification.data()->image()));

        QSize imageSize = notification.data()->image().size();

        if (imageSize.width() > KIconLoader::SizeHuge || imageSize.height() > KIconLoader::SizeHuge) {
            imageSize.scale(KIconLoader::SizeHuge, KIconLoader::SizeHuge, Qt::KeepAspectRatio);
        }

        icon->setMaximumIconSize(QSizeF(qMin((int)KIconLoader::SizeHuge, imageSize.width()),
                                  qMin((int)KIconLoader::SizeHuge, imageSize.height())));
    }


    //FIXME: this sounds wrong
    q->setPreferredHeight(mainLayout->effectiveSizeHint(Qt::MinimumSize).height());
}

void NotificationWidgetPrivate::destroy()
{
    Plasma::Animation *zoomAnim = Plasma::Animator::create(Plasma::Animator::ZoomAnimation);
    QObject::connect(zoomAnim, SIGNAL(finished()), q, SLOT(deleteLater()));
    zoomAnim->setTargetWidget(q);
    zoomAnim->start();
}

void NotificationWidgetPrivate::hideFinished()
{
    body->setVisible(!collapsed);
    body->setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
}

#include "notificationwidget.moc"
