/*
    Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>

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

#include "panelsvgwidget.h"

//Qt
#include <QPainter>
#include <QResizeEvent>

#include <plasma/panelsvg.h>

PanelSvgWidget::PanelSvgWidget(QWidget *parent)
    : QWidget(parent)
{
    background = new Plasma::PanelSvg(this);
    background->setImagePath("dialogs/kickoff");
    background->setEnabledBorders(Plasma::PanelSvg::AllBorders);
    background->resizePanel(size());
    background->setElementPrefix("borderview");

    connect(background, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

void PanelSvgWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    background->paintPanel(&painter);
}

void PanelSvgWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    background->resizePanel(event->size());

    update();
}
