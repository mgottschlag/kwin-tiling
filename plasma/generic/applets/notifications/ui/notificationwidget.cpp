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
#include <QtGui/QGraphicsGridLayout>
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
#include <KTextBrowser>

#include <Plasma/Animation>
#include <Plasma/Animator>
#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/TextBrowser>
#include <Plasma/Theme>

class NotificationWidgetPrivate
{
public:
    NotificationWidgetPrivate(NotificationWidget *q)
        : q(q),
          destroyOnClose(true),
          autoDelete(false),
          collapsed(false),
          icon(0),
          actionsWidget(0),
          signalMapper(new QSignalMapper(q))
    {
    }

    void setTextFields(const QString &applicationName, const QString &summary, const QString &message);
    void completeDetach();
    void updateActions();
    void updateNotification();
    void buttonClicked();
    void hideFinished();
    QRectF bigIconRect() const;

    NotificationWidget *q;

    QWeakPointer<Notification> notification;
    bool destroyOnClose;
    bool autoDelete;
    bool collapsed;

    QString message;
    Plasma::TextBrowser *messageLabel;
    Plasma::Label *title;
    Plasma::IconWidget *closeButton;
    Plasma::IconWidget *icon;
    QGraphicsLinearLayout *titleLayout;
    QGraphicsLinearLayout *mainLayout;
    QGraphicsGridLayout *bodyLayout;
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
    : QGraphicsWidget(parent),
      d(new NotificationWidgetPrivate(this))
{
    setFlag(QGraphicsItem::ItemHasNoContents, true);
    setMinimumWidth(300);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);


    d->iconPlaceSmall = new QGraphicsWidget(this);
    d->iconPlaceSmall->setMinimumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    d->iconPlaceSmall->setMaximumSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    d->icon = new Plasma::IconWidget(this);
    d->icon->setAcceptHoverEvents(false);
    d->icon->setAcceptedMouseButtons(Qt::NoButton);

    d->title = new Plasma::Label(this);
    d->title->setWordWrap(false);
    d->title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->title->setAlignment(Qt::AlignCenter);

    d->closeButton = new Plasma::IconWidget(this);
    d->closeButton->setSvg("widgets/configuration-icons", "close");
    d->closeButton->setMaximumSize(d->closeButton->sizeFromIconSize(KIconLoader::SizeSmall));
    d->closeButton->setMinimumSize(d->closeButton->maximumSize());
    connect(d->closeButton, SIGNAL(clicked()), notification, SLOT(deleteLater()));

    d->titleLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    d->titleLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->titleLayout->addItem(d->iconPlaceSmall);
    d->titleLayout->addItem(d->title);
    d->titleLayout->addItem(d->closeButton);



    d->body = new QGraphicsWidget(this);
    d->body->setContentsMargins(0,0,0,0);
    d->body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->bodyLayout = new QGraphicsGridLayout(d->body);
    d->bodyLayout->setSpacing(0);
    d->bodyLayout->setContentsMargins(0,0,0,0);

    d->messageLabel = new Plasma::TextBrowser(d->body);
    d->messageLabel->setPreferredWidth(0);
    d->messageLabel->nativeWidget()->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    d->messageLabel->nativeWidget()->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    d->messageLabel->nativeWidget()->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->messageLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(d->messageLabel->nativeWidget(), SIGNAL(urlClick(QString)),
            notification, SLOT(linkActivated(QString)));

    d->iconPlaceBig = new QGraphicsWidget(this);
    d->iconPlaceBig->setMaximumHeight(KIconLoader::SizeLarge);
    d->iconPlaceBig->setMinimumHeight(KIconLoader::SizeLarge);
    d->iconPlaceBig->setMaximumWidth(KIconLoader::SizeLarge);
    d->iconPlaceBig->setMinimumWidth(KIconLoader::SizeLarge);
    d->iconPlaceBig->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    d->bodyLayout->addItem(d->iconPlaceBig, 0, 0, Qt::AlignCenter);
    d->bodyLayout->addItem(d->messageLabel, 0, 1, Qt::AlignCenter);

    d->mainLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    d->mainLayout->setSpacing(0);
    d->mainLayout->addItem(d->titleLayout);
    d->mainLayout->addItem(d->body);

    d->notification = notification;

    connect(d->signalMapper, SIGNAL(mapped(QString)),
            notification, SLOT(triggerAction(QString)));
    connect(notification, SIGNAL(changed()),
            this, SLOT(updateNotification()));
    connect(notification, SIGNAL(destroyed()),
            this, SLOT(deleteLater()));

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

    setTitleBarVisible(true);

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

    if (collapse) {
        d->messageLabel->nativeWidget()->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    } else {
        d->messageLabel->nativeWidget()->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    }

    d->body->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);

    d->collapsed = collapse;
}

Notification *NotificationWidget::notification() const
{
    return d->notification.data();
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

void NotificationWidget::setTitleBarVisible(bool visible)
{
    if (visible) {
        d->iconPlaceSmall->show();
        d->title->show();
        d->closeButton->show();
        d->titleLayout->setMaximumHeight(QWIDGETSIZE_MAX);
    } else {
        d->iconPlaceSmall->hide();
        d->title->hide();
        d->closeButton->hide();
        d->titleLayout->setMaximumHeight(0);
    }
}

bool NotificationWidget::isTitleBarVisible() const
{
    return d->title->isVisible();
}

void NotificationWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget::resizeEvent(event);
    if (d->icon && !d->collapsed && d->animationGroup->state() != QAbstractAnimation::Running) {
        d->icon->setGeometry(d->bigIconRect());
    }
}

QRectF NotificationWidgetPrivate::bigIconRect() const
{
    return q->mapFromScene(iconPlaceBig->mapToScene(iconPlaceBig->boundingRect())).boundingRect();
}

void NotificationWidgetPrivate::setTextFields(const QString &applicationName,
                                                const QString &summary, const QString &message)
{
    if (!summary.isEmpty()) {
        title->setText(summary);
    } else {
        title->setText(i18n("Notification from %1", applicationName));
    }


    QString processed = message.trimmed();

    /*if there is a < that is not closed as a tag, replace it with an entity*/
    processed = processed.replace(QRegExp("<(?![^<]*>)"), "&lt;");
    processed.replace('\n', "<br>");

    QFontMetricsF fm(messageLabel->font());
    qreal maxLine = messageLabel->rect().width();

    QString parsed;

    QString::const_iterator i = processed.begin();
    bool inTag = false;
    QString word;
    QString sentence;

    while (i != processed.end()) {
        QChar c = *i;

        if (c == '<') {
            inTag = true;
            sentence.append(word);
            parsed.append(fm.elidedText(sentence, Qt::ElideRight, maxLine*4.6));
            sentence = QString();
            word = QString();
            word.append(c);
        } else if (c == '>') {
            word.append(c);
            if (!sentence.isEmpty()) {
                parsed.append(fm.elidedText(sentence, Qt::ElideRight, maxLine*4.6));
                sentence.clear();
            }
            inTag = false;
            parsed.append(word);
            word = QString();
        } else if (c == ' ') {
            word.append(c);
            if (inTag) {
                parsed.append(word);
            } else {
                sentence.append(word);
            }
            word = QString();
        } else {
            word.append(c);
        }

        ++i;
    }

    sentence.append(word);
    parsed.append(fm.elidedText(sentence, Qt::ElideRight, maxLine*4.6));

    messageLabel->setText(QLatin1String("<html>") + parsed + QLatin1String("</html>"));

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
    layout->setContentsMargins(0, 0, 0, 0);
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
    layout->addStretch();
    layout->activate();

    if (actionsWidget->size().width() > q->size().width() * 0.4) {
        layout->setOrientation(Qt::Horizontal);
        bodyLayout->addItem(actionsWidget, 1, 0, 1, 2, Qt::AlignCenter);
    } else {
        bodyLayout->addItem(actionsWidget, 0, 2, Qt::AlignCenter);
    }
}

void NotificationWidgetPrivate::buttonClicked()
{
    //a decsion has already been taken
    if (actionsWidget) {
        notification.data()->deleteLater();
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

        if (imageSize.width() > KIconLoader::SizeLarge || imageSize.height() > KIconLoader::SizeLarge) {
            imageSize.scale(KIconLoader::SizeLarge, KIconLoader::SizeLarge, Qt::KeepAspectRatio);
        }

        icon->setMaximumIconSize(QSizeF(qMin((int)KIconLoader::SizeLarge, imageSize.width()),
                                  qMin((int)KIconLoader::SizeLarge, imageSize.height())));
    }


    //FIXME: this sounds wrong
    q->setPreferredHeight(mainLayout->effectiveSizeHint(Qt::MinimumSize).height());
}

void NotificationWidgetPrivate::hideFinished()
{
    body->setVisible(!collapsed);
    body->setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
}

#include "notificationwidget.moc"
