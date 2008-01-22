/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "JobRunner.h"
#include "KfiConstants.h"
#include "Misc.h"
#include <KDE/KIO/JobUiDelegate>
#include <KDE/KIO/NetAccess>
#include <KDE/KIO/CopyJob>
#include <KDE/KIO/DeleteJob>
#include <KDE/KMessageBox>
#include <KDE/KGlobal>
#include <KDE/KIconLoader>
#include <KDE/KLocale>
#include <KDE/KPasswordDialog>
#include <KDE/SuProcess>
#include <KDE/OrgKdeKDirNotifyInterface>
#include <QtGui/QGridLayout>
#include <QtGui/QProgressBar>
#include <QtGui/QLabel>
#include <QtGui/QX11Info>
#include <QtCore/QTimer>
#include <X11/Xlib.h>
#include <fixx11h.h>
#include <sys/resource.h>

using namespace KDESu;

namespace KFI
{

static KUrl toggle(const KUrl &orig, bool enable)
{
    KUrl url(orig);

    url.setFileName(enable
                 ? Misc::getFile(orig.path()).mid(1)
                 : QChar('.')+Misc::getFile(orig.path()));
    return url;
}

class CSkipDialog : public KDialog
{
    public:

    enum Result
    {
        SKIP,
        AUTO_SKIP,
        CANCEL
    };

    CSkipDialog(QWidget *parent, bool multi, const QString &errorText)
        : KDialog(parent),
          itsResult(CANCEL)
    {
        setCaption(i18n( "Information"));
        setButtons(multi ? Cancel|User1|User2 : Cancel );
        setButtonText(User1, i18n("Skip"));
        setButtonText(User2, i18n("AutoSkip"));
        setMainWidget(new QLabel(errorText, this));
        resize(sizeHint());
    }

    Result go()
    {
        itsResult=CANCEL;
        exec();
        return itsResult;
    }

    void slotButtonClicked(int button)
    {
        switch(button)
        {
            case User1:
                itsResult=SKIP;
                break;
            case User2:
                itsResult=AUTO_SKIP;
                break;
            default:
                itsResult=CANCEL;
        }

        KDialog::accept();
    }

    private:

    Result itsResult;
};

class CPasswordDialog : public KPasswordDialog
{
    public:

    CPasswordDialog(QWidget *parent)
        : KPasswordDialog(parent),
          itsSuProc(KFI_SYS_USER)
    {
        setCaption(i18n("Authorisation Required"));

        if(itsSuProc.useUsersOwnPassword())
            setPrompt(i18n("The requested action requires administrator privileges.\n"
                           "If you have these privileges, then please enter your password."));
        else
            setPrompt(i18n("The requested action requires administrator privileges.\n"
                           "Please enter the system administrator's password."));

        setPixmap(DesktopIcon("dialog-password"));
    }

    bool checkPassword()
    {
        switch (itsSuProc.checkInstall(password().toLocal8Bit()))
        {
            case -1:
                showErrorMessage(itsSuProc.useUsersOwnPassword()
                                    ? i18n("Insufficient privileges.")
                                    : i18n("Conversation with su failed."), UsernameError);
                return false;
            case 0:
                return true;
            case SuProcess::SuNotFound:
                showErrorMessage(i18n("<p>Could not launch '%1'.</p>"
                                      "<p>Make sure your PATH is set correctly.</p>",
                                      itsSuProc.useUsersOwnPassword() ? "sudo" : "su"), FatalError);
                return false;
            case SuProcess::SuNotAllowed:
                showErrorMessage(i18n("Insufficient privileges."), FatalError);
                return false;
            case SuProcess::SuIncorrectPassword:
                showErrorMessage(i18n("Incorrect password, please try again."), PasswordError);
                return false;
            default:
                showErrorMessage(i18n("Internal error: illegal return from "
                                      "SuProcess::checkInstall()"), FatalError);
                done(Rejected);
                return false;
        }
    }

    SuProcess itsSuProc;
};

CJobRunner::CJobRunner(QWidget *parent, int xid)
           : CActionDialog(parent),
             itsIt(itsUrls.end()),
             itsEnd(itsIt),
             itsAutoSkip(false),
             itsCancelClicked(false),
             itsModified(false)
{
    // Set core dump size to 0 because we will have root's password in memory.
    struct rlimit rlim;
    rlim.rlim_cur=rlim.rlim_max=0;
    setrlimit(RLIMIT_CORE, &rlim);

    setModal(true);
    setButtons(KDialog::Cancel);
    if(NULL==parent && 0!=xid)
        XSetTransientForHint(QX11Info::display(), winId(), xid);

    QFrame *page = new QFrame(this);
    setMainWidget(page);

    QGridLayout *layout=new QGridLayout(page);
    layout->setMargin(KDialog::marginHint());
    layout->setSpacing(KDialog::spacingHint());
    itsStatusLabel=new QLabel(page);
    itsProgress=new QProgressBar(page);
    layout->addWidget(itsPixmapLabel, 0, 0, 2, 1);
    layout->addWidget(itsStatusLabel, 0, 1);
    layout->addWidget(itsProgress, 1, 1);
}

CJobRunner::~CJobRunner()
{
}

bool CJobRunner::getAdminPasswd(QWidget *parent)
{
    if(!Misc::root())
    {
        // Prompt the user for a password, if we do not already have it...
        if(itsPasswd.isEmpty() || 0!=SuProcess(KFI_SYS_USER).checkInstall(itsPasswd.toLocal8Bit()))
        {
            CPasswordDialog dlg(parent);

            if(!dlg.exec())
                return false;
            itsPasswd=dlg.password().toLocal8Bit();
        }
    }

    return true;
}

void CJobRunner::getAssociatedUrls(const KUrl &url, KUrl::List &list, bool afmAndPfm, QWidget *widget)
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

int CJobRunner::exec(ECommand cmd, const ItemList &urls, const KUrl &dest)
{
    switch(cmd)
    {
        case CMD_INSTALL:
            setCaption(i18n("Installing"));
            break;
        case CMD_DELETE:
            setCaption(i18n("Uninstalling"));
            break;
        case CMD_ENABLE:
            setCaption(i18n("Enabling"));
            break;
        case CMD_COPY:
            setCaption(i18n("Copying"));
            break;
        case CMD_MOVE:
            setCaption(i18n("Moving"));
            break;
        case CMD_UPDATE:
            setCaption(i18n("Updating"));
            break;
        default:
        case CMD_DISABLE:
            setCaption(i18n("Disabling"));
    }

    itsDest=dest;
    itsUrls=urls;
    if(CMD_INSTALL==cmd)
        qSort(itsUrls.begin(), itsUrls.end());  // Sort list of fonts so that we have type1 fonts followed by their metrics...
    itsIt=itsUrls.begin();
    itsEnd=itsUrls.end();
    itsProgress->setValue(0);
    itsProgress->setRange(0, itsUrls.count()+1);
    itsProgress->show();
    itsCmd=cmd;
    itsStatusLabel->setText(QString());
    itsAutoSkip=itsCancelClicked=itsModified=false;
    QTimer::singleShot(0, this, SLOT(doNext()));
    return CActionDialog::exec();
}

void CJobRunner::doNext()
{
    if(itsIt==itsEnd || CMD_UPDATE==itsCmd)
    {
        if(itsModified || CMD_UPDATE==itsCmd)
        {
            //
            // Installation, deletion, enabling, disabling, completed - so now reconfigure...
            QByteArray  packedArgs;
            QDataStream stream(&packedArgs, QIODevice::WriteOnly);

            itsStatusLabel->setText(i18n("Updating font configuration. Please wait..."));

            stream << KFI::SPECIAL_CONFIGURE;

            if(CMD_UPDATE==itsCmd)
            {
                itsProgress->hide();
                for(; itsIt!=itsEnd; ++itsIt)
                    stream << (*itsIt);
            }
            else
                itsProgress->setValue(itsProgress->maximum());

            itsUrls.empty();
            itsIt=itsEnd=itsUrls.end();

            KIO::Job *job=KIO::special(KUrl(KFI_KIO_FONTS_PROTOCOL":/"), packedArgs, KIO::HideProgressInfo);
            setMetaData(job);
            connect(job, SIGNAL(result(KJob *)), SLOT(cfgResult(KJob *)));
            job->ui()->setWindow(this);
            job->ui()->setAutoErrorHandlingEnabled(false);
            job->ui()->setAutoWarningHandlingEnabled(false);
        }
        else
            cfgResult(0L);
    }
    else
    {
        KIO::Job *job(NULL);

        switch(itsCmd)
        {
            case CMD_COPY:
            case CMD_INSTALL:
            {
                KUrl dest(itsDest);

                dest.setFileName(Misc::getFile((*itsIt).path()));
                itsStatusLabel->setText(CMD_INSTALL==itsCmd ? i18n("Installing %1", (*itsIt).displayName())
                                                            : i18n("Copying %1", (*itsIt).displayName()) );
                job=KIO::file_copy(*itsIt, dest, -1, KIO::HideProgressInfo);
                break;
            }
            case CMD_DELETE:
                itsStatusLabel->setText(i18n("Deleting %1", (*itsIt).displayName()));
                job=KIO::file_delete(*itsIt, KIO::HideProgressInfo);
                break;
            case CMD_ENABLE:
                itsStatusLabel->setText(i18n("Enabling %1", (*itsIt).displayName()));
                job=KIO::rename(*itsIt, toggle(*itsIt, true), KIO::HideProgressInfo);
                break;
            case CMD_DISABLE:
                itsStatusLabel->setText(i18n("Disabling %1", (*itsIt).displayName()));
                job=KIO::rename(*itsIt, toggle(*itsIt, false), KIO::HideProgressInfo);
                break;
            case CMD_MOVE:
            {
                KUrl dest(itsDest);

                dest.setFileName(Misc::getFile((*itsIt).path()));
                itsStatusLabel->setText(i18n("Moving %1", (*itsIt).displayName()));
                job=KIO::file_move(*itsIt, dest, -1, KIO::HideProgressInfo);
                break;
            }
            default:
                break;
        }
        itsProgress->setValue(itsProgress->value()+1);
        job->setUiDelegate(0L);  // Remove the ui-delegate, so that we can handle all error messages...
        setMetaData(job);
        connect(job, SIGNAL(result(KJob *)), SLOT(jobResult(KJob *)));
    }
}

void CJobRunner::jobResult(KJob *job)
{
    Q_ASSERT(job);

    if(itsCancelClicked)
    {
        stopAnimation();
        if(KMessageBox::Yes==KMessageBox::warningYesNo(this, i18n("Are you sure you wish to cancel?")))
            itsIt=itsEnd;
        itsCancelClicked=false;
        startAnimation();
    }

    // itsIt will equal itsEnd if user decided to cancel the current op
    if(itsIt==itsEnd)
        doNext();
    else if (!job->error())
    {
        itsModified=true;
        ++itsIt;
        doNext();
    }
    else
    {
        bool cont(itsAutoSkip && itsUrls.count()>1);

        if(!cont)
        {
            stopAnimation();

            ItemList::ConstIterator next(itsIt==itsEnd ? itsEnd : itsIt+1);

            if(1==itsUrls.count() || next==itsEnd)
            {
                if(!itsAutoSkip && !job->errorString().isEmpty())
                    KMessageBox::error(this, job->errorString());
            }
            else
            {
                CSkipDialog dlg(this, true, job->errorString());

                switch(dlg.go())
                {
                    case CSkipDialog::SKIP:
                        cont=true;
                        break;
                    case CSkipDialog::AUTO_SKIP:
                        cont=itsAutoSkip=true;
                        break;
                    case CSkipDialog::CANCEL:
                        break;
                }
            }
        }

        startAnimation();
        if(cont)
        {
            if(CMD_INSTALL==itsCmd && Item::TYPE1_FONT==(*itsIt).type) // Did we error on a pfa/pfb? if so, exclude the afm/pfm...
            {
                QString oldFile((*itsIt).fileName);
                ++itsIt;

                // Skip afm/pfm
                if(itsIt!=itsEnd && (*itsIt).fileName==oldFile && Item::TYPE1_METRICS==(*itsIt).type)
                    ++itsIt;
                // Skip pfm/afm
                if(itsIt!=itsEnd && (*itsIt).fileName==oldFile && Item::TYPE1_METRICS==(*itsIt).type)
                    ++itsIt;
            }
            else
                ++itsIt;
        }
        else
        {
            itsUrls.empty();
            itsIt=itsEnd=itsUrls.end();
        }
        doNext();
    }
}

void CJobRunner::cfgResult(KJob *job)
{
    stopAnimation();

    // KIO::file_xxxx() dont seem to emit kdirnotify signals, so do this now...
    if(itsModified && (CMD_COPY==itsCmd || CMD_INSTALL==itsCmd))
        org::kde::KDirNotify::emitFilesAdded(itsDest.url());

    if(job && 0==job->error())
    {
        hide();
        KMessageBox::information(parentWidget(),
                                 i18n("<p>Please note that any open applications will need to be "
                                      "restarted in order for any changes to be noticed.</p>"),
                                 i18n("Success"), "FontChangesAndOpenApps");
        accept();
    }
    else
        reject();
}

void CJobRunner::slotButtonClicked(int)
{
    if(itsIt!=itsEnd)
        itsCancelClicked=true;
}

//
// Tell the io-slave not to clear, and re-read, the list of fonts. And also, tell it to not
// automatically recreate the config files - we want that to happen after all fonts are installed,
// deleted, etc.
void CJobRunner::setMetaData(KIO::Job *job)
{
    job->addMetaData(KFI_KIO_TIMEOUT, "0");
    job->addMetaData(KFI_KIO_NO_CLEAR, "1");

    if(!Misc::root() && !itsPasswd.isEmpty())
        job->addMetaData(KFI_KIO_PASS, itsPasswd);
}

CJobRunner::Item::Item(const KUrl &u, const QString &n)
                : KUrl(u), name(n), fileName(Misc::getFile(u.path()))
{
    type=Misc::checkExt(fileName, "pfa") || Misc::checkExt(fileName, "pfb")
            ? TYPE1_FONT
            : Misc::checkExt(fileName, "afm") || Misc::checkExt(fileName, "pfm")
                ? TYPE1_METRICS
                : OTHER_FONT;

    if(OTHER_FONT!=type)
    {
        int pos(fileName.lastIndexOf('.'));

        if(-1!=pos)
            fileName=fileName.left(pos);
    }
}

bool CJobRunner::Item::operator<(const Item &o) const
{
    // Dont care about the order of non type1 fonts/metrics...
    if(OTHER_FONT==type)
        return true;

    int nameComp(fileName.compare(o.fileName));

    return nameComp<0 || (0==nameComp && type<o.type);
}

}

#include "JobRunner.moc"
