/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
 *   Copyright (C) 2010 by Ivan Cukic <ivan.cukic@kde.org>
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

#include "abstracticon.h"

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QtGui/QTextLayout>
#include <QtGui/QTextLine>

#include <KIconLoader>
#include <KIcon>
#include <KGlobalSettings>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>
#include <Plasma/PaintUtils>
#include <QWidget>

const int LINE_COUNT=2; //maximum lines for icon name

namespace Plasma
{

AbstractIcon::AbstractIcon(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_background(new Plasma::FrameSvg(this)),
      m_backgroundFadeAnim(0),
      m_backgroundPrefix("normal"),
      m_iconHeight(DEFAULT_ICON_SIZE),
      m_maxSize(maximumSize()),
      m_backgroundAlpha(1),
      m_selected(false),
      m_hovered(false)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setCacheMode(DeviceCoordinateCache);
    setAcceptHoverEvents(true);
    m_background->setCacheAllRenderedFrames(true);
    m_background->setImagePath("widgets/tasks");
    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(syncTheme()));
    syncTheme();
}

AbstractIcon::~AbstractIcon()
{
}

void AbstractIcon::syncTheme()
{
    m_background->setElementPrefix("normal");
    m_background->resizeFrame(size());
    qreal ml, mt, mr, mb;
    m_background->getMargins(ml, mt, mr, mb);

    setContentsMargins(ml, mt, mr, mb);
    updateGeometry();
    update();
}

void AbstractIcon::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    m_background->resizeFrame(event->newSize());
}

QSizeF AbstractIcon::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    if (which == Qt::MinimumSize || which == Qt::PreferredSize) {
        qreal l, t, r, b;
        getContentsMargins(&l, &t, &r, &b);
        QFontMetrics fm(font());
        const int minHeight = m_iconHeight + 2 + fm.height() * LINE_COUNT;
        QSizeF s(m_iconHeight * 2 + l + r, minHeight + t + b);
        return s;
    }

    return QGraphicsWidget::sizeHint(which, constraint);
}

void AbstractIcon::setIconSize(int height)
{
    m_iconHeight = height;
    updateGeometry();
    update();
}

int AbstractIcon::iconSize() const
{
    return m_iconHeight;
}

void AbstractIcon::setName(const QString &name)
{
    m_name = name;
    updateGeometry();
    update();
}

QString AbstractIcon::name() const
{
    return m_name;
}

void AbstractIcon::collapse()
{
    if (isVisible()) {
        setVisible(false);
        m_maxSize = maximumSize();
        //kDebug() << m_maxSize;
        setMaximumSize(0, 0);
    }
}

void AbstractIcon::expand()
{
    if (!isVisible()) {
        setVisible(true);
        setMaximumSize(m_maxSize);
    }
}

void AbstractIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = true;

    m_backgroundPrefix = "hover";
    m_oldBackgroundPrefix = m_selected ? "focus" : "normal";
    m_background->setElementPrefix(m_backgroundPrefix);
    m_background->resizeFrame(size());

    emit hoverEnter(this);

    fadeBackground(150);
}

void AbstractIcon::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = false;
    emit hoverLeave(this);

    m_backgroundPrefix = m_selected ? "focus" : "normal";
    m_oldBackgroundPrefix = "hover";
    m_background->setElementPrefix(m_backgroundPrefix);
    m_background->resizeFrame(size());

    fadeBackground(250);
}

void AbstractIcon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
        event->accept();
        setCursor(Qt::OpenHandCursor);
        QMimeData *data = mimeData();
        if (data && !data->formats().isEmpty()) {
            qDebug() << "Start Dragging";
            emit dragging(this);
            QDrag *drag = new QDrag(event->widget());
            QPixmap p = pixmap(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge));
            drag->setPixmap(p);

            drag->setMimeData(mimeData());
            drag->exec();
        } else {
            delete data;
        }
    }
}

void AbstractIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseReleaseEvent(event);
    if (isDraggable()) {
        setCursor(Qt::OpenHandCursor);
    }

    if (boundingRect().contains(event->pos())) {
        emit(clicked(this));
    }
}

void AbstractIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    if (isDraggable()) {
        setCursor(Qt::ClosedHandCursor);
    }
}

void AbstractIcon::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit doubleClicked(this);
}

void AbstractIcon::setSelected(bool selected)
{
    if (m_selected != selected) {
        m_selected = selected;

        if (m_selected) {
            m_backgroundPrefix = "focus";
            m_oldBackgroundPrefix = m_hovered ? "hover" : "normal";
        } else {
            m_backgroundPrefix = m_hovered ? "hover" : "normal";
            m_oldBackgroundPrefix = "focus";
        }

        fadeBackground(150);
    }
}

bool AbstractIcon::isSelected() const
{
    return m_selected;
}

bool AbstractIcon::isDraggable() const
{
    return cursor().shape() == Qt::OpenHandCursor ||
           cursor().shape() == Qt::ClosedHandCursor;
}

void AbstractIcon::setDraggable(bool draggable)
{
    if (draggable) {
        setCursor(Qt::OpenHandCursor);
    } else {
        unsetCursor();
    }
}

void AbstractIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    paintBackground(painter, option, widget);
    paintForeground(painter, option, widget);
}

void AbstractIcon::paintBackground(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (!option->rect.isValid()) {
        return;
    }

    if (!m_backgroundFadeAnim || m_backgroundFadeAnim->state() != QAbstractAnimation::Running) {
        m_background->setElementPrefix(m_backgroundPrefix);
        m_background->paintFrame(painter);
        return;
    }

    m_background->setElementPrefix(m_oldBackgroundPrefix);
    QPixmap bg = m_background->framePixmap();

    m_background->setElementPrefix(m_backgroundPrefix);
    bg = Plasma::PaintUtils::transition(bg, m_background->framePixmap(), m_backgroundAlpha);
    painter->drawPixmap(option->exposedRect, bg, option->exposedRect);
}

void AbstractIcon::paintForeground(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const QRectF rect = contentsRect();
    const int width = rect.width();
    const int height = rect.height();

    QRectF textRect(rect.x(), rect.y(), width, height - m_iconHeight - 2);
    QRect iconRect(rect.x() + qMax(0, (width / 2) - (m_iconHeight / 2)), textRect.bottom() + 2, m_iconHeight, m_iconHeight);

    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->setFont(font());

    int flags = Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap;
    QFontMetrics fm(font());
    qreal l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);

    int availableWidth = width - l - r;
    int totalWidth = availableWidth;
    int lineCreated = 0;
    QTextLayout textLayout(m_name, painter->font());

    textLayout.beginLayout();

    while (lineCreated < LINE_COUNT - 1) {
        QTextLine line = textLayout.createLine();

        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(availableWidth);
        totalWidth += line.naturalTextWidth();

        lineCreated++;
    }
    textLayout.endLayout();

    QString newText = fm.elidedText(m_name, Qt::ElideRight, totalWidth);
    //kDebug() << "Name=" << m_name << ", line=" << lineCreated << ", totalWidth=" <<  totalWidth << ",width=" << availableWidth;
    if (!newText.isEmpty() && qGray(painter->pen().color().rgb()) < 192) {
        const QRectF haloRect = fm.boundingRect(textRect.toAlignedRect(), flags, newText);
        PaintUtils::drawHalo(painter, haloRect);
    }

    painter->drawPixmap(iconRect, pixmap(QSize(m_iconHeight, m_iconHeight)));

    painter->drawText(textRect, flags, newText);
}

qreal AbstractIcon::backgroundFadeAlpha() const
{
    return m_backgroundAlpha;
}

void AbstractIcon::setBackgroundFadeAlpha(qreal progress)
{
    m_backgroundAlpha = progress;
    update();
}

void AbstractIcon::fadeBackground(int duration)
{
    if (m_oldBackgroundPrefix.isEmpty()) {
        update();
    } else {
        if (!m_backgroundFadeAnim) {
            m_backgroundFadeAnim = new QPropertyAnimation(this);
            m_backgroundFadeAnim->setEasingCurve(QEasingCurve::InQuad);
            m_backgroundFadeAnim->setPropertyName("backgroundFadeAlpha");
            m_backgroundFadeAnim->setTargetObject(this);
            m_backgroundFadeAnim->setStartValue(0);
            m_backgroundFadeAnim->setEndValue(1);
        }

        m_backgroundFadeAnim->setDuration(duration);
        m_backgroundFadeAnim->start();
    }
}

} // namespace Plasma


