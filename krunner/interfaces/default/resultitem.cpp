/***************************************************************************
 *   Copyright 2007 by Enrico Ros <enrico.ros@gmail.com>                   *
 *   Copyright 2007 by Riccardo Iaconelli <ruphy@kde.org>                  *
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
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


#include "resultitem.h"

#include <math.h>

#include <QtCore/QTimeLine>
#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QGraphicsItemAnimation>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>

#include <KDebug>
#include <KIcon>

#include <Plasma/PaintUtils>
#include <Plasma/Plasma>
#include <Plasma/RunnerManager>
#include <Plasma/PaintUtils>
#include <Plasma/FrameSvg>

#define TEXT_AREA_HEIGHT ResultItem::MARGIN + ResultItem::TEXT_MARGIN*2 + ResultItem::s_fontHeight
//#define NO_GROW_ANIM

void shadowBlur(QImage &image, int radius, const QColor &color);

int ResultItem::s_fontHeight = 0;

ResultItem::ResultItem(const Plasma::QueryMatch &match, QGraphicsWidget *parent, Plasma::FrameSvg *frame)
    : QGraphicsWidget(parent),
      m_match(0),
      m_frame(frame),
      m_tempTransp(1.0),
      m_highlight(false),
      m_index(-1),
      m_highlightTimerId(0)
{
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    setFocusPolicy(Qt::TabFocus);
    setCacheMode(DeviceCoordinateCache);
    resize(ITEM_SIZE, ITEM_SIZE + TEXT_AREA_HEIGHT);
    setZValue(0);

    if (s_fontHeight < 1) {
        //FIXME: reset when the application font changes
        QFontMetrics fm(font());
        s_fontHeight = fm.height();
        //kDebug() << "font height is: " << s_fontHeight;
    }

    setMatch(match);
}

ResultItem::~ResultItem()
{
}

QPointF ResultItem::targetPos() const
{
    const int x = HOVER_TROFF;
    //TODO: we will want/need something that doesn't rely on a fixed height
    const int y = m_index * ResultItem::BOUNDING_HEIGHT + HOVER_TROFF;
    //kDebug() << x << y << "for" << index;
    return QPointF(x, y);
}

void ResultItem::appear()
{
    setPos(targetPos());
    show();
}

void ResultItem::move()
{
    //qDebug() << "moving to" << index;
    setPos(targetPos());
}

void ResultItem::setMatch(const Plasma::QueryMatch &match)
{
    m_match = match;
    m_icon = KIcon(match.icon());
    update();
}

QString ResultItem::id() const
{
    return m_match.id();
}

bool ResultItem::compare(const ResultItem *one, const ResultItem *other)
{
    return other->m_match < one->m_match;
}

bool ResultItem::operator<(const ResultItem &other) const
{
    return m_match < other.m_match;
}

QString ResultItem::name() const
{
    return m_match.text();
}

QString ResultItem::description() const
{
    return m_match.subtext();
}

QString ResultItem::data() const
{
    return m_match.data().toString();
}

QIcon ResultItem::icon() const
{
    return m_icon;
}

Plasma::QueryMatch::Type ResultItem::group() const
{
    return m_match.type();
}

bool ResultItem::isQueryPrototype() const
{
    //FIXME: pretty lame way to decide if this is a query prototype
    return m_match.runner() == 0;
}

qreal ResultItem::priority() const
{
    // TODO, need to fator in more things than just this
    return m_match.relevance();
}

void ResultItem::setIndex(int index)
{
    if (m_index == index) {
        return;
    }

    bool first = m_index == -1;
    m_index = index;

    if (index < 0) {
        index = -1;
        return;
    }

    //kDebug() << index << first << hasFocus();
    if (first) {
        appear();
    } else {
        move();
    }
}

int ResultItem::index() const
{
    return m_index;
}

void ResultItem::remove()
{
    deleteLater();
}

void ResultItem::run(Plasma::RunnerManager *manager)
{
    manager->run(m_match);
}

void ResultItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    bool oldClipping = painter->hasClipping();
    painter->setClipping(false);

    QRectF rect = boundingRect().adjusted(0, 0, 0, -(TEXT_AREA_HEIGHT));
    QRect iRect = rect.toRect().adjusted(PADDING, PADDING, -PADDING, -PADDING);
    QSize iconSize = iRect.size().boundedTo(QSize(32, 32));
    //kDebug() << PADDING << PADDING;

    painter->setRenderHint(QPainter::Antialiasing);

    // Draw background
    if (isSelected() || m_highlight > 0) {
        m_frame->resizeFrame(rect.size());
        m_frame->paintFrame(painter, rect.topLeft());
    }/* else {
        m_frame->setElementPrefix("normal");
    }*/

    if (!hasFocus() && m_tempTransp < 0.9) {
        painter->setOpacity(m_tempTransp);

        if (!m_highlightTimerId) {
            m_highlightTimerId = startTimer(40);
        }
    }

    bool drawMixed = false;

    if (hasFocus() || isSelected()) {
        // here's what the next line means:
        // we check to see if the scene has focus, but that's overridden by the mouse hovering an
        // item ... or unless we are over 2 ticks into the higlight anim. complex but it works
        if (((scene() && !scene()->views().isEmpty() && !scene()->views()[0]->hasFocus()) &&
            !(option->state & QStyle::State_MouseOver)) || m_highlight > 2) {
            painter->drawPixmap(PADDING, PADDING, m_icon.pixmap(iconSize, QIcon::Active));
        } else {
            drawMixed = true;

            ++m_highlight;
            if (m_highlight == 1) {
                setGeometry(sceneBoundingRect().adjusted(-1, -1, 1, 1));
            } else if (m_highlight == 3) {
                setGeometry(sceneBoundingRect().adjusted(-2, -2, 2, 2));
            }

            if (!m_highlightTimerId) {
                m_highlightTimerId = startTimer(40);
            }
        }
    } else if (m_highlight > 0) {
        drawMixed = true;

        --m_highlight;
        if (m_highlight == 0) {
            setGeometry(sceneBoundingRect().adjusted(1, 1, -1, -1));
        } else if (m_highlight == 2) {
            setGeometry(sceneBoundingRect().adjusted(2, 2, -2, -2));
        }

        if (!m_highlightTimerId) {
            m_highlightTimerId = startTimer(40);
        }
    } else {
        painter->drawPixmap(PADDING, PADDING, m_icon.pixmap(iconSize, QIcon::Disabled));
    }

    if (drawMixed) {
        qreal factor = .2;

        if (m_highlight == 1) {
            factor = .4;
        } else if (m_highlight == 2) {
            factor = .6;
        } else if (m_highlight > 2) {
            factor = .8;
        }

        qreal activeOpacity = painter->opacity() * factor;

        painter->setOpacity(painter->opacity() * (1 - factor));
        painter->drawPixmap(PADDING, PADDING, m_icon.pixmap(iconSize, QIcon::Disabled));
        painter->setOpacity(activeOpacity);
        painter->drawPixmap(PADDING, PADDING, m_icon.pixmap(iconSize, QIcon::Active));
    }

    QRect textRect(iRect.topLeft() + QPoint(iconSize.width() + PADDING + TEXT_MARGIN, PADDING),
                   iRect.bottomRight());

    // Draw the text on a pixmap
    const QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    const int width = option->fontMetrics.width(name());
    QPixmap pixmap(textRect.size());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(textColor);
    //TODO: add subtext, make bold, etc...
    p.drawText(pixmap.rect(), Qt::AlignLeft, name());

    // Fade the pixmap out at the end
    if (width > pixmap.width()) {
        if (m_fadeout.isNull() || m_fadeout.height() != pixmap.height()) {
            QLinearGradient g(0, 0, 20, 0);
            g.setColorAt(0, layoutDirection() == Qt::LeftToRight ? Qt::white : Qt::transparent);
            g.setColorAt(1, layoutDirection() == Qt::LeftToRight ? Qt::transparent : Qt::white);
            m_fadeout = QPixmap(20, textRect.height());
            m_fadeout.fill(Qt::transparent);
            QPainter p(&m_fadeout);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(m_fadeout.rect(), g);
        }
        const QRect r = QStyle::alignedRect(layoutDirection(), Qt::AlignRight, m_fadeout.size(), pixmap.rect());
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(r.topLeft(), m_fadeout);
    }
    p.end();

    // Draw a drop shadow if we have a bright text color
    if (qGray(textColor.rgb()) > 192) {
        const int blur = 2;
        const QPoint offset(1, 1);

        QImage shadow(pixmap.size() + QSize(blur * 2, blur * 2), QImage::Format_ARGB32_Premultiplied);
        p.begin(&shadow);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(shadow.rect(), Qt::transparent);
        p.drawPixmap(blur, blur, pixmap);
        p.end();

        Plasma::PaintUtils::shadowBlur(shadow, blur, Qt::black);

        // Draw the shadow
        painter->drawImage(textRect.topLeft() - QPoint(blur, blur) + offset, shadow);
    }

    // Draw the text
    painter->drawPixmap(textRect.topLeft(), pixmap);
    painter->setClipping(oldClipping);
}

void ResultItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *e)
{
    QGraphicsItem::hoverLeaveEvent(e);
    //update();
    //emit hoverLeave(this);
    setFocus(Qt::MouseFocusReason);
    setSelected(false);
}

void ResultItem::hoverEnterEvent(QGraphicsSceneHoverEvent *e)
{
    QGraphicsItem::hoverEnterEvent(e);
//    update();
//    emit hoverEnter(this);
    //setFocusItem(this);
    setFocus(Qt::MouseFocusReason);
    scene()->clearSelection();
    setSelected(true);
}

void ResultItem::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e)

    m_tempTransp += 0.1;
    killTimer(m_highlightTimerId);
    m_highlightTimerId = 0;

    update();
}

void ResultItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    emit activated(this);
}

void ResultItem::focusInEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusInEvent(event);
    setZValue(1);

    if (!m_highlightTimerId) {
        m_highlightTimerId = startTimer(40);
    }

    emit hoverEnter(this);
}

void ResultItem::focusOutEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusOutEvent(event);
    setZValue(0);

    if (!m_highlightTimerId) {
        m_highlightTimerId = startTimer(40);
    }

    emit hoverLeave(this);
}

void ResultItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        emit activated(this);
    } else {
        QGraphicsWidget::keyPressEvent(event);
    }
}

QVariant ResultItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneHasChanged && scene()) {
        resize(scene()->width() - HOVER_TROFF * 2, geometry().height());
    }

    return QGraphicsWidget::itemChange(change, value);
}

#include "resultitem.moc"

