/*
    Copyright 2007 by Alexis Ménard <darktears31@gmail.com>

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

#include "notifierview.h"
#include "itemdelegate.h"

// Qt

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QScrollBar>
#include <QtGui/QHeaderView>

#include <KDebug>

using namespace Notifier;

NotifierView::NotifierView(QWidget *parent)
    : QTreeView(parent)
{
    setIconSize(QSize(ItemDelegate::ITEM_HEIGHT, ItemDelegate::ITEM_HEIGHT));
    setRootIsDecorated(false);
    setHeaderHidden(true);
}

NotifierView::~NotifierView()
{

}

void NotifierView::resizeEvent(QResizeEvent * event)
{
    //the columns after the first are squares ItemDelegate::ITEM_HEIGHT x ItemDelegate::ITEM_HEIGHT,
    //the first column takes all the remaining space
    if (header()->count() > 0) {
        header()->resizeSection(0, event->size().width() - (header()->count()-1)*ItemDelegate::ITEM_HEIGHT);
    }
}

#include "notifierview.moc"
