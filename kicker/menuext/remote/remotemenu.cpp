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
#include <kdirnotify.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kicon.h>
#include <kdesktopfile.h>
#include <kservice.h>

#include <QPixmap>
#include <QDir>
#include <QTimer>

#include "kickerSettings.h"

#define WIZARD_SERVICE "knetattach"

K_EXPORT_KICKER_MENUEXT(remotemenu, RemoteMenu)


RemoteMenu::RemoteMenu(QWidget *parent, const QStringList &/*args*/)
  : KPanelMenu(parent)
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

    org::kde::KDirNotify *kdirnotify = new org::kde::KDirNotify(QString(), QString(), QDBusConnection::sessionBus(), this);
    connect(kdirnotify, SIGNAL(FileRenamed(QString,QString)), SLOT(slotFileRenamed(QString,QString)));
    connect(kdirnotify, SIGNAL(FilesAdded(QString)), SLOT(slotFilesAdded(QString)));
    connect(kdirnotify, SIGNAL(FilesChanged(QStringList)), SLOT(slotFilesChanged(QStringList)));
    connect(kdirnotify, SIGNAL(FilesRemoved(QStringList)), SLOT(slotFilesRemoved(QStringList)));
}

RemoteMenu::~RemoteMenu()
{
}

void RemoteMenu::initialize()
{
    int id = 0;

    id = insertItem(KIcon("wizard"), i18n("Add Network Folder"));
    connectItem(id, this, SLOT(startWizard()));
    id = insertItem(KIcon("kfm"), i18n("Manage Network Folders"));
    connectItem(id, this, SLOT(openRemoteDir()));

    addSeparator();

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
                id = insertItem(KIcon(desktop.readIcon()), desktop.readName());
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
        url.setPath(KStandardDirs::locate("apps", service->desktopEntryPath()));
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

void RemoteMenu::slotFilesAdded(const QString &directory)
{
    if (KUrl(directory).protocol()=="remote") reinitialize();
}

void RemoteMenu::slotFilesRemoved(const QStringList &fileList)
{
    QStringList::ConstIterator it = fileList.begin();
    QStringList::ConstIterator end = fileList.end();
    
    for (; it!=end; ++it)
    {
        if (KUrl(*it).protocol()=="remote")
        {
            reinitialize();
            return;
        }
    }
}

void RemoteMenu::slotFilesChanged(const QStringList &fileList)
{
    slotFilesRemoved(fileList);
}

void RemoteMenu::slotFilesRenamed(const QString &src, const QString &dest)
{
    if (KUrl(src).protocol()=="remote" || KUrl(dest).protocol()=="remote")
        reinitialize();
}

#include "remotemenu.moc"
