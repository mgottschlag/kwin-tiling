/*
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "ui/tabbar.h"

// KDE
#include <KGlobalSettings>

// Qt
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>

#include <QGradient>
#include <QLinearGradient>

#include "plasma/plasma.h"
#include "plasma/phase.h"

using namespace Kickoff;

TabBar::TabBar(QWidget *parent)
        : QTabBar(parent),
        m_hoveredTabIndex(-1),
        m_switchOnHover(true),
        m_animateSwitch(true)
{
    m_lastIndex[0] = -1;
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(startAnimation()));

    m_tabSwitchTimer.setSingleShot(true);
    connect(&m_tabSwitchTimer, SIGNAL(timeout()), this, SLOT(switchToHoveredTab()));
    setMouseTracking(true);
}

void TabBar::setCurrentIndexWithoutAnimation(int index)
{
    disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(startAnimation()));
    setCurrentIndex(index);
    storeLastIndex();
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(startAnimation()));
    animationFinished();
}

void TabBar::setSwitchTabsOnHover(bool switchOnHover)
{
    m_switchOnHover = switchOnHover;
}

bool TabBar::switchTabsOnHover() const
{
    return m_switchOnHover;
}

void TabBar::setAnimateSwitch(bool animateSwitch)
{
    m_animateSwitch = animateSwitch;
}

bool TabBar::animateSwitch()
{
    return m_animateSwitch;
}

QSize TabBar::tabSize(int index) const
{
    QSize hint;
    const QFontMetrics metrics(KGlobalSettings::smallestReadableFont());
    const QSize textSize = metrics.size(Qt::TextHideMnemonic, tabText(index));
    hint.rwidth() = qMax(iconSize().width(), textSize.width()+4);
    hint.rheight() = iconSize().height() + textSize.height();
    hint.rwidth() += 2 * TAB_CONTENTS_MARGIN;
    hint.rheight() += 2 * TAB_CONTENTS_MARGIN;
    return hint;
}

void TabBar::storeLastIndex()
{
    // if first run
    if (m_lastIndex[0] == -1) {
        m_lastIndex[1] = currentIndex();
    }
    m_lastIndex[0] = m_lastIndex[1];
    m_lastIndex[1] = currentIndex();
}

int TabBar::lastIndex() const
{
    return m_lastIndex[0];
}

QSize TabBar::tabSizeHint(int index) const
{
    QSize hint = tabSize(index);

    if (count() > 0) {
        int minwidth = 0;
        for (int i = count() - 1; i >= 0; i--) {
            minwidth += tabSize(i).rwidth();
        }
        if (minwidth < width()) {
            hint.rwidth() += (width() - minwidth) / count();
        }
    }
    return hint;
}

QPainterPath TabBar::tabPath(const QRect &_r)
{
    const int radius = 6;
    Shape s = shape();
    QPainterPath path;
    QRect r = _r;

    switch (s) {
        case RoundedSouth:
        case TriangularSouth:
            r.adjust(3, 0, -3, -3);
            path.moveTo(rect().topLeft());
            path.lineTo(r.topLeft());
            // Top left corner
            path.quadTo(r.topLeft() + QPoint(radius, 0), r.topLeft() + QPoint(radius, radius));
            path.lineTo(r.bottomLeft() + QPoint(radius, -radius));
            // Bottom left corner
            path.quadTo(r.bottomLeft() + QPoint(radius, 0), r.bottomLeft() + QPoint(radius * 2, 0));
            path.lineTo(r.bottomRight() + QPoint(-radius * 2, 0));
            // Bottom right corner
            path.quadTo(r.bottomRight() + QPoint(-radius, 0), r.bottomRight() + QPoint(-radius, -radius));
            path.lineTo(r.topRight() + QPoint(-radius, radius));
            // Top right corner
            path.quadTo(r.topRight() + QPoint(-radius, 0), r.topRight());
            path.lineTo(rect().topRight());
            break;
        case RoundedNorth:
        case TriangularNorth:
            r.adjust(3, 3, -3, 0);
            path.moveTo(rect().bottomLeft());
            // Bottom left corner
            path.lineTo(r.bottomLeft());
            path.quadTo(r.bottomLeft() + QPoint(radius, 0), r.bottomLeft() + QPoint(radius, -radius));
            // Top left corner
            path.lineTo(r.topLeft() + QPoint(radius, radius));
            path.quadTo(r.topLeft() + QPoint(radius, 0), r.topLeft() + QPoint(radius * 2, 0));
            // Top right corner
            path.lineTo(r.topRight() + QPoint(-radius * 2, 0));
            path.quadTo(r.topRight() + QPoint(-radius, 0), r.topRight() + QPoint(-radius, radius));
            // Bottom right corner
            path.lineTo(r.bottomRight() + QPoint(-radius, -radius));
            path.quadTo(r.bottomRight() + QPoint(-radius, 0), r.bottomRight());
            path.lineTo(rect().bottomRight());
            break;
        case RoundedWest:
        case TriangularWest:
            r.adjust(3, 0, 0, 0);
            path.moveTo(rect().topRight());
            // Top right corner
            path.lineTo(r.topRight());
            path.quadTo(r.topRight() + QPoint(0, radius), r.topRight() + QPoint(-radius, radius));
            // Top left corner
            path.lineTo(r.topLeft() + QPoint(radius, radius));
            path.quadTo(r.topLeft() + QPoint(0, radius), r.topLeft() + QPoint(0, radius * 2));
            // Bottom left corner
            path.lineTo(r.bottomLeft() + QPoint(0, -radius * 2));
            path.quadTo(r.bottomLeft() + QPoint(0, -radius), r.bottomLeft() + QPoint(radius, -radius));
            // Bottom right corner
            path.lineTo(r.bottomRight() + QPoint(-radius, -radius));
            path.quadTo(r.bottomRight() + QPoint(0, -radius), r.bottomRight());
            path.lineTo(rect().bottomRight());
        case RoundedEast:
        case TriangularEast:
            r.adjust(0, 0, -3, 0);
            path.moveTo(rect().topLeft());
            // Top left corner
            path.lineTo(r.topLeft());
            path.quadTo(r.topLeft() + QPoint(0, radius), r.topLeft() + QPoint(radius, radius));
            // Top right corner
            path.lineTo(r.topRight() + QPoint(-radius, radius));
            path.quadTo(r.topRight() + QPoint(0, radius), r.topRight() + QPoint(0, radius * 2));
            // Bottom right corner
            path.lineTo(r.bottomRight() + QPoint(0, -radius * 2));
            path.quadTo(r.bottomRight() + QPoint(0, -radius), r.bottomRight() + QPoint(-radius, -radius));
            // Bottom left corner
            path.lineTo(r.bottomLeft() + QPoint(radius, -radius));
            path.quadTo(r.bottomLeft() + QPoint(radius, 0), r.bottomLeft());
            path.lineTo(rect().bottomLeft());
            break;
    }

    return path;
}

void TabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    //int numTabs = count();
    int currentTab = currentIndex();
    //bool ltr = painter.layoutDirection() == Qt::LeftToRight; // Not yet used
    painter.setFont(KGlobalSettings::smallestReadableFont());

    // Drawing Tabborders
    QRect movingRect;

    if (m_currentAnimRect.isNull()) {
        movingRect = tabRect(currentIndex());
    } else {
        movingRect = m_currentAnimRect;
    }
    QPainterPath path = tabPath(movingRect);

    painter.save();
    painter.translate(0.5, 0.5);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(path, palette().base());
    painter.setPen(QPen(palette().mid(), 1));
    painter.drawPath(path);
    painter.restore();

    QFontMetrics metrics(painter.font());
    int textHeight = metrics.height();
    int delta;

    for (int i = 0; i < count(); i++) {
        if (i == currentTab) {
            delta = m_animProgress * TAB_CONTENTS_MARGIN;
        } else if (i == lastIndex()) {
            delta = TAB_CONTENTS_MARGIN - m_animProgress * TAB_CONTENTS_MARGIN;
        } else {
            delta = 0;
        }

        QRect rect = tabRect(i).adjusted(TAB_CONTENTS_MARGIN, TAB_CONTENTS_MARGIN,
                                         -TAB_CONTENTS_MARGIN, -TAB_CONTENTS_MARGIN);
        // draw tab icon
        QRect iconRect = rect;
        iconRect.setBottom(iconRect.bottom() - textHeight);
        iconRect.adjust(0, -delta, 0, -delta);
        tabIcon(i).paint(&painter, iconRect);

        // draw tab text
        QRect textRect = rect;
        textRect.setTop(textRect.bottom() - textHeight);
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextHideMnemonic, tabText(i));
    }
}

void TabBar::leaveEvent(QEvent *event)
{
    Q_UNUSED(event)
    m_hoveredTabIndex = -1;
}

void TabBar::mouseMoveEvent(QMouseEvent *event)
{
    m_hoveredTabIndex = tabAt(event->pos());
    if (m_switchOnHover && m_hoveredTabIndex > -1 && m_hoveredTabIndex != currentIndex()) {
        m_tabSwitchTimer.stop();
        m_tabSwitchTimer.start(50);
    }
}

void TabBar::resizeEvent(QResizeEvent* event)
{
    QTabBar::resizeEvent(event);
    m_currentAnimRect = tabRect(currentIndex());
    update();
}

void TabBar::switchToHoveredTab()
{
    if (m_hoveredTabIndex < 0 || m_hoveredTabIndex == currentIndex()) {
        return;
    }

    if (m_animateSwitch) {
        setCurrentIndex(m_hoveredTabIndex);
    }
    else {
        setCurrentIndexWithoutAnimation(m_hoveredTabIndex);
    }
}

void TabBar::startAnimation()
{
    storeLastIndex();
    Plasma::Phase::self()->customAnimation(10, 150, Plasma::Phase::EaseInOutCurve, this, "onValueChanged");

}

void TabBar::onValueChanged(qreal value)
{
    if ((m_animProgress = value) == 1.0) {
        animationFinished();
        return;
    }

    bool horiz = true;

    switch (shape()) {
        case RoundedWest:
        case RoundedEast:
        case TriangularWest:
        case TriangularEast:
            horiz = false;
            break;
        default:
            // we're horizontal
            break;
    }
    // animation rect
    QRect rect = tabRect(currentIndex());
    QRect lastRect = tabRect(lastIndex());
    int x = horiz ? lastRect.x() - value * (lastRect.x() - rect.x()) : rect.x();
    int y = horiz ? rect.y() : lastRect.y() - value * (lastRect.y() - rect.y());
    QSizeF sz = lastRect.size() - value * (lastRect.size() - rect.size());
    m_currentAnimRect = QRect(x, y, sz.width(), sz.height());
    update();
}

void TabBar::animationFinished()
{
    m_currentAnimRect = QRect();
    update();
}

#include "tabbar.moc"
