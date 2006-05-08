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

#ifndef SYSTEMMENU_H
#define SYSTEMMENU_H

#include <kpanelmenu.h>
#include <kurl.h>
#include <kdirlister.h>
#include <kfileitem.h>

class SystemMenu : public KPanelMenu
{
    Q_OBJECT

    public:
        SystemMenu(QWidget *parent, const QStringList & /*args*/);
        ~SystemMenu();

    protected Q_SLOTS:
        void initialize();
        void slotExec(int id);

        void slotCompleted();

    private:
        void append(const QString &icon, const KUrl &url, const QString &label);
        KDirLister m_dirLister;
        KFileItemList m_entries;
        QMap<int, KUrl> m_urlMap;
};

#endif
