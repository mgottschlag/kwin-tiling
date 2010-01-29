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

#include <Plasma/FrameSvg>

StackDialog::StackDialog(QWidget *parent, Qt::WindowFlags f)
      : Plasma::Dialog(parent, f),
        m_notificationStack(0),
        m_drawLeft(true),
        m_drawRight(true)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/extender-background");
}

StackDialog::~StackDialog()
{
}

void StackDialog::setNotificationStack(SystemTray::NotificationStack *stack)
{
    setGraphicsWidget(stack);
    if (m_notificationStack) {
        disconnect(m_notificationStack, SIGNAL(updateRequested()), this, SLOT(update()));
    }

    m_notificationStack = stack;

    connect(m_notificationStack, SIGNAL(updateRequested()), this, SLOT(update()));
}

SystemTray::NotificationStack *StackDialog::notificartionStack() const
{
    return m_notificationStack;
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

#include "stackdialog.moc"