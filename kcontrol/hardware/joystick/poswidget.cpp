/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller                                   *
 *   m.koller@surfeu.at                                                    *
 *   This file is part of the KDE Control Center Module for Joysticks      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "poswidget.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QPaintEvent>

#define XY_WIDTH 220
#define MARK_WIDTH 10

//-----------------------------------------------------------------

PosWidget::PosWidget(QWidget *parent, const char *name)
  : QWidget(parent, name, Qt::WNoAutoErase), x(0), y(0), trace(false)
{
  setMinimumSize(XY_WIDTH, XY_WIDTH);
  setMaximumSize(XY_WIDTH, XY_WIDTH);
  setPaletteBackgroundColor(Qt::white);
}

//-----------------------------------------------------------------

void PosWidget::paintEvent(QPaintEvent *)
{
  QPainter paint(this);

  paint.drawRect(0, 0, width(), height());
  paint.setPen(Qt::gray);

  // draw a center grid
  paint.drawLine(XY_WIDTH/2, 1,
                 XY_WIDTH/2, XY_WIDTH - 2);

  paint.drawLine(1,            XY_WIDTH/2,
                 XY_WIDTH - 2, XY_WIDTH/2);

  // draw the current position marker
  paint.setPen(Qt::blue);

  paint.drawLine(x - MARK_WIDTH/2, y - MARK_WIDTH/2,
                 x + MARK_WIDTH/2, y + MARK_WIDTH/2);

  paint.drawLine(x - MARK_WIDTH/2, y + MARK_WIDTH/2,
                 x + MARK_WIDTH/2, y - MARK_WIDTH/2);
}

//-----------------------------------------------------------------

void PosWidget::changeX(int newX)
{
  // transform coordinates from joystick to widget coordinates
  newX = int((newX/65535.0)*XY_WIDTH + XY_WIDTH/2);

  if ( x == newX ) return;  // avoid unnecessary redraw

  eraseOld();

  x = newX;
}

//-----------------------------------------------------------------

void PosWidget::changeY(int newY)
{
  // transform coordinates from joystick to widget coordinates
  newY = int((newY/65535.0)*XY_WIDTH + XY_WIDTH/2);

  if ( y == newY ) return;  // avoid unnecessary redraw

  eraseOld();

  y = newY;
}

//-----------------------------------------------------------------

void PosWidget::showTrace(bool t)
{
  trace = t;

  if ( !trace )
  {
    erase();
    update();
  }
}

//-----------------------------------------------------------------

void PosWidget::eraseOld()
{
  QPainter paint(this);

  //paint.eraseRect(x - MARK_WIDTH/2, y - MARK_WIDTH/2, MARK_WIDTH + 1, MARK_WIDTH + 1);

  // erase previous cross (don't use eraseRect() so that trace flags will be not destroyed so much)
  paint.setPen(Qt::white);

  paint.drawLine(x - MARK_WIDTH/2, y - MARK_WIDTH/2,
                 x + MARK_WIDTH/2, y + MARK_WIDTH/2);

  paint.drawLine(x - MARK_WIDTH/2, y + MARK_WIDTH/2,
                 x + MARK_WIDTH/2, y - MARK_WIDTH/2);

  if ( trace )  // show previous position with a smaller black cross
  {
    paint.setPen(Qt::black);

    paint.drawLine(x - 2, y - 2,
                   x + 2, y + 2);

    paint.drawLine(x - 2, y + 2,
                   x + 2, y - 2);
  }

  update();
}

//-----------------------------------------------------------------

#include "poswidget.moc"
