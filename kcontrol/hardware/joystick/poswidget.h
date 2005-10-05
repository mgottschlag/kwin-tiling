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
#ifndef _POSWIDGET_H_
#define _POSWIDGET_H_

#include <qwidget.h>
//Added by qt3to4:
#include <QPaintEvent>

/**
  Widget to display the joystick-selected (x,y) position
*/
class PosWidget : public QWidget
{
  Q_OBJECT
  
  public:
    PosWidget(QWidget *parent = 0, const char *name = 0);

    void changeX(int x);
    void changeY(int y);

    // define if a trace of the moving joystick shall be displayed
    // setting it to false will erase all previous marks from the widget
    // NOTE: the traced positions are not stored and will be erased if the widget is covered/redisplayed
    void showTrace(bool t);

  protected:
    virtual void paintEvent(QPaintEvent *);

  private:
    void eraseOld();

  private:
    int x, y;
    bool trace;
};

#endif
