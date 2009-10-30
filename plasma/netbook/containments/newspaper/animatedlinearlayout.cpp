////////////////////////////////////////////////////////////////////////
// animatedlinearlayout.cpp                                            //
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

#include "animatedlinearlayout.h"
#include "../common/proxylayout.h"

#include <QGraphicsWidget>
#include <QDebug>

AnimatedLinearLayout::AnimatedLinearLayout(Qt::Orientation orientation, QGraphicsLayoutItem *parent)
    : QGraphicsLinearLayout(orientation, parent)
{
}

AnimatedLinearLayout::AnimatedLinearLayout(QGraphicsLayoutItem *parent)
    : QGraphicsLinearLayout(parent)
{
}

void AnimatedLinearLayout::addItem(QGraphicsLayoutItem *item)
{
    if(!item->isLayout()) {
        ProxyLayout *proxyItem = new ProxyLayout(static_cast<QGraphicsWidget*>(item));
        QGraphicsLinearLayout::addItem(proxyItem);
    } else
        QGraphicsLinearLayout::addItem(item);
}

void AnimatedLinearLayout::removeAt(int index)
{
    QGraphicsLayoutItem *layoutItem = QGraphicsLinearLayout::itemAt(index);

    QGraphicsLinearLayout::removeAt(index);
    ProxyLayout *layout = dynamic_cast<ProxyLayout *>(layoutItem);
    if(layout) {
        delete layout;
    }
}
