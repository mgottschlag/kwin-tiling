/*****************************************************************

Copyright (c) 1996-2004 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <math.h>

#include <QApplication>
#include <kdebug.h>
#include <kglobal.h>

#include "containerarea.h"
#include "containerarealayout.h"

ContainerAreaLayoutItem::ContainerAreaLayoutItem(QLayoutItem* i,
                                                 const ContainerAreaLayout* layout)
    : item(i),
      m_freeSpaceRatio(0.0),
      m_layout(layout)
{
}

ContainerAreaLayoutItem::~ContainerAreaLayoutItem()
{
    delete item;
}

int ContainerAreaLayoutItem::heightForWidth(int w) const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
    {
        return container->heightForWidth(w);
    }
    else
    {
        return item->sizeHint().height();
    }
}

int ContainerAreaLayoutItem::widthForHeight(int h) const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
    {
        return container->widthForHeight(h);
    }
    else
    {
        return item->sizeHint().width();
    }
}

bool ContainerAreaLayoutItem::isStretch() const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    return container ? container->isStretch() : false;
}

double ContainerAreaLayoutItem::freeSpaceRatio() const
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
        return qBound(0.0, container->freeSpace(), 1.0);
    else
        return m_freeSpaceRatio;
}

void ContainerAreaLayoutItem::setFreeSpaceRatio(double ratio)
{
    BaseContainer* container = dynamic_cast<BaseContainer*>(item->widget());
    if (container)
        container->setFreeSpace(ratio);
    else
        m_freeSpaceRatio = ratio;
}

Qt::Orientation ContainerAreaLayoutItem::orientation() const
{
    return m_layout->orientation();
}

QRect ContainerAreaLayoutItem::geometryR() const
{
    return m_layout->transform(geometry());
}

void ContainerAreaLayoutItem::setGeometryR(const QRect& r)
{
    setGeometry(m_layout->transform(r));
}

int ContainerAreaLayoutItem::widthForHeightR(int h) const
{
    if (orientation() == Qt::Horizontal)
    {
        return widthForHeight(h);
    }
    else
    {
        return heightForWidth(h);
    }
}

int ContainerAreaLayoutItem::widthR() const
{
    if (orientation() == Qt::Horizontal)
    {
        return geometry().width();
    }
    else
    {
        return geometry().height();
    }
}

int ContainerAreaLayoutItem::heightR() const
{
    if (orientation() == Qt::Horizontal)
    {
        return geometry().height();
    }
    else
    {
        return geometry().width();
    }
}

int ContainerAreaLayoutItem::leftR() const
{
    if (orientation() == Qt::Horizontal)
    {
        if (QApplication::isRightToLeft())
            return m_layout->geometry().right() - geometry().right();
        else
            return geometry().left();
    }
    else
    {
        return geometry().top();
    }
}

int ContainerAreaLayoutItem::rightR() const
{
    if (orientation() == Qt::Horizontal)
    {
        if (QApplication::isRightToLeft())
            return m_layout->geometry().right() - geometry().left();
        else
            return geometry().right();
    }
    else
    {
        return geometry().bottom();
    }
}



ContainerAreaLayout::ContainerAreaLayout( QWidget* parent )
    : QLayout(parent),
      m_dirty(true),
      m_orientation(Qt::Horizontal),
      m_direction( LeftToRight ),
      m_stretchEnabled(true)
{
}

ContainerAreaLayout::~ContainerAreaLayout()
{
    QLayoutItem *l;
    while ((l = takeAt(0)))
    {
        delete l;
    }
}

void ContainerAreaLayout::setOrientation( Qt::Orientation o )
{
    m_orientation = o;

    const bool reverse = QApplication::isRightToLeft();
    if ( m_orientation == Qt::Horizontal )
    {
        if ( reverse )
        {
            setDirection( LeftToRight );
        }
        else
        {
            setDirection( RightToLeft );
        }
    }
    else
    {
        if ( reverse )
        {
            setDirection( BottomToTop );
        }
        else
        {
            setDirection( TopToBottom );
        }
    }
}

QSize ContainerAreaLayout::calculateSize(SizeType sizeType) const
{
    int size = 0;
    if ( sizeType == MinimumSize )
    {
        size = Plasma::sizeValue(Plasma::SizeNormal);
    }
    else
    {
        size = Plasma::sizeValue(Plasma::SizeTiny);
    }

    if (orientation() == Qt::Horizontal)
    {
        return QSize(widthForHeight(size), size);
    }
    else
    {
        return QSize(size, heightForWidth(size));
    }
}

QSize ContainerAreaLayout::sizeHint() const
{
    return calculateSize(SizeHint);
}

QSize ContainerAreaLayout::minimumSize() const
{
    return calculateSize(MinimumSize);
}

/*
bool ContainerAreaLayout::hasHeightForWidth() const
{
    return true;
}
*/

int ContainerAreaLayout::heightForWidth(int w) const
{
    int height = 0;
    foreach (Item *item, m_items)
    {
        height += qMax(0, item->heightForWidth(w));
    }

    return height;
}

int ContainerAreaLayout::widthForHeight(int h) const
{
    int width = 0;
    foreach (Item *item, m_items)
    {
        width += qMax(0, item->widthForHeight(h));
    }

    return width;
}

void ContainerAreaLayout::invalidate()
{
    // use this in the future
    m_dirty = true;
    QLayout::invalidate();
}

int ContainerAreaLayout::count() const
{
    return m_items.count();
}

QLayoutItem *ContainerAreaLayout::itemAt( int index ) const
{
    Item *caItem = m_items.value(index);
    if (caItem)
    {
        return caItem->item;
    }
    else
    {
        return 0;
    }
}

QLayoutItem *ContainerAreaLayout::takeAt( int index )
{
    if (index >= 0 && index < m_items.count())
    {
        Item *it = m_items.takeAt(index);
        return it->item;
    }
    return 0;
}

Qt::Orientations ContainerAreaLayout::expandingDirections() const
{
    return m_orientation;
}

void ContainerAreaLayout::setGeometry(const QRect& rect)
{
    //RESEARCH: when can we short curcuit this?
    //          maybe a dirty flag to be set when we have containers
    //          that needs laying out?

    if ( m_dirty || rect != geometry() )
    {
        QLayout::setGeometry(rect);
        if ( m_dirty )
        {
            m_dirty = false;
        }

        float totalFreeSpace = qMax(0, widthR() - widthForHeightR(heightR()));
        int occupiedSpace = 0;

        int n = m_items.count();
        for (int i = 0; i < n; i++)
        {
            Item *cur = m_items.at(i);
            Item *next = m_items.value(i + 1);      // does range checking for us, not that much slower..

            double fs = cur->freeSpaceRatio();
            double freeSpace = fs * totalFreeSpace;
            int pos = int(rint(freeSpace)) + occupiedSpace;

            int w = cur->widthForHeightR(heightR());
            occupiedSpace += w;
            if (m_stretchEnabled && cur->isStretch())
            {
                if (next)
                {
                    double nfs = next->freeSpaceRatio();
                    w += int((nfs - fs)*totalFreeSpace);
                }
                else
                {
                    w = widthR() - pos;
                }
            }
            cur->setGeometryR(QRect(pos, 0, w, heightR()));
        }
    }
}

void ContainerAreaLayout::addItem(QLayoutItem* item)
{
    Item *it = new Item(item, this);
    m_items.append(it);
    invalidate();
}

void ContainerAreaLayout::setStretchEnabled(bool enable)
{
    if (m_stretchEnabled != enable)
    {
        m_stretchEnabled = enable;
        invalidate();
    }
}

int ContainerAreaLayout::distanceToPrevious(ItemList::const_iterator it) const
{
    if (it != m_items.constEnd())
    {
        Item* cur = *it;
        Item* prev = 0;

        if (it != m_items.constBegin())
        {
            --it;
            prev = *it;
        }

        return prev ? cur->leftR() - prev->leftR() - prev->widthForHeightR(heightR()) :
                      cur->leftR() - leftR();
    }

    return 0;
}

void ContainerAreaLayout::updateFreeSpaceValues()
{
    int freeSpace = qMax(0, widthR() - widthForHeightR(heightR()));
    double fspace = 0;

    ItemList::const_iterator itEnd = m_items.constEnd();
    for (ItemList::const_iterator it = m_items.constBegin();
         it != itEnd;
         ++it)
    {
        int distance = distanceToPrevious(it);

        if (distance < 0)
        {
            distance = 0;
        }

        fspace += distance;
        double ssf = ( freeSpace == 0 ? 0 : fspace/freeSpace );
        if (ssf > 1)
        {
            ssf = 1;
        }
        if (ssf < 0)
        {
            ssf = 0;
        }

        (*it)->setFreeSpaceRatio(ssf);
    }
}

void ContainerAreaLayout::insertIntoFreeSpace(QWidget* widget, QPoint insertionPoint)
{
    if (!widget)
    {
        return;
    }

    addWidget(widget);

    Item* item = m_items.last();
    if (!item)
    {
        // this should never happen as we just added the item above
        // but we do this to be safe.
        return;
    }

    ItemList::iterator currentIt = m_items.begin();
    ItemList::iterator itEnd = m_items.end();
    if (currentIt == itEnd)
    {
        // this shouldn't happen either, but again... we try and be safe
        return;
    }

    ItemList::iterator nextIt = currentIt;
    ++nextIt;

    if (nextIt == itEnd)
    {
        // first item in!
        item->setGeometryR(QRect(insertionPoint.x(), insertionPoint.y(), widget->width(), widget->height()));
        updateFreeSpaceValues();
        return;
    }

    int insPos = (orientation() == Qt::Horizontal) ? insertionPoint.x(): insertionPoint.y();
    Item *next;
    Item *current;
    for (; nextIt != itEnd; ++currentIt, ++nextIt)
    {
        next = *nextIt;
        current = *currentIt;
        if (current == item || next == item)
        {
            continue;
        }

        if (insPos == 0)
        {
            if (current->rightR() + 3 < next->leftR())
            {
                insPos = current->rightR();
                break;
            }
        }
        else
        {
            if (currentIt == m_items.begin() &&
                (current->leftR() > insPos ||
                 (current->leftR() <= insPos) &&
                 (current->rightR() > insPos)))
            {
                break;
            }

            if ((current->rightR() < insPos) && (next->leftR() > insPos))
            {
                // Free space available at insertion point!
                if (insPos + item->widthR() > next->leftR())
                {
                    // We have overlap on the right, move to the left
                    insPos = next->leftR() - item->widthR();
                    if (insPos < current->rightR())
                    {
                        // We have overlap on the left as well, move to the right
                        insPos = current->rightR();
                        // We don't fit entirely, let updateFreeSpaceValues sort it out
                    }
                }
                current = next;
                break;
            }

            if ((next->leftR() <= insPos) && (next->rightR() > insPos))
            {
                // Insert at the location of next
                current = next;
                insPos = next->leftR();
                break;
            }
        }
    }

    QRect geom = item->geometryR();
    geom.moveLeft(insPos);
    item->setGeometryR(geom);
    widget->setGeometry(transform(geom)); // widget isn't shown, layout not active yet

    if (current)
    {
        m_items.removeLast();
        int index = m_items.indexOf(current);

        if (index == 0)
        {
            m_items.prepend(item);
        }
        else if (index < 0)
        {
            // yes, we just removed it from the end, but
            // if we remove it afterwards and it insertIt
            // was our item then we end up with a bad iterator
            m_items.append(item);
        }
        else
        {
            m_items.insert(index, item);
        }
    }

    updateFreeSpaceValues();
}

void ContainerAreaLayout::moveContainerSwitch( QWidget* container, int distance )
{
    const bool horizontal    = orientation() == Qt::Horizontal;
    const bool reverseLayout = QApplication::isRightToLeft();

    if (horizontal && reverseLayout)
    {
        distance = -distance;
    }

    const bool forward = distance > 0;

    // Get the iterator 'it' pointing to 'moving'.
    ItemList::const_iterator it = m_items.constBegin();
    ItemList::const_iterator itEnd = m_items.constEnd();
    while (it != itEnd && (*it)->item->widget() != container)
    {
        ++it;
    }

    if (it == itEnd)
    {
        return;
    }


    Item* moving = *it;
    Item* next;
    if(forward) {
        ++it;
        if(it != m_items.constEnd()) {
            next = *it;
        } else {
            next = 0;
        }
    } else {
        if(it != m_items.constBegin()) {
            --it;
            next = *it;
        } else {
            next = 0;
        }
    }
    Item* last = moving;

    while (next)
    {
        // Calculate the position and width of the virtual container
        // containing 'moving' and 'next'.
        int tpos = forward ? next->leftR() - moving->widthR()
                           : next->leftR();
        int tsize = moving->widthR() + next->widthR();

        // Determine the middle of the containers.
        int tmiddle = tpos + tsize / 2;
        int movingMiddle = moving->leftR() + distance + moving->widthR() / 2;

        // Check if the middle of 'moving' has moved past the middle of the
        // virtual container.
        if (!forward && movingMiddle > tmiddle
          || forward && movingMiddle < tmiddle)
            break;

        // Move 'next' to the other side of 'moving'.
        QRect geom = next->geometryR();
        if (forward)
            geom.moveLeft(geom.left() - moving->widthR());
        else
            geom.moveLeft(geom.left() + moving->widthR());
        next->setGeometryR(geom);

        // Store 'next'. It may become null in the next iteration, but we
        // need its value later.
        last = next;
        if(forward) {
            ++it;
            if(it != m_items.constEnd()) {
                next = *it;
            } else {
                next = 0;
            }
        } else {
            if(it != m_items.constBegin()) {
                --it;
                next = *it;
            } else {
                next = 0;
            }
        }
    }

    int newPos = moving->leftR() + distance;
    if (last != moving)
    {
        // The case that moving has switched position with at least one other container.
        newPos = forward ? qMax(newPos, last->rightR() + 1)
                         : qMin(newPos, last->leftR() - moving->widthR());

        // Move 'moving' to its new position in the container list.
        int index = m_items.indexOf(moving);

        if (index >= 0)
        {
            ItemList::iterator itMoving = m_items.begin()+index;
            ItemList::iterator itLast = itMoving;
            if (forward)
            {
                ++itLast;
                ++itLast;
            }
            else
            {
                --itLast;
            }

            m_items.erase(itMoving);

            if (itLast == m_items.end())
            {
                if (forward)
                {
                    m_items.append(moving);
                }
                else
                {
                    m_items.push_front(moving);
                }
            }
            else
            {
                m_items.insert(itLast, moving);
            }
        }
    }
    else if (next)
    {
        // Make sure that the moving container will not overlap the next one.
        newPos = forward ? qMin(newPos, next->leftR() - moving->widthR())
                         : qMax(newPos, next->rightR() + 1);
    }

    // Move the container to its new position and prevent it from moving outside the panel.
    QRect geom = moving->geometryR();
    distance = qBound(0, newPos, widthR() - moving->widthR());
    geom.moveLeft(distance);
    moving->setGeometryR(geom);

    updateFreeSpaceValues();
}

int ContainerAreaLayout::moveContainerPush(QWidget* a, int distance)
{
    const bool horizontal    = orientation() == Qt::Horizontal;
    const bool reverseLayout = QApplication::isRightToLeft();

    // Get the iterator 'it' pointing to the layoutitem representing 'a'.
    ItemList::const_iterator it = m_items.constBegin();
    ItemList::const_iterator itEnd = m_items.constEnd();
    while (it != itEnd && (*it)->item->widget() != a)
    {
        ++it;
    }

    if (it != itEnd)
    {
        if (horizontal && reverseLayout)
        {
            distance = -distance;
        }

        int retVal = moveContainerPushRecursive(it, distance);
        updateFreeSpaceValues();
        if (horizontal && reverseLayout)
        {
            retVal = -retVal;
        }
        return retVal;
    }
    else
    {
        return 0;
    }
}

int ContainerAreaLayout::moveContainerPushRecursive(ItemList::const_iterator it,
                                                    int distance)
{
    if (distance == 0)
        return 0;

    const bool forward = distance > 0;

    int available; // Space available for the container to move.
    int moved;     // The actual distance the container will move.
    Item* cur  = *it;
    forward ? ++it : --it;
    Item* next = (it != m_items.constEnd()) ? *it : 0;

    if (!next)
    {
        available = forward ? rightR() - cur->rightR()
                            : -cur->leftR();
    }
    else
    {
        available = forward ? next->leftR()  - cur->rightR() - 1
                            : next->rightR() - cur->leftR()  + 1;

        if (!forward && distance < available
          || forward && distance > available)
            available += moveContainerPushRecursive(it, distance - available);
    }
    moved = forward ? qMin(distance, available)
                    : qMax(distance, available);

    QRect geom = cur->geometryR();
    geom.moveLeft(geom.left() + moved);
    cur->setGeometryR(geom);

    return moved;
}

QRect ContainerAreaLayout::transform(const QRect& r) const
{
    if (orientation() == Qt::Horizontal)
    {
        if (QApplication::isRightToLeft())
        {
            QRect t = r;
            t.moveLeft(geometry().right() - r.right());
            return t;
        }
        else
        {
            return r;
        }
    }
    else
    {
        return QRect(r.y(), r.x(), r.height(), r.width());
    }
}

int ContainerAreaLayout::widthForHeightR(int h) const
{
    if (orientation() == Qt::Horizontal)
    {
        return widthForHeight(h);
    }
    else
    {
        return heightForWidth(h);
    }
}

int ContainerAreaLayout::widthR() const
{
    if (orientation() == Qt::Horizontal)
    {
        return geometry().width();
    }
    else
    {
        return geometry().height();
    }
}

int ContainerAreaLayout::heightR() const
{
    if (orientation() == Qt::Horizontal)
    {
        return geometry().height();
    }
    else
    {
        return geometry().width();
    }
}

int ContainerAreaLayout::leftR() const
{
    if (orientation() == Qt::Horizontal)
        return geometry().left();
    else
        return geometry().top();
}

int ContainerAreaLayout::rightR() const
{
    if (orientation() == Qt::Horizontal)
        return geometry().right();
    else
        return geometry().bottom();
}

