/* This file is part of the KDE Project
   Copyright (c) 2006 Lukas Tinkl <ltinkl@suse.cz>
   Copyright (c) 2008 Lubos Lunak <l.lunak@suse.cz>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _FREESPACENOTIFIER_H_
#define _FREESPACENOTIFIER_H_

#include <Qt/qtimer.h>

#include <kdialog.h>

class Ui_FreeSpaceWidget;

class FreeSpaceNotifier
: public QObject
{
    Q_OBJECT
    public:
        FreeSpaceNotifier( QObject* parent = NULL );
        virtual ~FreeSpaceNotifier();
    private slots:
        void checkFreeDiskSpace();
        void resetLastAvailable();
        void slotYes();
        void slotNo();
        void slotCancel();
    private:
        void cleanupDialog( long newLimit );
        QTimer timer;
        QTimer* lastAvailTimer;
        KDialog* dialog;
        Ui_FreeSpaceWidget* widget;
        long limit;
        long lastAvail; // used to supress repeated warnings when available space hasn't changed
};

#endif
