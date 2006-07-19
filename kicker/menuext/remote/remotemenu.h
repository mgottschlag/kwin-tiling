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

#include <kpanelmenu.h>
#include <QMap>

class RemoteMenu : public KPanelMenu
{
    Q_OBJECT

    public:
        RemoteMenu(QWidget *parent, const QStringList & /*args*/);
        ~RemoteMenu();

    private Q_SLOTS:
        void slotFilesAdded(const QString &directory);
        void slotFilesRemoved(const QStringList &fileList);
        void slotFilesChanged(const QStringList &fileList);
        void slotFilesRenamed(const QString &src, const QString &dest);

    protected Q_SLOTS:
        void initialize();
        void startWizard();
        void openRemoteDir();
        void slotExec(int id);

    private:
        QMap<int, QString> m_desktopMap;
};

#endif
