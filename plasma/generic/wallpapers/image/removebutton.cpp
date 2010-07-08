/***************************************************************************
 *   Copyright (C) 2008 by Peter Penz <peter.penz@gmx.at>                  *
 *   Copyright (C) 2010 by Davide Bettio <davide.bettio@kdemail.net>       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "removebutton.h"

#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <klocale.h>

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QTimer>
#include <QTimeLine>

RemoveButton::RemoveButton(QWidget* parent) :
    QAbstractButton(parent),
    m_isHovered(false),
    m_leftMouseButtonPressed(false),
    m_fadingValue(0),
    m_icon(),
    m_fadingTimeLine(0)
{
    setFocusPolicy(Qt::NoFocus);
    parent->installEventFilter(this);
    resize(sizeHint());
    connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)),
            this, SLOT(refreshIcon()));
    
    m_icon = KIconLoader::global()->loadIcon("edit-delete",
                                             KIconLoader::NoGroup,
                                             qMin(width(), height()));
    setToolTip(i18n("Remove from list"));
}

RemoveButton::~RemoveButton()
{
}

QSize RemoveButton::sizeHint() const
{
    return QSize(32, 32);
}

void RemoveButton::reset()
{
    m_itemName = "";
    hide();
}

void RemoveButton::setVisible(bool visible)
{
    QAbstractButton::setVisible(visible);

    stopFading();
    if (visible) {
        startFading();
    }

}

bool RemoveButton::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == parent()) {
        switch (event->type()) {
        case QEvent::Leave:
            hide();
            break;

        case QEvent::MouseMove:
            if (m_leftMouseButtonPressed) {
                // Don't forward mouse move events to the viewport,
                // otherwise a rubberband selection will be shown when
                // clicking on the selection removeButton and moving the mouse
                // above the viewport.
                return true;
            }
            break;

        default:
            break;
        }
    }

    return QAbstractButton::eventFilter(obj, event);
}

void RemoveButton::enterEvent(QEvent* event)
{
    QAbstractButton::enterEvent(event);

    // if the mouse cursor is above the selection removeButton, display
    // it immediately without fading timer
    m_isHovered = true;
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
    }
    m_fadingValue = 255;

    update();
}

void RemoveButton::leaveEvent(QEvent* event)
{
    QAbstractButton::leaveEvent(event);
    m_isHovered = false;
    update();
}

void RemoveButton::mousePressEvent(QMouseEvent* event)
{
    QAbstractButton::mousePressEvent(event);
    m_leftMouseButtonPressed = (event->buttons() & Qt::LeftButton);
}

void RemoveButton::mouseReleaseEvent(QMouseEvent* event)
{
    QAbstractButton::mouseReleaseEvent(event);
    m_leftMouseButtonPressed = (event->buttons() & Qt::LeftButton);
}

void RemoveButton::resizeEvent(QResizeEvent* event)
{
    QAbstractButton::resizeEvent(event);

    m_icon = KIconLoader::global()->loadIcon("edit-delete",
                                             KIconLoader::NoGroup,
                                             qMin(width(), height()));
    update();
}

void RemoveButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setClipRect(event->rect());

    // draw the icon overlay
    if (m_isHovered) {
        KIconEffect iconEffect;
        QPixmap activeIcon = iconEffect.apply(m_icon, KIconLoader::Desktop, KIconLoader::ActiveState);
        painter.drawPixmap(0, 0, activeIcon);
    } else {
        if (m_fadingValue < 255) {
            // apply an alpha mask respecting the fading value to the icon
            QPixmap icon = m_icon;
            QPixmap alphaMask(icon.width(), icon.height());
            const QColor color(m_fadingValue, m_fadingValue, m_fadingValue);
            alphaMask.fill(color);
            icon.setAlphaChannel(alphaMask);
            painter.drawPixmap(0, 0, icon);
        } else {
            // no fading is required
            painter.drawPixmap(0, 0, m_icon);
        }
    }
}

void RemoveButton::setFadingValue(int value)
{
    m_fadingValue = value;
    if (m_fadingValue >= 255) {
        Q_ASSERT(m_fadingTimeLine != 0);
        m_fadingTimeLine->stop();
    }
    update();
}

void RemoveButton::refreshIcon()
{
    m_icon = KIconLoader::global()->loadIcon("edit-delete",
                                             KIconLoader::NoGroup,
                                             qMin(width(), height()));
    update();
}

void RemoveButton::startFading()
{
    Q_ASSERT(m_fadingTimeLine == 0);

    const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    const int duration = animate ? 600 : 1;

    m_fadingTimeLine = new QTimeLine(duration, this);
    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));
    m_fadingTimeLine->setFrameRange(0, 255);
    m_fadingTimeLine->start();
    m_fadingValue = 0;
}

void RemoveButton::stopFading()
{
    if (m_fadingTimeLine != 0) {
        m_fadingTimeLine->stop();
        delete m_fadingTimeLine;
        m_fadingTimeLine = 0;
    }
    m_fadingValue = 0;
}

void RemoveButton::setItemName(const QString& name)
{
   m_itemName = name;
}

QString RemoveButton::itemName() const
{
   return m_itemName;
}

#include "removebutton.moc"
