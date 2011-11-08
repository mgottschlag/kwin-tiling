/***************************************************************************
 *   stacdialog.h                                                          *
 *   Copyright (C) 2010 Marco Martin <notmart@gmail.com>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "stackdialog.h"
#include "notificationstack.h"
#include "notificationwidget.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsLayout>
#include <QLayout>
#include <QPropertyAnimation>
#include <QTimer>

#include <KWindowSystem>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/FrameSvg>
#include <Plasma/WindowEffects>

static const uint hideTimeout = 15 * 1000;

StackDialog::StackDialog(QWidget *parent, Qt::WindowFlags f)
      : Plasma::Dialog(parent, f),
        m_applet(0),
        m_windowToTile(0),
        m_notificationStack(0),
        m_view(0),
        m_drawLeft(true),
        m_drawRight(true),
        m_autoHide(true),
        m_hasCustomPosition(true)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/extender-background");
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    KWindowSystem::setType(winId(), NET::Dock);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
}

StackDialog::~StackDialog()
{
}

void StackDialog::setNotificationStack(NotificationStack *stack)
{
    setGraphicsWidget(stack);

    if (!m_view && layout()) {
        m_view = qobject_cast<QGraphicsView *>(layout()->itemAt(0)->widget());
        if (m_view) {
            m_view->installEventFilter(this);
        }
    }

    if (m_notificationStack) {
        disconnect(m_notificationStack, 0, this, 0);
    }

    m_notificationStack = stack;

    connect(m_notificationStack, SIGNAL(updateRequested()), this, SLOT(update()));
    connect(m_notificationStack, SIGNAL(hideRequested()), this, SLOT(hide()));
}

NotificationStack *StackDialog::notificartionStack() const
{
    return m_notificationStack;
}

void StackDialog::setApplet(Plasma::Applet *applet)
{
    m_applet = applet;
    adjustPosition(QPoint(-1, -1));
}

Plasma::Applet *StackDialog::applet() const
{
    return m_applet;
}

void StackDialog::setWindowToTile(QWidget *widget)
{
    if (m_windowToTile) {
        m_windowToTile->removeEventFilter(this);
        delete m_windowToTileAnimation;
    }

    m_windowToTile = widget;
    m_windowToTile->installEventFilter(this);
    m_windowToTileAnimation = new QPropertyAnimation(m_windowToTile, "pos", this);
    m_windowToTileAnimation->setDuration(250);
    m_windowToTileAnimation->setEasingCurve(QEasingCurve::InOutQuad);
}

QWidget *StackDialog::windowToTile() const
{
    return m_windowToTile;
}

void StackDialog::setAutoHide(const bool autoHide)
{
    m_autoHide = autoHide;
}

bool StackDialog::autoHide() const
{
    return m_autoHide;
}

void StackDialog::adjustWindowToTilePos()
{
    if (m_applet && m_windowToTile && m_hasCustomPosition) {
        m_windowToTileAnimation->setStartValue(m_windowToTile->pos());

        if (isVisible()) {
            //FIXME assumption that y starts from 0
            if (m_applet->location() == Plasma::TopEdge || pos().y() < m_windowToTile->size().height()) {
                m_windowToTileAnimation->setEndValue(QPoint(m_windowToTile->pos().x(), geometry().bottom()));
            } else {
                m_windowToTileAnimation->setEndValue(QPoint(m_windowToTile->pos().x(), pos().y() - m_windowToTile->size().height()));
            }
        } else if (m_applet && m_applet->containment() && m_applet->containment()->corona()) {
            m_windowToTileAnimation->setEndValue(QPoint(m_applet->containment()->corona()->popupPosition(m_applet, m_windowToTile->size())));
        }
        m_windowToTileAnimation->start();
    }
}

void StackDialog::paintEvent(QPaintEvent *e)
{
    Plasma::Dialog::paintEvent(e);

    QPainter painter(this);
    if (m_notificationStack) {
        //FIXME: assumption++
        QGraphicsLayout *mainLayout = static_cast<QGraphicsLayout *>(m_notificationStack->layout());

        if (!m_notificationStack->currentNotificationWidget() || mainLayout->count() < 2) {
            return;
        }

        for (int i = 0; i < mainLayout->count(); ++i) {
            //assumption++ all items are NotificationWidget
            NotificationWidget *nw = static_cast<NotificationWidget *>(mainLayout->itemAt(i));

            //first element
            if (i == 0 && m_notificationStack->currentNotificationWidget() != nw) {
                continue;
            } else if (i == 0) {
                m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)(Plasma::FrameSvg::AllBorders&~Plasma::FrameSvg::TopBorder));
            //last element
            } else if (i == mainLayout->count()-1 && m_notificationStack->currentNotificationWidget() != nw) {
                continue;
            } else if (i == mainLayout->count()-1) {
                m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)Plasma::FrameSvg::AllBorders&~Plasma::FrameSvg::BottomBorder);
            //element under the active one
            } else if (m_notificationStack->currentNotificationWidget()->pos().y() < nw->pos().y()) {
                m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)Plasma::FrameSvg::AllBorders&~Plasma::FrameSvg::TopBorder);
            //element over the active one
            } else if (m_notificationStack->currentNotificationWidget()->pos().y() > nw->pos().y()) {
                m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)Plasma::FrameSvg::AllBorders&~Plasma::FrameSvg::BottomBorder);
            //active element
            } else {
                m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
            }

            qreal left, top, right, bottom;
            m_background->getMargins(left, top, right, bottom);
            m_background->resizeFrame(QSizeF(size().width(), nw->size().height() + top+bottom));

            int topMargin = contentsRect().top();

            if (!m_drawLeft) {
                m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)m_background->enabledBorders()&~Plasma::FrameSvg::LeftBorder);
            }
            if (!m_drawRight) {
                m_background->setEnabledBorders((Plasma::FrameSvg::EnabledBorders)m_background->enabledBorders()&~Plasma::FrameSvg::RightBorder);
            }

            m_background->paintFrame(&painter, QPointF(0, nw->pos().y() - top + topMargin));
        }
    }
}

void StackDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)

    adjustPosition(adjustedSavedPos());

    adjustWindowToTilePos();

    if (m_autoHide) {
        m_hideTimer->start(hideTimeout);
    }

    adjustWindowToTilePos();
    Plasma::Dialog::showEvent(event);
}

void StackDialog::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)

    m_hideTimer->stop();

    adjustWindowToTilePos();

    m_notificationStack->adjustSize();
    Plasma::Dialog::hideEvent(event);
}

void StackDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    adjustWindowToTilePos();
    Plasma::Dialog::resizeEvent(event);
    if (m_hasCustomPosition) {
        adjustPosition(pos());
    } else {
        move(m_applet->containment()->corona()->popupPosition(m_applet, size()));
    }
}

void StackDialog::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)

    adjustWindowToTilePos();
    Plasma::Dialog::moveEvent(event);
}

void StackDialog::enterEvent(QEvent *event)
{
    Q_UNUSED(event)

    m_hideTimer->stop();
}

void StackDialog::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)

    if (m_autoHide) {
        m_hideTimer->start(hideTimeout);
    }
    Plasma::Dialog::leaveEvent(event);
}

void StackDialog::adjustPosition(const QPoint &pos)
{
    if (!m_applet) {
        return;
    }

    QPoint customPosition = pos;

    const QPoint popupPosition = m_applet->containment()->corona()->popupPosition(m_applet, size());

    if ((customPosition == QPoint(-1, -1))) {
        move(popupPosition);
        Plasma::WindowEffects::slideWindow(this, m_applet->location());
        m_hasCustomPosition = false;
    } else {
        if (m_applet->containment() &&
            m_applet->containment()->corona() &&
            m_notificationStack) {
            QRect screenRect = m_applet->containment()->corona()->availableScreenRegion(m_applet->containment()->screen()).boundingRect();

            customPosition.rx() = qMax(customPosition.x(), screenRect.left());
            customPosition.ry() = qMax(customPosition.y(), screenRect.top());
            customPosition.rx() = qMin(customPosition.x() + size().width(), screenRect.right()) - size().width();
            customPosition.ry() = qMin(customPosition.y() + size().height(), screenRect.bottom()) - size().height();

            bool closerToBottom = (customPosition.ry() > (screenRect.height() / 2));
            if (!m_lastSize.isNull() && closerToBottom
                && (m_lastSize.height() > size().height())) {
                customPosition.ry() += m_lastSize.height() - size().height();
            }
            m_lastSize = size();
        }

        move(customPosition);
        Plasma::WindowEffects::slideWindow(this, Plasma::Desktop);
        m_hasCustomPosition = true;
    }
}

void StackDialog::savePosition(const QPoint& pos)
{
    QByteArray horizSide, vertSide;
    QPoint pixelsToSave;
    QDesktopWidget widget;
    const QRect realScreenRect = widget.screenGeometry(m_applet->containment()->screen());

    int screenRelativeX = pos.x() - realScreenRect.x();
    int diffWithRight = realScreenRect.width() - (screenRelativeX + size().width());
    if (screenRelativeX < diffWithRight) {
        horizSide = "l";
        pixelsToSave.rx() = screenRelativeX;
    } else {
        horizSide = "r";
        pixelsToSave.rx() = diffWithRight;
    }

    int screenRelativeY = pos.y() - realScreenRect.y();
    int diffWithBottom = realScreenRect.height() - (screenRelativeY + size().height());
    if (screenRelativeY < diffWithBottom) {
        vertSide = "t";
        pixelsToSave.ry() = screenRelativeY;
    } else {
        vertSide = "b";
        pixelsToSave.ry() = diffWithBottom;
    }

    kDebug() << "Affinity-v" << vertSide;
    kDebug() << "Affinity-h" << horizSide;
    kDebug() << "Y: " << pixelsToSave.ry();
    kDebug() << "X: " << pixelsToSave.rx();

    const QPoint popupPosition = m_applet->containment()->corona()->popupPosition(m_applet, size());

    m_applet->config().writeEntry("customPosition", pixelsToSave);
    m_applet->config().writeEntry("customPositionAffinityHoriz", horizSide);
    m_applet->config().writeEntry("customPositionAffinityVert", vertSide);
}

QPoint StackDialog::adjustedSavedPos() const
{
    QPoint pos = m_applet->config().readEntry("customPosition", QPoint(-1, -1));

    if (pos != QPoint(-1, -1)) {
        QDesktopWidget widget;
        const QRect realScreenRect = widget.screenGeometry(m_applet->containment()->screen());
        QByteArray horizSide = m_applet->config().readEntry("customPositionAffinityHoriz").toLatin1();
        QByteArray vertSide = m_applet->config().readEntry("customPositionAffinityVert").toLatin1();

        if (horizSide == "l") {
            pos.rx() += realScreenRect.x();
        } else {
            pos.rx() = realScreenRect.x() + (realScreenRect.width() - pos.rx() - size().width());
        }

        if (vertSide == "t") {
            pos.ry() += realScreenRect.y();
        } else {
            pos.ry() = realScreenRect.y() + (realScreenRect.height() - pos.ry() - size().height());
        }
    }
    return pos;
}

bool StackDialog::event(QEvent *event)
{
    bool ret = Dialog::event(event);

    if (event->type() == QEvent::ContentsRectChange) {
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);

        m_drawLeft = (left != 0);
        m_drawRight = (right != 0);
        update();
    }

    return ret;
}

bool StackDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (m_windowToTile && watched == m_windowToTile &&
        event->type() == QEvent::Show && isVisible()) {
        adjustWindowToTilePos();
    } else if (watched == m_notificationStack && event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        m_dragPos = me->pos().toPoint();
    } else if (watched == m_notificationStack && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        adjustPosition(me->screenPos() - m_dragPos);
    } else if (watched == m_notificationStack && event->type() == QEvent::GraphicsSceneMouseRelease) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        adjustPosition(me->screenPos() - m_dragPos);
        savePosition(me->screenPos() - m_dragPos);
    }

    return Plasma::Dialog::eventFilter(watched, event);
}

#include "stackdialog.moc"
