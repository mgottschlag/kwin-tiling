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

#include <QGraphicsLayout>
#include <QPropertyAnimation>
#include <QTimer>

#include <KWindowSystem>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/FrameSvg>

static const uint hideTimeout = 15 * 1000;

StackDialog::StackDialog(QWidget *parent, Qt::WindowFlags f)
      : Plasma::Dialog(parent, f),
        m_applet(0),
        m_windowToTile(0),
        m_notificationStack(0),
        m_drawLeft(true),
        m_drawRight(true),
        m_autoHide(true)
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
    if (m_notificationStack) {
        disconnect(m_notificationStack, 0, this, 0);
    }

    m_notificationStack = stack;

    connect(m_notificationStack, SIGNAL(updateRequested()), this, SLOT(update()));
    connect(m_notificationStack, SIGNAL(hideRequested()), this, SLOT(hide()));
    connect(m_notificationStack, SIGNAL(moveRequested(const QPoint &)), this, SLOT(moveRequested(const QPoint &)));
}

void StackDialog::moveRequested(const QPoint &point)
{
    move(pos()+point);
}

NotificationStack *StackDialog::notificartionStack() const
{
    return m_notificationStack;
}

void StackDialog::setApplet(Plasma::Applet *applet)
{
    m_applet = applet;
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
    if (m_applet && m_windowToTile) {
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
    Plasma::Dialog::hideEvent(event);
}

void StackDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    adjustWindowToTilePos();
    Plasma::Dialog::resizeEvent(event);
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
    }

    return Plasma::Dialog::eventFilter(watched, event);
}

#include "stackdialog.moc"
