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
#include <kio/job.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kapplication.h>

#include "UpdateDialog.h"

#define KFI_ICON    "fonts"
#define KFI_CAPTION I18N_NOOP("Font Installer")

namespace KFI
{

static QString removeQuotes(const QString &item)
{
    unsigned int len=item.length();

    return (item[0]==QChar('\'') || item[0]==QChar('\"')) &&
           (item[len-1]==QChar('\'') || item[len-1]==QChar('\"'))
        ? item.mid(1, len-2)
        : item;
}

//
// Tell the io-slave not to clear, and re-read, the list of fonts. And also, tell it to not
// automatically recreate the config files - we want that to happen after all fonts are installed,
// deleted, etc.
static void setMetaData(KIO::Job *job)
{
    job->addMetaData(KFI_KIO_TIMEOUT, "0");
    job->addMetaData(KFI_KIO_NO_CLEAR, "1");
}

CInstaller::EReturn CInstaller::install(const QStringList &fonts)
{
    QStringList::ConstIterator it(fonts.begin()),
                               end(fonts.end());

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
            {
                KProcess proc;

                proc << "kdesu" << "-d" << "--icon" << KFI_ICON
                     << "--caption" << KFI_CAPTION
                     << "--noignorebutton"
                     << itsApp
                     << QString().sprintf("0x%x", itsXid);

                for(; it!=end; ++it)
                    proc << removeQuotes(*it);

                proc.start(KProcess::Block);
                return ROOT_INSTALL;
            }
            case KMessageBox::Cancel:
                return USER_CANCELLED;
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
            urls.insert(KUrl(*it));

    if(urls.count())
    {
        KUrl::List                list;
        QSet<KUrl>                copy;
        QSet<KUrl>::ConstIterator it(urls.begin()),
                                  end(urls.end());

        for(; it!=end; ++it)
            list.append(*it);

        KIO::Job *job=KIO::copy(list, KUrl(Misc::root()
                                          ? KFI_KIO_FONTS_PROTOCOL":/"
                                            : KFI_KIO_FONTS_PROTOCOL":/"KFI_KIO_FONTS_USER"/"),
                                true);
        setMetaData(job);
        connect(job, SIGNAL(result(KJob *)), this, SLOT(fontsInstalled(KJob *)));
        return INSTALLING;
    }
    else
    {
        error();
        return NO_FONTS;
    }
}

CInstaller::~CInstaller()
{
    delete itsTempDir;
    delete itsUpdateDialog;
}

void CInstaller::error()
{
    KMessageBox::error(itsParent, i18n("Font installation failed."));
    qApp->exit(-8);
}

void CInstaller::fontsInstalled(KJob *job)
{
    if(job && 0==job->error())
    {
        QByteArray  packedArgs;
        QDataStream stream(&packedArgs, QIODevice::WriteOnly);

        stream << KFI::SPECIAL_CONFIGURE << 1;

        KIO::Job *job=KIO::special(KUrl(KFI_KIO_FONTS_PROTOCOL":/"), packedArgs, false);
        connect(job, SIGNAL(result(KJob *)), this, SLOT(systemConfigured(KJob *)));

        delete itsTempDir;
        itsTempDir=NULL;

        itsUpdateDialog=new CUpdateDialog(itsParent, itsXid);
        itsUpdateDialog->start();
    }
    else
        error();
}

void CInstaller::systemConfigured(KJob *job)
{
    if(itsUpdateDialog)
        itsUpdateDialog->stop();

    if(job && 0==job->error())
    {
        KMessageBox::information(itsParent, i18n("<p><b>Installation Complete</b><p>"
                                                 "<p>Please note that any open applications will "
                                                 "need to be restarted in order to see the new "
                                                 "font(s).</p>"));
        qApp->exit(0);
    }
    else
        error();
}

}

#include "Installer.moc"
