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

// Qt

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QScrollBar>

#include <KDebug>

#include "itemdelegate.h"

using namespace Notifier;

NotifierView::NotifierView(QWidget *parent)
    : QListView(parent)
{
    setIconSize(QSize(ItemDelegate::ITEM_HEIGHT,ItemDelegate::ITEM_HEIGHT));
}

NotifierView::~NotifierView()
{

}

#include "notifierview.moc"
