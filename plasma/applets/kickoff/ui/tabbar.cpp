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

// Qt
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>

#include <QGradient>
#include <QLinearGradient>

#include "plasma/plasma.h"

using namespace Kickoff;

TabBar::TabBar(QWidget *parent)
    : QTabBar(parent),
      m_hoveredTabIndex(-1),
      m_switchOnHover(true)
{
    //FIXME: should be replaced with a Phase custom animation
    m_animator.setDuration(150);
    m_animator.setCurveShape(QTimeLine::EaseInOutCurve);
    m_animator.setFrameRange(0, 10);
    connect(&m_animator, SIGNAL(frameChanged(int)), this, SLOT(update()));
    connect(&m_animator, SIGNAL(finished()), this, SLOT(animationFinished()));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(startAnimation()));

    m_tabSwitchTimer.setSingleShot(true);
    connect(&m_tabSwitchTimer, SIGNAL(timeout()), this, SLOT(switchToHoveredTab()));
    setMouseTracking(true);
}

void TabBar::setCurrentIndexWithoutAnimation(int index)
{
    blockSignals(true);
    setCurrentIndex(index);
    blockSignals(false);
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

QSize TabBar::tabSizeHint(int index) const
{
    QSize hint;
    const QFontMetrics metrics(font());
    const QSize textSize = metrics.size(Qt::TextHideMnemonic,tabText(index));

    hint.rwidth() = qMax(iconSize().width(),textSize.width());
    hint.rheight() = iconSize().height() + textSize.height();

    hint.rwidth() += 2*TAB_CONTENTS_MARGIN;
    hint.rheight() += 2*TAB_CONTENTS_MARGIN;

    return hint;
}

void TabBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    int numTabs = count();
    int currentTab = currentIndex();

    if (m_animStates.size() != numTabs) {
        m_animStates.resize(numTabs);
    }

   for (int i=0 ; i<count() ; i++) {
        QRect rect = tabRect(i).adjusted(TAB_CONTENTS_MARGIN,TAB_CONTENTS_MARGIN,
                                        -TAB_CONTENTS_MARGIN,-TAB_CONTENTS_MARGIN);

        // draw background for selected tab
        if (i == currentTab) {
            m_animStates[i] = qMin(m_animator.endFrame(), ++m_animStates[i]);
        } else if (m_animStates[i] > 0) {
            m_animStates[i] = qMax(0, m_animStates[i] - 2);
        }

        if (m_animStates[i] > 0) {
            // Draws the selected item with a gradient
            qreal alpha = m_animStates[i] / qreal(m_animator.endFrame());
            QLinearGradient g(0, 0, 0, tabRect(i).height());
            QColor base(palette().highlight().color());
            base.setAlphaF(alpha);
            g.setColorAt(0, base);
            g.setColorAt(0.3, base.lighter(110));
            g.setColorAt(1, base.lighter(125));
            QBrush bgBrush(g);
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.fillPath(Plasma::roundedRectangle(tabRect(i).adjusted(2, 0, -2, 0), 6), bgBrush);
        }

        // draw tab icon and text
        QFontMetrics metrics(painter.font());
        int textHeight = metrics.height();
        QRect iconRect = rect;
        int delta = int(m_animStates[i] / qreal(m_animator.endFrame()) * 4);
        iconRect.setBottom(iconRect.bottom()-textHeight);
        iconRect.adjust(0, -delta, 0, -delta);
        tabIcon(i).paint(&painter,iconRect);

        QRect textRect = rect;
        textRect.setTop(textRect.bottom()-textHeight);
        painter.drawText(textRect,Qt::AlignCenter|Qt::TextHideMnemonic,tabText(i));
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

void TabBar::switchToHoveredTab()
{
    if (m_hoveredTabIndex < 0 || m_hoveredTabIndex == currentIndex()) {
        return;
    }

    setCurrentIndex(m_hoveredTabIndex);
}

void TabBar::startAnimation()
{
    m_animator.start();
}

void TabBar::animationFinished()
{
    int currentTab = currentIndex();
    update();

    int numTabs = count();

    if (m_animStates.size() != numTabs)
       return;

    for (int i = 0; i < numTabs; ++i) {
        if (i == currentTab) {
            m_animStates[i] = m_animator.endFrame();
        } else {
            m_animStates[i] = 0;
        }
    }
    update();
}

#include "tabbar.moc"
