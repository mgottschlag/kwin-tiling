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

#include "Installer.h"
#include "Misc.h"
#include <QFile>
#include <QList>
#include <kzip.h>
#include <ktempdir.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include "JobRunner.h"

namespace KFI
{

static void getAssociatedUrls(const KUrl &url, KUrl::List &list, bool afmAndPfm, QWidget *widget)
{
    QString ext(url.path());
    int     dotPos(ext.lastIndexOf('.'));
    bool    check(false);

    if(-1==dotPos) // Hmm, no extension - check anyway...
        check=true;
    else           // Cool, got an extension - see if its a Type1 font...
    {
        ext=ext.mid(dotPos+1);
        check=0==ext.compare("pfa", Qt::CaseInsensitive) ||
              0==ext.compare("pfb", Qt::CaseInsensitive);
    }

    if(check)
    {
        const char *afm[]={"afm", "AFM", "Afm", NULL},
                   *pfm[]={"pfm", "PFM", "Pfm", NULL};
        bool       gotAfm(false),
                   localFile(url.isLocalFile());
        int        e;

        for(e=0; afm[e]; ++e)
        {
            KUrl statUrl(url);
            KIO::UDSEntry uds;

            statUrl.setPath(Misc::changeExt(url.path(), afm[e]));

            if(localFile ? Misc::fExists(statUrl.path()) : KIO::NetAccess::stat(statUrl, uds, widget))
            {
                list.append(statUrl);
                gotAfm=true;
                break;
            }
        }

        if(afmAndPfm || !gotAfm)
            for(e=0; pfm[e]; ++e)
            {
                KUrl          statUrl(url);
                KIO::UDSEntry uds;
                statUrl.setPath(Misc::changeExt(url.path(), pfm[e]));
                if(localFile ? Misc::fExists(statUrl.path()) : KIO::NetAccess::stat(statUrl, uds, widget))
                {
                    list.append(statUrl);
                    break;
                }
            }
    }
}

int CInstaller::install(const QStringList &fonts)
{
    QStringList::ConstIterator it(fonts.begin()),
                               end(fonts.end());
    bool                       sysInstall(false);

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

    QSet<KUrl> urls;

    for(; it!=end; ++it)
        if(Misc::isPackage(*it))
        {
            KZip zip(*it);

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
                            itsTempDir=new KTempDir;
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
                                urls.insert(KUrl(itsTempDir->name()+name));
                            }
                        }
                    }
                }
            }
        }
        else
        {
            KUrl::List associatedUrls;

            getAssociatedUrls(*it, associatedUrls, false, itsParent);
            urls.insert(KUrl(*it));

            KUrl::List::Iterator aIt(associatedUrls.begin()),
                                 aEnd(associatedUrls.end());

            for(; aIt!=aEnd; ++aIt)
                urls.insert(*aIt);
        }

    if(urls.count())
    {
        CJobRunner::ItemList      list;
        QSet<KUrl>::ConstIterator it(urls.begin()),
                                  end(urls.end());

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
