/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "FontLister.h"
#include "Misc.h"
#include "KfiConstants.h"
#include <kdirnotify.h>

namespace KFI
{

CFontLister::CFontLister(QObject *parent)
           : QObject(parent),
             itsAutoUpdate(true),
             itsUpdateRequired(false),
             itsJob(NULL),
             itsJobSize(0)
{
  org::kde::KDirNotify *kdirnotify = new org::kde::KDirNotify(QString(), QString(),
                                                              QDBusConnection::sessionBus(), this);
  connect(kdirnotify, SIGNAL(FileRenamed(QString,QString)), SLOT(fileRenamed(QString,QString)));
  connect(kdirnotify, SIGNAL(FilesAdded(QString)), SLOT(filesAdded(QString)));
  connect(kdirnotify, SIGNAL(FilesRemoved(QStringList)), SLOT(filesRemoved(QStringList)));
}

void CFontLister::scan(const KUrl &url)
{
    if(!busy())
    {
        if(Misc::root())
            itsJob=KIO::listDir(KUrl(KFI_KIO_FONTS_PROTOCOL":/"), false);
        else if(url.isEmpty())
            itsJob=KIO::listDir(KUrl(KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_ALL), false);
        else
            itsJob=KIO::listDir(url, false);

        emit message(i18n("Scanning font list..."));
        connect(itsJob, SIGNAL(entries(KIO::Job *, const KIO::UDSEntryList &)), this,
                SLOT(entries(KIO::Job *, const KIO::UDSEntryList &)));
        connect(itsJob, SIGNAL(infoMessage(KJob *, const QString&, const QString& )),
                SLOT(infoMessage(KJob *, const QString &)));
        connect(itsJob, SIGNAL(result(KJob *)), this, SLOT(result(KJob *)));
        //connect(itsJob, SIGNAL(percent(KJob *, unsigned long)), this,
        //        SLOT(percent(KJob *, unsigned long)));
        connect(itsJob, SIGNAL(totalSize(KJob *, qulonglong)), this,
                SLOT(totalSize(KJob *, qulonglong)));
        connect(itsJob, SIGNAL(processedSize(KJob *, qulonglong)), this,
                SLOT(processedSize(KJob *, qulonglong)));
    }
}

void CFontLister::setAutoUpdate(bool on)
{
    if(itsAutoUpdate!=on)
    {
        itsAutoUpdate=on;
        if(on)
            if(itsUpdateRequired)
            {
                itsUpdateRequired=false;
                scan();
            }
    }
}

void CFontLister::fileRenamed(const QString &from, const QString &to)
{
    KUrl          fromU(from);
    KFileItemList refresh;

    if(KFI_KIO_FONTS_PROTOCOL==fromU.protocol())
    {
        QMap<KUrl, KFileItem *>::Iterator it(itsItems.find(fromU));

        if(it!=itsItems.end())
        {
            KFileItem *item(*it);
            KUrl      toU(to);

            item->setUrl(toU);
            itsItems.remove(it);
            if(itsItems.contains(toU))
            {
                emit deleteItem(item);
                delete item;
            }
            else
            {
                itsItems[toU]=item;
                refresh.append(item);
            }
        }
    }

    if(refresh.count())
        emit refreshItems(refresh);
}

void CFontLister::filesAdded(const QString &dir)
{
    KUrl url(dir);

    if(KFI_KIO_FONTS_PROTOCOL==url.protocol())
        if(itsAutoUpdate)
            scan(url);
        else
            itsUpdateRequired=true;
}

void CFontLister::filesRemoved(const QStringList &files)
{
    QStringList::ConstIterator it(files.begin()),
                               end(files.end());

    for(; it!=end; ++it)
    {
        KUrl url(*it);

        if(KFI_KIO_FONTS_PROTOCOL==url.protocol())
        {
            QMap<KUrl, KFileItem *>::Iterator it(itsItems.find(url));

            if(it!=itsItems.end())
            {
                KFileItem *item(*it);
                emit deleteItem(item);
                printf("FILE DELETED:%s\n", (*it)->url().prettyUrl().toLatin1().constData());
                delete item;
                itsItems.remove(it);
            }
        }
    }
}

void CFontLister::result(KJob *job)
{
    itsJob=NULL;

    if(job && !job->error())
    {
        QMap<KUrl, KFileItem *>::Iterator it(itsItems.begin());

        while(it!=itsItems.end())
            if((*it)->isMarked())
            {
                (*it)->unmark();
                ++it;
            }
            else
            {
                QMap<KUrl, KFileItem *>::Iterator remove(it);
                KFileItem                         *item(*it);

                emit deleteItem(item);
                ++it;
                delete item;
                itsItems.remove(remove);
            }
    }
    else
    {
        QMap<KUrl, KFileItem *>::Iterator it(itsItems.begin()),
                                          end(itsItems.end());

        for(; it!=end; ++it)
            (*it)->unmark();
    }

    emit completed();
}

void CFontLister::entries(KIO::Job *, const KIO::UDSEntryList &entries)
{
    KIO::UDSEntryList::ConstIterator it(entries.begin()),
                                     end(entries.end());
    KFileItemList                    newFonts;

    for(; it!=end; ++it)
    {
        const QString name((*it).stringValue(KIO::UDS_NAME));

        if(!name.isEmpty() && name!="." && name!="..")
        {
            KUrl url((*it).stringValue(KIO::UDS_URL));

            if(!itsItems.contains(url))
            {
                KFileItem *item(new KFileItem(*it, url));

                itsItems[url]=item;
                newFonts.append(item);
            }
            itsItems[url]->mark();
        }
    }

    if(newFonts.count())
        emit newItems(newFonts);
}

/*
void CFontLister::percent(KJob *job, unsigned long p)
{
    emit percent(p);
}
*/

void CFontLister::processedSize(KJob *, qulonglong s)
{
    emit percent(itsJobSize>0 ? (s*100)/itsJobSize : 100);
}

void CFontLister::totalSize(KJob *, qulonglong s)
{
    itsJobSize=s;
}

void CFontLister::infoMessage(KJob *, const QString &msg)
{
    emit message(msg);
}

}

#include "FontLister.moc"
