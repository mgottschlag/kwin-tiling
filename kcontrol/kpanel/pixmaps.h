/*
 * pixmaps.h
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

#ifndef __KCONTROL_PIXMAPS_H
#define __KCONTROL_PIXMAPS_H

#include <kcontrol.h>
#include <kiconloaderdialog.h>
#include <qcombobox.h>
#include <qcheckbox.h>

class KPixmapConfig : public KConfigWidget
{
  Q_OBJECT
public:
    enum ItemID{ Panel_ID=0, AppIcon_ID, Taskbar_ID, TaskBtn_ID};
    
    KPixmapConfig( QWidget *parent=0, const char* name=0 );
    ~KPixmapConfig( );
    virtual void loadSettings();
    virtual void saveSettings();
    virtual void applySettings();
protected slots:
    void iconSlot(const char *icon);
    void itemSlot(int index);
    void enableSlot();
protected:
    QString icons[4];
    bool enabled[4];
    KIconLoader ldr;
    KIconLoaderButton *currentIcon;
    QComboBox *itemCombo;
    QCheckBox *enableBox;
};

#endif

