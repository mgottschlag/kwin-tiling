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

#include "remotemenu.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <kservice.h>

#include <qpixmap.h>
#include <qdir.h>
#include <qtimer.h>

#include "kickerSettings.h"

#define WIZARD_SERVICE "knetattach"

K_EXPORT_KICKER_MENUEXT(remotemenu, RemoteMenu)


RemoteMenu::RemoteMenu(QWidget *parent, const QStringList &/*args*/)
  : KPanelMenu(parent), KDirNotify()
{
    KGlobal::dirs()->addResourceType("remote_entries",
    KStandardDirs::kde_default("data") + "remoteview");

    QString path = KGlobal::dirs()->saveLocation("remote_entries");

    QDir dir = path;
    if (!dir.exists())
    {
        dir.cdUp();
        dir.mkdir("remoteview");
    }
}

RemoteMenu::~RemoteMenu()
{
}

void RemoteMenu::initialize()
{
    int id = 0;

    id = insertItem(SmallIconSet("wizard"), i18n("Add Network Folder"));
    connectItem(id, this, SLOT(startWizard()));
    id = insertItem(SmallIconSet("kfm"), i18n("Manage Network Folders"));
    connectItem(id, this, SLOT(openRemoteDir()));

    insertSeparator();

    m_desktopMap.clear();
    QStringList names_found;
    QStringList dirList = KGlobal::dirs()->resourceDirs("remote_entries");

    QStringList::ConstIterator dirpath = dirList.begin();
    QStringList::ConstIterator end = dirList.end();
    for(; dirpath!=end; ++dirpath)
    {
        QDir dir = *dirpath;
        if (!dir.exists()) continue;

        QStringList filenames
            = dir.entryList( QDir::Files | QDir::Readable );

        QStringList::ConstIterator name = filenames.begin();
        QStringList::ConstIterator endf = filenames.end();

        for(; name!=endf; ++name)
        {
            if (!names_found.contains(*name))
            {
                names_found.append(*name);
                QString filename = *dirpath+*name;
                KDesktopFile desktop(filename);
                id = insertItem(SmallIconSet(desktop.readIcon()), desktop.readName());
                connectItem(id, this, SLOT(slotExec(int)));
                m_desktopMap[id] = filename;
            }
        }
    }
}

void RemoteMenu::startWizard()
{
    KUrl url;
    KService::Ptr service = KService::serviceByDesktopName(WIZARD_SERVICE);

    if (service && service->isValid())
    {
        url.setPath(locate("apps", service->desktopEntryPath()));
        new KRun(url, 0, true); // will delete itself
    }
}

void RemoteMenu::openRemoteDir()
{
    new KRun(KUrl("remote:/"), this);
}

void RemoteMenu::slotExec(int id)
{
    if (m_desktopMap.contains(id))
    {
        new KRun(m_desktopMap[id], this);
    }
}

ASYNC RemoteMenu::FilesAdded(const KUrl &directory)
{
    if (directory.protocol()=="remote") reinitialize();
}

ASYNC RemoteMenu::FilesRemoved(const KUrl::List &fileList)
{
    KUrl::List::ConstIterator it = fileList.begin();
    KUrl::List::ConstIterator end = fileList.end();
    
    for (; it!=end; ++it)
    {
        if ((*it).protocol()=="remote")
        {
            reinitialize();
            return;
        }
    }
}

ASYNC RemoteMenu::FilesChanged(const KUrl::List &fileList)
{
    FilesRemoved(fileList);
}

ASYNC RemoteMenu::FilesRenamed(const KUrl &src, const KUrl &dest)
{
    if (src.protocol()=="remote" || dest.protocol()=="remote")
        reinitialize();
}

#include "remotemenu.moc"
