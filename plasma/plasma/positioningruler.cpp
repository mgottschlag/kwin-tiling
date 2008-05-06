 /*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "positioningruler.h"

#include <QPainter>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QApplication>
#include <QDesktopWidget>

#include <KIcon>
#include <KColorUtils>
#include <KWindowSystem>

#include <plasma/theme.h>
#include <plasma/svg.h>
#include <plasma/containment.h>


class PositioningRuler::Private
{
public:
    Private()
       : location(Plasma::BottomEdge),
         alignment(Qt::AlignLeft),
         dragging(NoElement),
         startDragPos(0,0),
         offset(0),
         minLength(0),
         maxLength(0),
         availableLength(0),
         leftMaxSliderRect(QRect(0,0,0,0)),
         rightMaxSliderRect(QRect(0,0,0,0)),
         leftMinSliderRect(QRect(0,0,0,0)),
         rightMinSliderRect(QRect(0,0,0,0)),
         offsetSliderRect(QRect(0,0,0,0)),
         slider(0),
         elementPrefix(QString())
    {
    }

    ~Private()
    {
    }

    void moveSlider(QRect &sliderRect, QRect &symmetricSliderRect, const QPoint &newPos)
    {
        if (location == Plasma::LeftEdge || location == Plasma::RightEdge) {
            sliderRect.moveTop(newPos.y());
            if (alignment == Qt::AlignCenter) {
                symmetricSliderRect.moveTop((offset + (offset - newPos.y())));
            }
        } else {
            sliderRect.moveLeft(newPos.x());
            if (alignment == Qt::AlignCenter) {
                symmetricSliderRect.moveLeft((offset + (offset - newPos.x())));
            }
        }
    }

    int sliderRectToLength(const QRect &sliderRect)
    {
        int sliderPos;
        int offsetPos;

        if (location == Plasma::LeftEdge || location == Plasma::RightEdge) {
            sliderPos = sliderRect.center().y();
            offsetPos = offsetSliderRect.center().y();
        } else {
            sliderPos = sliderRect.center().x();
            offsetPos = offsetSliderRect.center().x();
        }

        if (alignment == Qt::AlignCenter) {
            return 2 * qAbs(sliderPos - offsetPos);
        } else {
            return qAbs(sliderPos - offsetPos);
        }
    }

    void loadSlidersGraphics()
    {
        QString elementPrefix;

        switch (location) {
        case Plasma::LeftEdge:
            elementPrefix = "west-";
            break;
        case Plasma::RightEdge:
            elementPrefix = "east-";
            break;
        case Plasma::TopEdge:
            elementPrefix = "north-";
            break;
        case Plasma::BottomEdge:
        default:
            elementPrefix = "south-";
            break;
        }

        leftMaxSliderRect.setSize(slider->elementSize(elementPrefix + "maxslider"));
        rightMaxSliderRect.setSize(leftMaxSliderRect.size());
        leftMinSliderRect.setSize(slider->elementSize(elementPrefix + "minslider"));
        rightMinSliderRect.setSize(leftMinSliderRect.size());
        offsetSliderRect.setSize(slider->elementSize(elementPrefix + "offsetslider"));
    }

    enum DragElement { NoElement = 0,
                       LeftMaxSlider,
                       RightMaxSlider,
                       LeftMinSlider,
                       RightMinSlider,
                       OffsetSlider
                     };

    Plasma::Location location;
    Qt::Alignment alignment;
    DragElement dragging;
    QPoint startDragPos;
    int offset;
    int minLength;
    int maxLength;
    int availableLength;
    QRect leftMaxSliderRect;
    QRect rightMaxSliderRect;
    QRect leftMinSliderRect;
    QRect rightMinSliderRect;
    QRect offsetSliderRect;
    Plasma::Svg *slider;
    QString elementPrefix;
};

PositioningRuler::PositioningRuler(QWidget* parent)
   : QWidget(parent),
     d(new Private())
{
   d->slider = new Plasma::Svg(this);
   d->slider->setImagePath("widgets/containment-controls");

   d->loadSlidersGraphics();
}

PositioningRuler::~PositioningRuler()
{
}

QSize PositioningRuler::sizeHint() const
{
    //FIXME:why must add 6 pixels????
    
    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        return QSize(d->leftMaxSliderRect.width() + d->leftMinSliderRect.width() + 6, d->availableLength);
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        return QSize(d->availableLength, d->leftMaxSliderRect.height() + d->leftMinSliderRect.height() + 6);
        break;
    }
    
}

void PositioningRuler::setLocation(const Plasma::Location &loc)
{
    if (d->location == loc) {
        return;
    }

    d->location = loc;

    d->loadSlidersGraphics();

    update();
}

Plasma::Location PositioningRuler::location() const
{
    return d->location;
}

void PositioningRuler::setAlignment(const Qt::Alignment &align)
{
    if (d->alignment == align) {
        return;
    }

    d->alignment = align;
    update();
}

Qt::Alignment PositioningRuler::alignment() const
{
    return d->alignment;
}

void PositioningRuler::setOffset(const int &newOffset)
{
    const int delta = newOffset - d->offset;

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        d->offsetSliderRect.moveCenter(QPoint(d->offsetSliderRect.center().x(), d->offsetSliderRect.center().y() + delta));
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        d->offsetSliderRect.moveCenter(QPoint(d->offsetSliderRect.center().x() + delta, d->offsetSliderRect.center().y()));
        break;
    }

    d->offset = newOffset;
    d->maxLength += delta;
    d->minLength += delta;
}

int PositioningRuler::offset() const
{
    return d->offset;
}

void PositioningRuler::setMaxLength(const int &newMax)
{
    int deltaX;
    int deltaY;

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        deltaX = 0;
        deltaY = newMax - d->maxLength;
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        deltaX = newMax - d->maxLength;
        deltaY = 0;
        break;
    }

    if (d->alignment == Qt::AlignLeft) {
        d->rightMaxSliderRect.moveCenter(QPoint(d->rightMaxSliderRect.center().x() + deltaX,
                                         d->rightMaxSliderRect.center().y() + deltaY));
    } else if (d->alignment == Qt::AlignRight) {
        d->leftMaxSliderRect.moveCenter(QPoint(d->leftMaxSliderRect.center().x() - deltaX,
                                        d->leftMaxSliderRect.center().y() - deltaY));
    //center
    } else {
        d->rightMaxSliderRect.moveCenter(QPoint(d->rightMaxSliderRect.center().x() + deltaX/2,
                                         d->rightMaxSliderRect.center().y() + deltaY/2));
        d->leftMaxSliderRect.moveCenter(QPoint(d->leftMaxSliderRect.center().x() - deltaX/2,
                                        d->leftMaxSliderRect.center().y() - deltaY/2));
    }

    d->maxLength = newMax;
    if (d->minLength > d->maxLength) {
        setMinLength(newMax);
    }
}

int PositioningRuler::maxLength() const
{
    return d->maxLength;
}

void PositioningRuler::setMinLength(const int &newMin)
{
    int deltaX;
    int deltaY;

    switch (d->location) {
    case Plasma::LeftEdge:
    case Plasma::RightEdge:
        deltaX = 0;
        deltaY = newMin - d->minLength;
        break;
    case Plasma::TopEdge:
    case Plasma::BottomEdge:
    default:
        deltaX = newMin - d->minLength;
        deltaY = 0;
        break;
    }


    if (d->alignment == Qt::AlignLeft) {
        d->rightMinSliderRect.moveCenter(QPoint(d->rightMinSliderRect.center().x() + deltaX,
                                         d->rightMinSliderRect.center().y() + deltaY));
    } else if (d->alignment == Qt::AlignRight) {
        d->leftMinSliderRect.moveCenter(QPoint(d->leftMinSliderRect.center().x() - deltaX,
                                        d->leftMinSliderRect.center().y() - deltaY));
    //center
    } else {
        d->rightMinSliderRect.moveCenter(QPoint(d->rightMinSliderRect.center().x() + deltaX/2,
                                         d->rightMinSliderRect.center().y() + deltaY/2));
        d->leftMinSliderRect.moveCenter(QPoint(d->leftMinSliderRect.center().x() - deltaX/2,
                                        d->leftMinSliderRect.center().y() - deltaY/2));
    }

    d->minLength = newMin;
    if (d->minLength > d->minLength) {
        setMaxLength(newMin);
    }
}

int PositioningRuler::minLength() const
{
    return d->minLength;
}

void PositioningRuler::setAvailableLength(const int &newAvail)
{
    d->availableLength = newAvail;
    
    if (d->maxLength > newAvail) {
        setMaxLength(newAvail);
    }

    if (d->minLength > newAvail) {
        setMinLength(newAvail);
    }
}

int PositioningRuler::availableLength() const
{
    return d->availableLength;
}

void PositioningRuler::paintEvent(QPaintEvent *event)
{
    //Draw background
    int x = 0;
    int y = 0;
    if (d->location == Plasma::LeftEdge || d->location == Plasma::RightEdge) {
        x = event->rect().width();
    } else {
        y = event->rect().height();
    }

    QPainter painter(this);
    QLinearGradient gradient(0, 0, x, y);
    QColor startColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    startColor.setAlphaF(0.25);
    QColor endColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    endColor.setAlphaF(0.25);

    gradient.setColorAt(0, startColor);
    gradient.setColorAt(1, endColor);

    painter.fillRect(event->rect(), gradient);


    QString elementPrefix;

    switch (d->location) {
    case Plasma::LeftEdge:
        elementPrefix = "west-";
        break;
    case Plasma::RightEdge:
        elementPrefix = "east-";
        break;
    case Plasma::TopEdge:
        elementPrefix = "north-";
        break;
    case Plasma::BottomEdge:
    default:
        elementPrefix = "south-";
        break;
    }

    //Draw handles
    if (d->alignment != Qt::AlignLeft) {
        d->slider->paint(&painter, d->leftMaxSliderRect, elementPrefix + "maxslider");
        d->slider->paint(&painter, d->leftMinSliderRect, elementPrefix + "minslider");
    }

    if (d->alignment != Qt::AlignRight) {
        d->slider->paint(&painter, d->rightMaxSliderRect, elementPrefix + "maxslider");
        d->slider->paint(&painter, d->rightMinSliderRect, elementPrefix + "minslider");
    }

    d->slider->paint(&painter, d->offsetSliderRect, elementPrefix + "offsetslider");
}

void PositioningRuler::mousePressEvent(QMouseEvent *event)
{
    if (d->alignment != Qt::AlignLeft && d->leftMaxSliderRect.contains(event->pos())) {
        d->dragging = Private::LeftMaxSlider;
        d->startDragPos = QPoint(event->pos().x() - d->leftMaxSliderRect.left(), event->pos().y() - d->leftMaxSliderRect.top());
    } else if (d->alignment != Qt::AlignRight && d->rightMaxSliderRect.contains(event->pos())) {
        d->dragging = Private::RightMaxSlider;
        d->startDragPos = QPoint(event->pos().x() - d->rightMaxSliderRect.left(), event->pos().y() - d->rightMaxSliderRect.top());
    } else if (d->alignment != Qt::AlignLeft && d->leftMinSliderRect.contains(event->pos())) {
        d->dragging = Private::LeftMinSlider;
        d->startDragPos = QPoint(event->pos().x() - d->leftMinSliderRect.left(), event->pos().y() - d->leftMinSliderRect.top());
    } else if (d->alignment != Qt::AlignRight && d->rightMinSliderRect.contains(event->pos())) {
        d->dragging = Private::RightMinSlider;
        d->startDragPos = QPoint(event->pos().x() - d->rightMinSliderRect.left(), event->pos().y() - d->rightMinSliderRect.top());
    } else if (d->offsetSliderRect.contains(event->pos())) {
        d->dragging = Private::OffsetSlider;
        d->startDragPos = QPoint(event->pos().x() - d->offsetSliderRect.left(), event->pos().y() - d->offsetSliderRect.top());
    } else {
        d->dragging = Private::NoElement;
    }

    QWidget::mousePressEvent(event);
}

void PositioningRuler::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)

    d->startDragPos = QPoint(0, 0);
    d->dragging = Private::NoElement;
}

void PositioningRuler::mouseMoveEvent(QMouseEvent *event)
{
    if (d->dragging == Private::NoElement) {
        return;
    }

    QPoint newPos = QPoint(qMin(event->pos().x() - d->startDragPos.x(), width() - d->leftMaxSliderRect.width()/2 + 1),
                           qMin(event->pos().y() - d->startDragPos.y(), height() - d->leftMaxSliderRect.height()/2 + 1));
    newPos = QPoint(qMax(newPos.x(), 0 - d->leftMaxSliderRect.width()/2), qMax(newPos.y(), 0 - d->leftMaxSliderRect.height()/2));

    //TODO:Vertical panel
    switch (d->dragging) {
    case Private::LeftMaxSlider:
        d->moveSlider(d->leftMaxSliderRect, d->rightMaxSliderRect, newPos);
        d->maxLength = d->sliderRectToLength(d->leftMaxSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->leftMinSliderRect, d->rightMinSliderRect, newPos);
            d->minLength = d->maxLength;
        }
        break;
    case Private::RightMaxSlider:
        d->moveSlider(d->rightMaxSliderRect, d->leftMaxSliderRect, newPos);
        d->maxLength = d->sliderRectToLength(d->rightMaxSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->rightMinSliderRect, d->leftMinSliderRect, newPos);
            d->minLength = d->maxLength;
        }
        break;
    case Private::LeftMinSlider:
        d->moveSlider(d->leftMinSliderRect, d->rightMinSliderRect, newPos);
        d->minLength = d->sliderRectToLength(d->leftMinSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->leftMaxSliderRect, d->rightMaxSliderRect, newPos);
            d->maxLength = d->minLength;
        }
        break;
    case Private::RightMinSlider:
        d->moveSlider(d->rightMinSliderRect, d->leftMinSliderRect, newPos);
        d->minLength = d->sliderRectToLength(d->rightMinSliderRect);

        if (d->minLength > d->maxLength) {
            d->moveSlider(d->rightMaxSliderRect, d->leftMaxSliderRect, newPos);
            d->maxLength = d->minLength;
        }
        break;
    case Private::OffsetSlider:
    {
        const int oldOffset = d->offset;

        if (d->location == Plasma::LeftEdge || d->location == Plasma::RightEdge) {
            d->offsetSliderRect.moveTop(newPos.y());
            d->offset = d->offsetSliderRect.center().y();
        } else {
            d->offsetSliderRect.moveLeft(newPos.x());
            d->offset = d->offsetSliderRect.center().x();
        }

        d->maxLength -= (d->offset - oldOffset);
        d->minLength -= (d->offset - oldOffset);

        break;
    }
    default:
        break;
    }

    emit rulersMoved(d->offset, d->minLength, d->maxLength);
    update();
}

void PositioningRuler::resizeEvent(QResizeEvent *event)
{
    //FIXME: the following is waay too clumsy...

    int rightMaxPos;
    int leftMaxPos;
    int rightMinPos;
    int leftMinPos;
    int offsetPos;

    switch (d->alignment) {
    case Qt::AlignLeft:
        rightMaxPos = d->offset + d->maxLength;
        leftMaxPos = 0;
        rightMinPos = d->offset + d->minLength;
        leftMinPos = 0;
        offsetPos = d->offset;
        break;
    case Qt::AlignRight:
        leftMaxPos = event->size().width() - d->offset - d->maxLength;
        rightMaxPos = 0;
        leftMinPos = event->size().width() - d->offset - d->maxLength;
        rightMinPos = 0;
        offsetPos = event->size().width() - d->offset;
        break;
    case Qt::AlignCenter:
    default:
        leftMaxPos = event->size().width()/2 + d->offset + d->maxLength/2;
        rightMaxPos = event->size().width()/2 + d->offset - d->maxLength/2;

        leftMinPos = event->size().width()/2 + d->offset + d->minLength/2;
        rightMinPos = event->size().width()/2 + d->offset - d->minLength/2;

        offsetPos = event->size().width()/2 - d->offset;
        break;
    }

    switch (d->location) {
    case Plasma::LeftEdge:
        d->leftMaxSliderRect.moveCenter(QPoint(3*(event->size().width()/4), leftMaxPos));
        d->rightMaxSliderRect.moveCenter(QPoint(3*(event->size().width()/4), rightMaxPos));
    
        d->leftMinSliderRect.moveCenter(QPoint(event->size().width()/4, leftMinPos));
        d->rightMinSliderRect.moveCenter(QPoint(event->size().width()/4, rightMinPos));
    
        d->offsetSliderRect.moveCenter(QPoint(3*(event->size().width()/4), offsetPos));
        break;
    case Plasma::RightEdge:
        d->leftMaxSliderRect.moveCenter(QPoint(event->size().width()/4, leftMaxPos));
        d->rightMaxSliderRect.moveCenter(QPoint(event->size().width()/4, rightMaxPos));
    
        d->leftMinSliderRect.moveCenter(QPoint(3*(event->size().width()/4), leftMinPos));
        d->rightMinSliderRect.moveCenter(QPoint(3*(event->size().width()/4), rightMinPos));
    
        d->offsetSliderRect.moveCenter(QPoint(event->size().width()/4, offsetPos));
        break;
    case Plasma::TopEdge:
        d->leftMaxSliderRect.moveCenter(QPoint(leftMaxPos, 3*(event->size().height()/4)));
        d->rightMaxSliderRect.moveCenter(QPoint(rightMaxPos, 3*(event->size().height()/4)));
    
        d->leftMinSliderRect.moveCenter(QPoint(leftMinPos, event->size().height()/4));
        d->rightMinSliderRect.moveCenter(QPoint(rightMinPos, event->size().height()/4));
    
        d->offsetSliderRect.moveCenter(QPoint(offsetPos, 3*(event->size().height()/4)));
        break;
    case Plasma::BottomEdge:
    default:
        d->leftMaxSliderRect.moveCenter(QPoint(leftMaxPos, event->size().height()/4));
        d->rightMaxSliderRect.moveCenter(QPoint(rightMaxPos, event->size().height()/4));
    
        d->leftMinSliderRect.moveCenter(QPoint(leftMinPos, 3*(event->size().height()/4)));
        d->rightMinSliderRect.moveCenter(QPoint(rightMinPos, 3*(event->size().height()/4)));
    
        d->offsetSliderRect.moveCenter(QPoint(offsetPos, event->size().height()/4));
        break;
    }

    event->accept();
}

#include "positioningruler.moc"
