////////////////////////////////////////////////////////////////////////
// animatedgridlayout.cpp                                              //
//                                                                     //
// Copyright(C) 2009 Igor Trindade Oliveira <igor.oliveira@indt.org.br>//
// Copyright(C) 2009 Adenilson Cavalcanti <adenilson.silva@idnt.org.br>//
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#include "animatedgridlayout.h"
#include "../common/proxylayout.h"

#include <QGraphicsWidget>
#include <KDebug>

AnimatedGridLayout::AnimatedGridLayout(QGraphicsLayoutItem *parent)
    : QGraphicsGridLayout(parent)
{
}

void AnimatedGridLayout::addItem(QGraphicsLayoutItem *item, int row, int column, Qt::Alignment alignment)
{
    if(!item->isLayout()) {
        ProxyLayout *proxyItem = new ProxyLayout(static_cast<QGraphicsWidget*>(item));
        QGraphicsGridLayout::addItem(proxyItem, row, column, alignment);
    } else {
        QGraphicsGridLayout::addItem(item, row, column, alignment);
    }
}

QGraphicsLayoutItem *AnimatedGridLayout::itemAt(int index) const
{
    QGraphicsLayoutItem *layoutItem = QGraphicsGridLayout::itemAt(index);

    if (layoutItem) {
        ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
        if (layout) {
            return layout->widget();
        }
    }

    return layoutItem;
}

QGraphicsLayoutItem *AnimatedGridLayout::itemAt(int row, int column) const
{
    QGraphicsLayoutItem *layoutItem = QGraphicsGridLayout::itemAt(row, column);
    if(layoutItem) {
        ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
        if(layout) {
            return layout->widget();
        }
    }

    return layoutItem;
}


void AnimatedGridLayout::removeAt(int index)
{
    QGraphicsLayoutItem *layoutItem = QGraphicsGridLayout::itemAt(index);

    ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
    if(layout) {
        delete layout;
    }
    QGraphicsGridLayout::removeAt(index);
}
