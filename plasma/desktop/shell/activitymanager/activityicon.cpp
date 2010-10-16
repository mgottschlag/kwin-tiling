/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activityicon.h"

#include "activity.h"
#include "desktopcorona.h"
#include "plasmaapp.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsGridLayout>
#include <QPainter>
#include <QCursor>

#include <KIconLoader>
#include <KIcon>

#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/LineEdit>
#include <Plasma/IconWidget>

ActivityIcon::ActivityIcon(const QString &id)
    :AbstractIcon(0),
    m_removeIcon("edit-delete"),
    m_stopIcon("media-playback-stop"),
    m_playIcon("media-playback-start"),
    m_configureIcon("configure"),
    m_removable(true),
    m_inlineWidgetAnim(0)
{
    DesktopCorona *c = qobject_cast<DesktopCorona*>(PlasmaApp::self()->corona());
    m_activity = c->activity(id);
    connect(this, SIGNAL(clicked(Plasma::AbstractIcon*)), m_activity, SLOT(activate()));
    connect(m_activity, SIGNAL(opened()), this, SLOT(repaint()));
    connect(m_activity, SIGNAL(closed()), this, SLOT(repaint()));
    connect(m_activity, SIGNAL(nameChanged(QString)), this, SLOT(setName(QString)));
    setName(m_activity->name());
}

ActivityIcon::~ActivityIcon()
{
}

QPixmap ActivityIcon::pixmap(const QSize &size)
{
    return m_activity ? m_activity->pixmap(size) : QPixmap();
}

QMimeData* ActivityIcon::mimeData()
{
    //TODO: how shall we use d&d?
    return 0;
}

void ActivityIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractIcon::paint(painter, option, widget);

    if (!m_activity) {
        return;
    }

    //qreal l, t, r, b;
    //getContentsMargins(&l, &t, &r, &b);
    //kDebug() << preferredSize() << geometry() << contentsRect() << l << t << r << b;
    QRectF rect = contentsRect();

    rect.adjust(0, rect.height() - iconSize(), 0, 0);

    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax<double>(0.0, (rect.width() - iconSize()) / 2.0); //icon's centered

    if (m_activity->isRunning()) {
        if (m_removable) {
            //draw stop icon
            qreal stopX = iconX + iconSize() - cornerIconSize.width();
            qreal stopY = rect.y();
            painter->drawPixmap(stopX, stopY, m_stopIcon.pixmap(cornerIconSize));
        }
    } else {
        //draw play icon, centered
        qreal playIconSize = KIconLoader::SizeMedium;
        qreal offset = (iconSize() - playIconSize) / 2.0;
        qreal playX = iconX + offset;
        qreal playY = rect.y() + offset;
        painter->drawPixmap(playX, playY, m_playIcon.pixmap(playIconSize));

        if (m_removable) {
            //the Remove corner-button
            qreal removeX = iconX + iconSize() - cornerIconSize.width();
            qreal removeY = rect.y();
            painter->drawPixmap(removeX, removeY, m_removeIcon.pixmap(cornerIconSize));
        }

    }

    qreal configX = iconX + iconSize() - cornerIconSize.width();
    qreal configY = rect.bottom() - cornerIconSize.height();
    painter->drawPixmap(configX, configY, m_configureIcon.pixmap(cornerIconSize));
}

void ActivityIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_activity) {
        return;
    }

    //check whether one of our corner icons was clicked
    //FIXME this is duplicate code, should get cleaned up later
    QRectF rect = contentsRect();
    rect.adjust(0, rect.height() - iconSize(), 0, 0);

    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax<double>(0.0, (rect.width() / 2) - (iconSize() / 2));

    qreal removeX = iconX + iconSize() - cornerIconSize.width();
    qreal removeY = rect.y();
    QRectF removeRect(QPointF(removeX, removeY), cornerIconSize);
    if (m_removable && removeRect.contains(event->pos())) {
        if (m_activity->isRunning()) {
            m_activity->close();
        } else {
            showRemovalConfirmation();
        }
        return;
    }

    qreal configX = iconX + iconSize() - cornerIconSize.width();
    qreal configY = rect.bottom() - cornerIconSize.height();
    QRectF configRect(QPointF(configX, configY), cornerIconSize);
    if (configRect.contains(event->pos())) {
        showConfiguration();
        return;
    }

    AbstractIcon::mouseReleaseEvent(event);
}

class MakeRoomAnimation : public QAbstractAnimation
{
public:
    MakeRoomAnimation(ActivityIcon *icon, qreal addedWidth, QObject *parent)
        : QAbstractAnimation(parent),
          m_icon(icon),
          m_addWidth(addedWidth)
    {
        qreal l, t, b;
        m_icon->getContentsMargins(&l, &t, &m_startRMargin, &b);
        m_startWidth = icon->contentsRect().width();
    }

    int duration() const
    {
        return 100;
    }

    void updateCurrentTime(int currentTime)
    {
        qreal delta = m_addWidth * (currentTime / 100.0);
        qreal l, t, r, b;
        m_icon->getContentsMargins(&l, &t, &r, &b);
        if (currentTime == 0 && direction() == Backward) {
            m_icon->setContentsMargins(l, t, m_startRMargin, b);
            m_icon->setMinimumSize(0, 0);
            m_icon->setPreferredSize(l + m_startRMargin + m_startWidth, m_icon->size().height());
            m_icon->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        } else {
            QSize s(m_startWidth + l + m_startRMargin + delta, m_icon->size().height());
            m_icon->setContentsMargins(l, t, m_startRMargin + delta, b);
            m_icon->setMaximumSize(s);
            m_icon->setMinimumSize(s);
            m_icon->setPreferredSize(s);
        }

        m_icon->getContentsMargins(&l, &t, &r, &b);
        //kDebug() << currentTime << m_startWidth << m_addWidth << delta << m_icon->size() << l << t << r << b;
    }

private:
    ActivityIcon *m_icon;
    qreal m_startWidth;
    qreal m_startRMargin;
    qreal m_addWidth;
};

void ActivityIcon::showRemovalConfirmation()
{
    QGraphicsWidget *w = new QGraphicsWidget(this);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(w);
    layout->setOrientation(Qt::Vertical);
    layout->setContentsMargins(0, 0, 0, 0);
    w->setLayout(layout);

    Plasma::Label *l = new Plasma::Label(w);
    l->setText(i18n("Remove activity?"));
    l->setAlignment(Qt::AlignCenter);
    layout->addItem(l);

    Plasma::PushButton *p = new Plasma::PushButton(w);
    p->setText(i18n("Confirm Removal"));
    layout->addItem(p);
    connect(p, SIGNAL(clicked()), m_activity, SLOT(destroy()));

    p = new Plasma::PushButton(w);
    p->setText(i18n("Cancel Removal"));
    layout->addItem(p);
    connect(p, SIGNAL(clicked()), this, SLOT(cancelRemoval()));

    w->setMaximumSize(QSize(0, size().height()));
    w->adjustSize();
    w->setPos(contentsRect().topRight() + QPoint(4, 0));

    m_inlineWidget = w;
    QTimer::singleShot(0, this, SLOT(startInlineAnim()));
}

void ActivityIcon::showConfiguration()
{
    QGraphicsWidget *w = new QGraphicsWidget(this);
    QGraphicsGridLayout *layout = new QGraphicsGridLayout(w);

    layout->setContentsMargins(0, 0, 0, 0);
    w->setLayout(layout);

    Plasma::IconWidget * icon = new Plasma::IconWidget(w);
    icon->setIcon(KIcon("plasma"));
    icon->setMinimumIconSize(QSizeF(32, 32));
    icon->setPreferredIconSize(QSizeF(32, 32));
    // l->setText("###");

    Plasma::Label *labelName = new Plasma::Label(w);
    labelName->setText(i18n("Activity name"));

    Plasma::LineEdit *editName = new Plasma::LineEdit(w);

    Plasma::PushButton * buttonSave = new Plasma::PushButton(w);
    buttonSave->setText(i18n("Save"));
    // connect(p, SIGNAL(clicked()), m_activity, SLOT(destroy()));

    Plasma::PushButton * buttonCancel = new Plasma::PushButton(w);
    buttonCancel->setText(i18n("Cancel"));
    // connect(p, SIGNAL(clicked()), m_activity, SLOT(destroy()));

    // layout
    layout->addItem(icon, 0, 0, 2, 1);

    layout->addItem(labelName, 0, 1);
    layout->addItem(editName,  1, 1);

    layout->addItem(buttonSave,   0, 2);
    layout->addItem(buttonCancel, 1, 2);

    w->setMaximumSize(QSize(0, size().height()));
    w->adjustSize();
    w->setPos(contentsRect().topRight() + QPoint(4, 0));

    m_inlineWidget = w;
    QTimer::singleShot(0, this, SLOT(startInlineAnim()));
}

void ActivityIcon::startInlineAnim()
{
    QGraphicsWidget * w = m_inlineWidget.data();
    //kDebug() << "Booh yah!" << w;
    if (!w) {
        return;
    }

    //kDebug() << w->preferredSize() << w->layout()->preferredSize();
    if (!m_inlineWidgetAnim) {
        m_inlineWidgetAnim = new MakeRoomAnimation(this, w->layout()->preferredSize().width() + 4, this);
        connect(m_inlineWidgetAnim, SIGNAL(finished()), this, SLOT(makeInlineWidgetVisible()));
    }

    m_inlineWidgetAnim->start();
}

void ActivityIcon::cancelRemoval()
{
    if (m_inlineWidget) {
        m_inlineWidget.data()->deleteLater();
        m_inlineWidget.data()->hide();
    }

    if (m_inlineWidgetAnim) {
        m_inlineWidgetAnim->setDirection(QAbstractAnimation::Backward);
        if (m_inlineWidgetAnim->state() != QAbstractAnimation::Running) {
            m_inlineWidgetAnim->start(QAbstractAnimation::DeleteWhenStopped);
            m_inlineWidgetAnim = 0;
        }
    }
}

void ActivityIcon::makeInlineWidgetVisible()
{
    if (m_inlineWidget) {
        m_inlineWidget.data()->show();
    }
}

void ActivityIcon::repaint()
{
    update();
}

void ActivityIcon::setRemovable(bool removable)
{
    if (removable == m_removable) {
        return;
    }

    m_removable = removable;
    update();
}

Activity* ActivityIcon::activity()
{
    return m_activity;
}

void ActivityIcon::activityRemoved()
{
    m_activity = 0;
    deleteLater();
}

#include "activityicon.moc"

