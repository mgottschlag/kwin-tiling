/*
 * colors.h
 *
 * Copyright (c) 1999 Daniel M. Duley <mosfet@jorsm.com>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __KCONTROL_COLORS_H
#define __KCONTROL_COLORS_H

#include <kcontrol.h>
#include <kcolorbtn.h>
#include <qcombobox.h>

class KColorConfig : public KConfigWidget
{
  Q_OBJECT
public:
    enum ItemID{ Panel_ID=0, DeskFg_ID, DeskBg_ID, AppIcon_ID,
        Taskbar_ID, TaskBtnFg_ID, TaskBtnBg_ID, BlinkHigh_ID, BlinkLow_ID};
    
    KColorConfig( QWidget *parent=0, const char* name=0 );
    ~KColorConfig( );
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();
protected slots:
    void colorSlot(const QColor &color);
    void itemSlot(int index);
protected:
    QColor colors[9];
    KColorButton *currentColor;
    QComboBox *itemCombo;
};

#endif

