/* This file is part of the KDE project
   Copyright (c) 2004 Kevin Ottens <ervin ipsquad net>

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

#ifndef REMOTEMENU_H
#define REMOTEMENU_H

#include <kdirnotify.h>
#include <kpanelmenu.h>
#include <qmap.h>

class RemoteMenu : public KPanelMenu, public KDirNotify
{
    Q_OBJECT
    K_DCOP

    public:
        RemoteMenu(QWidget *parent, const char *name,
                   const QStringList & /*args*/);
        ~RemoteMenu();

    k_dcop:
        virtual ASYNC FilesAdded(const KUrl &directory);
        virtual ASYNC FilesRemoved(const KUrl::List &fileList);
        virtual ASYNC FilesChanged(const KUrl::List &fileList);
        virtual ASYNC FilesRenamed(const KUrl &src, const KUrl &dest);

    protected Q_SLOTS:
        void initialize();
        void startWizard();
        void openRemoteDir();
        void slotExec(int id);

    private:
        QMap<int, QString> m_desktopMap;
};

#endif
