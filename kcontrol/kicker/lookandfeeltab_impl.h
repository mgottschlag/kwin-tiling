/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 */

#ifndef __lookandfeeltab_impl_h__
#define __lookandfeeltab_impl_h__

#include <qstring.h>
#include <qpixmap.h>

#include "lookandfeeltab.h"

class LookAndFeelTab : public LookAndFeelTabBase
{
    Q_OBJECT

public:
    LookAndFeelTab( KickerConfig *parent=0, const char* name=0 );

    void load();
    void save();
    void defaults();
    void show();

signals:
    void changed();

protected slots:
    void browse_theme();
    void hideButtonsSet(int index);
    
private:
    QString theme;
    QPixmap theme_preview;
    KickerConfig* kconf;
};

#endif
