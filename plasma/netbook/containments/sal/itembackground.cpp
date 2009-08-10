/***************************************************************************
 *   Copyright 2009 by Alessandro Diaferia <alediaferia@gmail.com>         *
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
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
#include "itembackground.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include <KDebug>

#include <Plasma/FrameSvg>
#include <Plasma/Theme>

ItemBackground::ItemBackground(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      m_frameSvg(new Plasma::FrameSvg(this))
{
    setContentsMargins(0, 0, 0, 0);

    m_frameSvg->setImagePath("widgets/viewitem");
    m_frameSvg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    m_frameSvg->setCacheAllRenderedFrames(true);
    m_frameSvg->setElementPrefix("hover");

    setAcceptedMouseButtons(0);

}

ItemBackground::~ItemBackground()
{}

void ItemBackground::animatedGeometryTransform(const QRectF &newGeometry)
{
    setGeometry(newGeometry);
}

void ItemBackground::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (m_frameSvg->size() != option->rect.size()) {
        m_frameSvg->resizeFrame(option->rect.size());
    }
    m_frameSvg->paintFrame(painter, option->rect.topLeft());
}
