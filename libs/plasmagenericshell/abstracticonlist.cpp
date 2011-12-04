/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
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

#include "abstracticonlist.h"

#include <cmath>

#include <QGraphicsSceneWheelEvent>
#include <QGraphicsView>
#include <QHash>
#include <QToolButton>

#include <KIconLoader>
#include <KIcon>
#include <KPushButton>

#include <Plasma/Animation>
#include <Plasma/Animator>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Theme>
#include <Plasma/ToolButton>

const int ICON_SIZE = 70;
const int SEARCH_DELAY = 300;

namespace Plasma
{

AbstractIconList::AbstractIconList(Plasma::Location loc, QGraphicsItem *parent)
    : Plasma::ScrollWidget(parent),
      m_orientation((loc == Plasma::LeftEdge || loc == Plasma::RightEdge) ? Qt::Vertical : Qt::Horizontal),
      m_location(loc),
      m_searchDelayTimer(new QTimer(this)),
      m_iconSize(16)
{
    setOverflowBordersVisible(false);
    m_searchDelayTimer->setSingleShot(true);
    m_searchDelayTimer->setInterval(SEARCH_DELAY);
    connect(m_searchDelayTimer, SIGNAL(timeout()), this, SLOT(setSearch()));

    init();
}

AbstractIconList::~AbstractIconList()
{

    //FIXME: if the follow foreach looks silly, that's because it is.
    //       but Qt 4.6 currently has a devastating bug that crashes
    //       when we don't do precisely this
    foreach (QGraphicsWidget *item, m_allAppletsHash) {
        item->setParentItem(0);
        item->deleteLater();
    }
}

void AbstractIconList::init()
{
    //init applets list
    m_appletListWidget = new QGraphicsWidget(this);
    m_appletListLinearLayout = new QGraphicsLinearLayout(m_orientation, m_appletListWidget);
    m_appletListLinearLayout->setSpacing(0);
    setWidget(m_appletListWidget);
    adjustFromOrientation();
}

Plasma::Location AbstractIconList::location()
{
    return m_location;
}

void AbstractIconList::setLocation(Plasma::Location location)
{
    if (m_location == location) {
        return;
    }

    m_location = location;
    m_orientation = ((location == Plasma::LeftEdge || location == Plasma::RightEdge)?Qt::Vertical:Qt::Horizontal);
    adjustFromOrientation();
}

void AbstractIconList::adjustFromOrientation()
{
    m_appletListLinearLayout->setOrientation(m_orientation);
    if (m_orientation == Qt::Horizontal) {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_appletListWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    } else {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_appletListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

void AbstractIconList::searchTermChanged(const QString &text)
{
    m_searchString = text;
    m_searchDelayTimer->start();
}

void AbstractIconList::setSearch()
{
    //pass it down to the subclass
    setSearch(m_searchString);
    updateList();
}

//all items are always in the list. filter updates just hide/show.
//what do we need the visible-list for? getting position, finding the end of the list,
//calculating how many fit on the list...
//TODO it might be a bit easier if we explicitly told all icons to be the same (set) size.
void AbstractIconList::showIcon(AbstractIcon *icon)
{
    if (icon) {
        icon->expand();
        m_currentAppearingAppletsOnList.append(icon);
    }
}

//FIXME this isn't ideal. it'd be nicer if I could just use setVisible
//but I haven't eliminated the need for the visible-list yet.
void AbstractIconList::hideIcon(AbstractIcon *icon)
{
    if (icon) {
        icon->collapse();
    }

    m_currentAppearingAppletsOnList.removeAll(icon);
}

//a faster way, given that we still need the visible-list
void AbstractIconList::hideAllIcons()
{
    foreach (QWeakPointer<AbstractIcon> pIcon, m_currentAppearingAppletsOnList) {
        if (pIcon.isNull()) {
            continue;
        }
        AbstractIcon *icon = pIcon.data();
        icon->collapse();
    }
    m_currentAppearingAppletsOnList.clear();
}

void AbstractIconList::removeIcon(AbstractIcon *icon)
{
    m_appletListLinearLayout->removeItem(icon);
    disconnect(icon, SIGNAL(clicked(Plasma::AbstractIcon*)), this, SLOT(itemSelected(Plasma::AbstractIcon*)));
}

void AbstractIconList::addIcon(AbstractIcon *icon)
{
    icon->setParent(m_appletListWidget); //FIXME redundant?

    //we don't add it to the hash here because we don't know its id.

    //FIXME surely this should be checked in the setter :P
    if (m_iconSize != AbstractIcon::DEFAULT_ICON_SIZE) {
        icon->setIconSize(m_iconSize);
    }

    m_appletListLinearLayout->addItem(icon);
    m_appletListLinearLayout->setAlignment(icon, Qt::AlignHCenter);
    showIcon(icon);

    connect(icon, SIGNAL(clicked(Plasma::AbstractIcon*)), this, SLOT(itemSelected(Plasma::AbstractIcon*)));
}

void AbstractIconList::itemSelected(Plasma::AbstractIcon *icon)
{
    if (m_selectedItem) {
        m_selectedItem.data()->setSelected(false);
    }

    icon->setSelected(true);
    m_selectedItem = icon;
}

void AbstractIconList::updateList()
{
    //pure virtual
    updateVisibleIcons();

    m_appletListWidget->adjustSize();

    updateGeometry();
    resetScroll();
}

void AbstractIconList::resetScroll()
{
    m_appletListWidget->setPos(0,0);
}

qreal AbstractIconList::visibleStartPosition()
{
    if (m_orientation == Qt::Horizontal) {
        return mapToItem(m_appletListWidget, boundingRect().left(), 0).x();
    } else {
        return mapToItem(m_appletListWidget, 0, boundingRect().top()).y();
    }
}

qreal AbstractIconList::visibleEndPosition()
{
    if (m_orientation == Qt::Horizontal) {
        return mapToItem(m_appletListWidget, boundingRect().right(), 0).x();
    } else {
        return mapToItem(m_appletListWidget, 0, boundingRect().bottom()).y();
    }
}

qreal AbstractIconList::listSize()
{
    if (m_orientation == Qt::Horizontal) {
        return m_appletListWidget->boundingRect().size().width();
    } else {
        return m_appletListWidget->boundingRect().size().height();
    }
}

qreal AbstractIconList::windowSize()
{
    return (visibleEndPosition() - visibleStartPosition());
}

qreal AbstractIconList::itemPosition(int i)
{
    AbstractIcon *applet = m_currentAppearingAppletsOnList.value(i).data(); 
    if (!applet) {
        return 0;
    }

    if (m_orientation == Qt::Horizontal) {
        return applet->pos().x();
    } else {
        return applet->pos().y();
    }
}

void AbstractIconList::setIconSize(int size)
{
    if (m_iconSize == size || size < 16) {
        return;
    }

    m_iconSize = size;

    foreach (AbstractIcon *applet, m_allAppletsHash) {
        applet->setIconSize(size);
    }

    adjustSize();
}

int AbstractIconList::iconSize() const
{
    return m_iconSize;
}

} // namespace Plasma

