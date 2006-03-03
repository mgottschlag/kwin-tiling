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

#include "systemmenu.h"

#include <krun.h>
#include <kiconloader.h>
#include <qpixmap.h>

K_EXPORT_KICKER_MENUEXT(systemmenu, SystemMenu)


SystemMenu::SystemMenu(QWidget *parent, const char *name,
                       const QStringList &/*args*/)
  : KPanelMenu( parent, name)
{
    connect( &m_dirLister, SIGNAL( completed() ),
             this, SLOT( slotCompleted() ) );

    m_dirLister.openURL(KUrl("system:/"));
}

SystemMenu::~SystemMenu()
{
}

void SystemMenu::append(const QString &icon, const KUrl &url,
                        const QString &label)
{
    int id = insertItem(SmallIconSet(icon), label);
    m_urlMap.insert(id, url);
}

void SystemMenu::initialize()
{
    if (isVisible()) return;

    clear();

    if (m_entries.isEmpty())
    {
        insertItem(i18n("Empty..."));
        return;
    }

    m_urlMap.clear();

    KFileItemList::ConstIterator it = m_entries.begin();
    KFileItemList::ConstIterator end = m_entries.end();

    for (; it!=end; ++it)
    {
        QString icon = (*it)->iconName();
        KUrl url = (*it)->url();
        QString name = (*it)->name();
        append(icon, url, name);
    }
}

void SystemMenu::slotExec(int id)
{
    if(!m_urlMap.contains(id)) return;

    new KRun(m_urlMap[id], this); // will delete itself
}

void SystemMenu::slotCompleted()
{
    m_entries = m_dirLister.items(KDirLister::AllItems);
    setInitialized(false);
}

#include "systemmenu.moc"
