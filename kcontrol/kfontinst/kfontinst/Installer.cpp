/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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

#include "Installer.h"
#include "Misc.h"
#include <QFile>
#include <QList>
#include <kzip.h>
#include <ktempdir.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kio/netaccess.h>
#include "JobRunner.h"

namespace KFI
{

int CInstaller::install(const QSet<KUrl> &urls)
{
    QSet<KUrl>::ConstIterator it(urls.begin()),
                              end(urls.end());
    bool                      sysInstall(false);
    CJobRunner *jobRunner=new CJobRunner(itsParent);

    if(!Misc::root())
    {
        switch(KMessageBox::questionYesNoCancel(itsParent,
                                       i18n("Do you wish to install the font(s) for personal use "
                                            "(only usable by you), or "
                                            "system-wide (usable by all users)?"),
                                       i18n("Where to Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                       KGuiItem(i18n(KFI_KIO_FONTS_SYS))))
        {
            case KMessageBox::No:
                if(!jobRunner->getAdminPasswd(itsParent))
                    return -1;
                break;
            case KMessageBox::Cancel:
                return -1;
            default:
                break;
        }
    }

    QSet<KUrl> instUrls;

    for(; it!=end; ++it)
    {
        KUrl local(KIO::NetAccess::mostLocalUrl(*it, NULL));
        bool package(false);

        if(local.isLocalFile())
        {
            QString localFile(local.path());

            if(Misc::isPackage(localFile))
            {
                KZip zip(localFile);

                package=true;
                if(zip.open(QIODevice::ReadOnly))
                {
                    const KArchiveDirectory *zipDir=zip.directory();

                    if(zipDir)
                    {
                        QStringList fonts(zipDir->entries());

                        if(fonts.count())
                        {
                            QStringList::ConstIterator it(fonts.begin()),
                                                       end(fonts.end());

                            if(!itsTempDir)
                            {
                                itsTempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                                itsTempDir->setAutoRemove(true);
                            }

                            for(; it!=end; ++it)
                            {
                                const KArchiveEntry *entry=zipDir->entry(*it);

                                if(entry && entry->isFile())
                                {
                                    ((KArchiveFile *)entry)->copyTo(itsTempDir->name());

                                    QString name(entry->name());

                                    //
                                    // Cant install hidden fonts, therefore need to unhide 1st!
                                    if(Misc::isHidden(name))
                                    {
                                        ::rename(QFile::encodeName(itsTempDir->name()+
                                                                name).data(),
                                            QFile::encodeName(itsTempDir->name()+
                                                                name.mid(1)).data());
                                        name=name.mid(1);
                                    }
                                    instUrls.insert(KUrl(itsTempDir->name()+name));
                                }
                            }
                        }
                    }
                }
            }
        }
        if(!package)
        {
            KUrl::List associatedUrls;

            CJobRunner::getAssociatedUrls(*it, associatedUrls, false, itsParent);
            instUrls.insert(*it);

            KUrl::List::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                instUrls.insert(*aIt);
        }
    }

    if(instUrls.count())
    {
        CJobRunner::ItemList      list;
        QSet<KUrl>::ConstIterator it(instUrls.begin()),
                                  end(instUrls.end());

        for(; it!=end; ++it)
            list.append(*it);

        return jobRunner->exec(CJobRunner::CMD_INSTALL, list,
                               KUrl(Misc::root()
                                        ? KFI_KIO_FONTS_PROTOCOL":/"
                                        : sysInstall
                                            ? KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_SYS"/"
                                            : KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_USER"/"));
    }
    else
        return -1;
}

CInstaller::~CInstaller()
{
    delete itsTempDir;
}

}

#include "Installer.moc"
