/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __advancedtab_h__
#define __advancedtab_h__

#include <qwidget.h>
#include <qstring.h>

#include <klineedit.h>

class AdvancedTab : public QWidget
{
    Q_OBJECT;

public:
    AdvancedTab( QWidget *parent=0, const char *name=0 );

    void apply( bool desktopFileNeedsSave );
    void reset();

signals:
    void changed( bool desktopFileNeedsSave );

public slots:
    void setDesktopFile(const QString& desktopFile);

protected slots:
    void keyButtonPressed();
    
protected:
    QString _desktopFile;
    KLineEdit* _keyEdit;
    bool _khotkeysNeedsSave;
};

#endif
