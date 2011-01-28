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
#include <KColorScheme>

// Qt
#include <QMouseEvent>
#include <QPainter>
#include <QIcon>
#include <QEasingCurve>
#include <QPropertyAnimation>

#include <Plasma/Plasma>
#include <Plasma/Theme>
#include <Plasma/FrameSvg>


using namespace Kickoff;

TabBar::TabBar(QWidget *parent)
        : KTabBar(parent),
        m_hoveredTabIndex(-1),
        m_switchOnHover(true),
        m_animateSwitch(true),
        m_animProgress(1.0)
{
    m_lastIndex[0] = -1;
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(startAnimation()));

    m_tabSwitchTimer.setSingleShot(true);
    connect(&m_tabSwitchTimer, SIGNAL(timeout()), this, SLOT(switchToHoveredTab()));
    setAcceptDrops(true);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setUsesScrollButtons(false);

    background = new Plasma::FrameSvg(this);
    background->setImagePath("dialogs/kickoff");
    background->setEnabledBorders(
        Plasma::FrameSvg::BottomBorder |
        Plasma::FrameSvg::LeftBorder |
        Plasma::FrameSvg::RightBorder
    );
    background->resizeFrame(size());
    background->setElementPrefix("plain");

    connect(background, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

void TabBar::setShape(Shape shape)
{
    resize(0, 0);    // This is required, so that the custom implementation of tabSizeHint,
    // which expands the tabs to the full width of the widget does not pick up
    // the previous width, e.g. if the panel is moved from the bottom to the left
    KTabBar::setShape(shape);
    resize(sizeHint());
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

bool TabBar::animateSwitch() const 
{
    return m_animateSwitch;
}

QSize TabBar::tabSize(int index) const
{
    QSize hint;
    const QFontMetrics metrics(KGlobalSettings::smallestReadableFont());
    const QSize textSize = metrics.size(Qt::TextHideMnemonic, tabText(index));
    hint.rwidth() = qMax(iconSize().width(), textSize.width());
    hint.rheight() = iconSize().height() + textSize.height();
    hint.rwidth() += 4 * TAB_CONTENTS_MARGIN;
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
    int minwidth = 0;
    int minheight = 0;

    Shape s = shape();
    switch (s) {
    case RoundedSouth:
    case TriangularSouth:
    case RoundedNorth:
    case TriangularNorth:
        if (count() > 0) {
            for (int i = count() - 1; i >= 0; i--) {
                minwidth += tabSize(i).width();
            }
            if (minwidth < width()) {
                hint.rwidth() += (width() - minwidth) / count();
            }
        }
        break;
    case RoundedWest:
    case TriangularWest:
    case RoundedEast:
    case TriangularEast:
        if (count() > 0) {
            for (int i = count() - 1; i >= 0; i--) {
                minheight += tabSize(i).height();
            }
            if (minheight < height()) {
                hint.rheight() += (height() - minheight) / count();
            }
        }
        hint.rwidth() = qMax(hint.width(), width());
        break;
    }
    return hint;
}

QSize TabBar::sizeHint() const
{
    int width = 0;
    int height = 0;

    if (isVertical()) {
        for (int i = count() - 1; i >= 0; i--) {
            height += tabSize(i).height();
        }

        width = tabSize(0).width();
    } else {
        for (int i = count() - 1; i >= 0; i--) {
            width += tabSize(i).width();
        }

        height = tabSize(0).height();
    }
    return QSize(width, height);
}

QPainterPath TabBar::tabPath(const QRectF &_r)
{
    const qreal radius = 6;
    Shape s = shape();
    QPainterPath path;
    QRectF r = _r;

    switch (s) {
    case RoundedSouth:
    case TriangularSouth:
        r.adjust(0, 0, 0, -3);
        path.moveTo(r.topLeft());
        // Top left corner
        path.quadTo(r.topLeft() + QPointF(radius, 0), r.topLeft() + QPointF(radius, radius));
        path.lineTo(r.bottomLeft() + QPointF(radius, -radius));
        // Bottom left corner
        path.quadTo(r.bottomLeft() + QPointF(radius, 0), r.bottomLeft() + QPointF(radius * 2, 0));
        path.lineTo(r.bottomRight() + QPoint(-radius * 2, 0));
        // Bottom right corner
        path.quadTo(r.bottomRight() + QPointF(-radius, 0), r.bottomRight() + QPointF(-radius, -radius));
        path.lineTo(r.topRight() + QPointF(-radius, radius));
        // Top right corner
        path.quadTo(r.topRight() + QPointF(-radius, 0), r.topRight());
        break;
    case RoundedNorth:
    case TriangularNorth:

        r.adjust(0, 3, 0, 1);
        path.moveTo(r.bottomLeft());
        // Bottom left corner
        path.quadTo(r.bottomLeft() + QPointF(radius, 0), r.bottomLeft() + QPointF(radius, -radius));
        // Top left corner
        path.lineTo(r.topLeft() + QPointF(radius, radius));
        path.quadTo(r.topLeft() + QPoint(radius, 0), r.topLeft() + QPointF(radius * 2, 0));
        // Top right corner
        path.lineTo(r.topRight() + QPointF(-radius * 2, 0));
        path.quadTo(r.topRight() + QPointF(-radius, 0), r.topRight() + QPointF(-radius, radius));
        // Bottom right corner
        path.lineTo(r.bottomRight() + QPointF(-radius, -radius));
        path.quadTo(r.bottomRight() + QPointF(-radius, 0), r.bottomRight());
        break;
    case RoundedWest:
    case TriangularWest:
        r.adjust(3, 0, 1, 0);
        path.moveTo(r.topRight());
        // Top right corner
        path.lineTo(r.topRight());
        path.quadTo(r.topRight() + QPointF(0, radius), r.topRight() + QPointF(-radius, radius));
        // Top left corner
        path.lineTo(r.topLeft() + QPointF(radius, radius));
        path.quadTo(r.topLeft() + QPointF(0, radius), r.topLeft() + QPointF(0, radius * 2));
        // Bottom left corner
        path.lineTo(r.bottomLeft() + QPointF(0, -radius * 2));
        path.quadTo(r.bottomLeft() + QPointF(0, -radius), r.bottomLeft() + QPointF(radius, -radius));
        // Bottom right corner
        path.lineTo(r.bottomRight() + QPointF(-radius, -radius));
        path.quadTo(r.bottomRight() + QPointF(0, -radius), r.bottomRight());
        break;
    case RoundedEast:
    case TriangularEast:
        r.adjust(0, 0, -3, 0);
        path.moveTo(r.topLeft());
        // Top left corner
        path.quadTo(r.topLeft() + QPointF(0, radius), r.topLeft() + QPointF(radius, radius));
        // Top right corner
        path.lineTo(r.topRight() + QPointF(-radius, radius));
        path.quadTo(r.topRight() + QPointF(0, radius), r.topRight() + QPointF(0, radius * 2));
        // Bottom right corner
        path.lineTo(r.bottomRight() + QPointF(0, -radius * 2));
        path.quadTo(r.bottomRight() + QPointF(0, -radius), r.bottomRight() + QPointF(-radius, -radius));
        // Bottom left corner
        path.lineTo(r.bottomLeft() + QPointF(radius, -radius));
        path.quadTo(r.bottomLeft() + QPointF(0, -radius), r.bottomLeft());
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

    background->paintFrame(&painter);

    //bool ltr = painter.layoutDirection() == Qt::LeftToRight; // Not yet used
    painter.setFont(KGlobalSettings::smallestReadableFont());

    // Drawing Tabborders
    QRectF movingRect;

    if (m_currentAnimRect.isNull()) {
        movingRect = tabRect(currentIndex());
    } else {
        movingRect = m_currentAnimRect;
    }
    QPainterPath path = tabPath(movingRect);

    painter.save();
    painter.setPen(QPen(palette().base(), 1));

    painter.setRenderHint(QPainter::Antialiasing);
    
    painter.fillPath(path, palette().base());

    painter.restore();

    QFontMetrics metrics(painter.font());
    int textHeight = metrics.height();

    for (int i = 0; i < count(); i++) {
        QRect rect = tabRect(i).adjusted(TAB_CONTENTS_MARGIN, TAB_CONTENTS_MARGIN,
                                         -TAB_CONTENTS_MARGIN, -TAB_CONTENTS_MARGIN);
        // draw tab icon
        QRectF iconRect = rect;
        iconRect.setBottom(iconRect.bottom() - textHeight);
        iconRect.adjust(0, (isVertical() ? 1 : 0) * TAB_CONTENTS_MARGIN + 3, 0, 0);

        tabIcon(i).paint(&painter, iconRect.toRect());

        // draw tab text
        if (i != currentTab || m_animProgress < 0.9) {
            //painter.setPen(QPen(KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText), 0));
            painter.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        } else {
            painter.setPen(QPen(KColorScheme(QPalette::Active).foreground(KColorScheme::NormalText), 0));
        }
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
    KTabBar::resizeEvent(event);
    m_currentAnimRect = tabRect(currentIndex());

    background->resizeFrame(event->size());

    update();
}

void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    m_hoveredTabIndex = tabAt(event->pos());
    m_tabSwitchTimer.stop();
    m_tabSwitchTimer.start(50);
    event->ignore();
}

void TabBar::switchToHoveredTab()
{
    if (m_hoveredTabIndex < 0 || m_hoveredTabIndex == currentIndex()) {
        return;
    }

    if (m_animateSwitch) {
        setCurrentIndex(m_hoveredTabIndex);
    } else {
        setCurrentIndexWithoutAnimation(m_hoveredTabIndex);
    }
}

void TabBar::startAnimation()
{
    storeLastIndex();

    QPropertyAnimation *animation = m_animation.data();
    if (animation) {
        animation->pause();
    } else {
        animation = new QPropertyAnimation(this, "animValue");
        animation->setEasingCurve(QEasingCurve::OutQuad);
        animation->setDuration(150);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
    }

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

qreal TabBar::animValue() const
{
    return m_animProgress;
}

void TabBar::setAnimValue(qreal value)
{
    if ((m_animProgress = value) == 1.0) {
        animationFinished();
        return;
    }
    // animation rect
    QRect rect = tabRect(currentIndex());
    QRect lastRect = tabRect(lastIndex());
    int x = isHorizontal() ? (int)(lastRect.x() - value * (lastRect.x() - rect.x())) : rect.x();
    int y = isHorizontal() ? rect.y() : (int)(lastRect.y() - value * (lastRect.y() - rect.y()));
    QSizeF sz = lastRect.size() - value * (lastRect.size() - rect.size());
    m_currentAnimRect = QRect(x, y, (int)(sz.width()), (int)(sz.height()));
    update();
}

void TabBar::animationFinished()
{
    m_currentAnimRect = QRect();
    update();
}

bool TabBar::isVertical() const
{
    Shape s = shape();
    if (s == RoundedWest ||
            s == RoundedEast ||
            s == TriangularWest ||
            s == TriangularEast) {
        return true;
    }
    return false;
}

bool TabBar::isHorizontal() const
{
    return !isVertical();
}

#include "tabbar.moc"
