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

#include "FontViewPart.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "FcEngine.h"
#include "PreviewSelectAction.h"
#include <QGridLayout>
#include <QBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QFile>
#include <QLabel>
#include <QValidator>
#include <QRegExp>
#include <QStringList>
#include <QTimer>
#include <QApplication>
#include <QGroupBox>
#include <QProcess>
#include <klocale.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <ktoolbarlabelaction.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <kmimetype.h>
#include <kfilemetainfo.h>
#include <kzip.h>
#include <ktempdir.h>
#include <kstandarddirs.h>
#include <fontconfig/fontconfig.h>

// Enable the following to allow printing of non-installed fonts. Doesnt seem to work :-(
//#define KFI_PRINT_APP_FONTS

namespace KFI
{

CFontViewPart::CFontViewPart(QWidget *parent)
             : itsConfig(KGlobal::config()),
               itsProc(NULL),
               itsTempDir(NULL)
{
    CFcEngine::instance()->readConfig(*itsConfig);
    CFcEngine::setBgndCol(QApplication::palette().color(QPalette::Active, QPalette::Base));
    CFcEngine::setTextCol(QApplication::palette().color(QPalette::Active, QPalette::Text));

    // create browser extension (for printing when embedded into browser)
    itsExtension = new BrowserExtension(this);

    itsFrame=new QFrame(parent);

    QFrame    *previewFrame=new QFrame(itsFrame);
    QGroupBox *metaBox=new QGroupBox(i18n("Information:"), itsFrame);

    itsFaceWidget=new QWidget(itsFrame);

    QGridLayout *mainLayout=new QGridLayout(itsFrame);

    mainLayout->setMargin(KDialog::marginHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    QBoxLayout *previewLayout=new QBoxLayout(QBoxLayout::LeftToRight, previewFrame),
               *faceLayout=new QBoxLayout(QBoxLayout::LeftToRight, itsFaceWidget);
    QBoxLayout *metaLayout=new QBoxLayout(QBoxLayout::LeftToRight, metaBox);

    itsMetaLabel=new QLabel(metaBox);
    itsMetaLabel->setAlignment(Qt::AlignTop);
    metaLayout->addWidget(itsMetaLabel);
    previewLayout->setMargin(0);
    previewLayout->setSpacing(0);
    faceLayout->setMargin(0);
    faceLayout->setSpacing(KDialog::spacingHint());

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(Qt::ClickFocus);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setFrameShadow(QFrame::Sunken);
    setComponentData(KComponentData(KFI_NAME));

    itsPreview=new CFontPreview(previewFrame);
    itsPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFaceLabel=new QLabel(i18n("Show Face:"), itsFaceWidget);
    itsFaceSelector=new KIntNumInput(1, itsFaceWidget);
    itsInstallButton=new QPushButton(i18n("Install..."), itsFrame);
    itsInstallButton->setEnabled(false);
    previewLayout->addWidget(itsPreview);
    faceLayout->addWidget(itsFaceLabel);
    faceLayout->addWidget(itsFaceSelector);
    faceLayout->addItem(new QSpacerItem(KDialog::spacingHint(), 0, QSizePolicy::Fixed, QSizePolicy::Fixed));
    itsFaceWidget->hide();

    mainLayout->addWidget(previewFrame, 0, 0, 4, 1);
    mainLayout->addWidget(metaBox, 0, 1);
    mainLayout->addWidget(itsFaceWidget, 1, 1);
    mainLayout->addItem(new QSpacerItem(KDialog::spacingHint(), KDialog::spacingHint(),
                                        QSizePolicy::Fixed, QSizePolicy::MinimumExpanding), 2, 1);
    mainLayout->addWidget(itsInstallButton, 3, 1);
    connect(itsPreview, SIGNAL(status(bool)), SLOT(previewStatus(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), SLOT(install()));
    connect(itsFaceSelector, SIGNAL(valueChanged(int)), itsPreview, SLOT(showFace(int)));

    itsChangeTextAction=actionCollection()->addAction("changeText");
    itsChangeTextAction->setIcon(KIcon("text"));
    itsChangeTextAction->setText(i18n("Change Text..."));
    connect(itsChangeTextAction, SIGNAL(triggered(bool)), SLOT(changeText()));

    KToolBarLabelAction *toolbarLabelAction = new KToolBarLabelAction(i18n("Display:"), this);
    actionCollection()->addAction("displayLabel", toolbarLabelAction);
    CPreviewSelectAction *displayTypeAction=new CPreviewSelectAction(this, true);
    actionCollection()->addAction("displayType", displayTypeAction);
    connect(displayTypeAction, SIGNAL(range(const QList<CFcEngine::TRange> &)),
            SLOT(displayType(const QList<CFcEngine::TRange> &)));

    setXMLFile("kfontviewpart.rc");
    setWidget(itsFrame);
    itsExtension->enablePrint(false);
}

CFontViewPart::~CFontViewPart()
{
    delete itsTempDir;
    itsTempDir=NULL;
}

bool CFontViewPart::openUrl(const KUrl &url)
{
    if (!url.isValid() || !closeUrl())
        return false;

    itsMetaLabel->setText(QString());
    itsMetaInfo.clear();

    if(KFI_KIO_FONTS_PROTOCOL==url.protocol() || KIO::NetAccess::mostLocalUrl(url, itsFrame).isLocalFile())
    {
        setUrl(url);
        emit started(0);
        setLocalFilePath(this->url().path());
        bool ret=openFile();
        if (ret)
            emit completed();
        return ret;
    }
    else
        return ReadOnlyPart::openUrl(url);
}

bool CFontViewPart::openFile()
{
    // NOTE: Can't do the real open here, as we don't seem to be able to use KIO::NetAccess functions
    // during initial start-up. Bug report 111535 indicates that calling "konqueror <font>" crashes.
    itsInstallButton->setEnabled(false);
    QTimer::singleShot(0, this, SLOT(timeout()));
    return true;
}

void CFontViewPart::timeout()
{
    if(!itsInstallButton)
        return;

    bool          isFonts(KFI_KIO_FONTS_PROTOCOL==url().protocol()),
                  isDisabled(false),
                  showFs(false);
    KUrl          displayUrl(url()),
                  fileUrl;
    int           fileIndex(-1);
    QString       name;
    unsigned long styleInfo(KFI_NO_STYLE_INFO);

    itsMetaUrl=url();
    delete itsTempDir;
    itsTempDir=NULL;

    if(isFonts)
    {
        FcInitReinitialize();

        //
        // This is a fonts:/Url. Check to see whether we were passed any details in the query...
        QString path=url().queryItem(KFI_FILE_QUERY),
                mime=url().queryItem(KFI_MIME_QUERY);

        name=url().queryItem(KFI_NAME_QUERY);
        styleInfo=Misc::getIntQueryVal(url(), KFI_STYLE_QUERY, KFI_NO_STYLE_INFO);
        fileIndex=Misc::getIntQueryVal(url(), KFI_KIO_FACE, -1);

        if(name.isEmpty() && path.isEmpty())
        {
            // OK, no useable info in the query - stat fonts:/ to get the required info...
            KIO::UDSEntry udsEntry;

            if(KIO::NetAccess::stat(url(), udsEntry, NULL))
            {
                name=udsEntry.stringValue(KIO::UDS_NAME);
                styleInfo=udsEntry.numberValue(UDS_EXTRA_FC_STYLE);
                isDisabled=udsEntry.numberValue(KIO::UDS_HIDDEN, 0) ? true : false;
                mime=udsEntry.stringValue(KIO::UDS_MIME_TYPE);
            }
        }
        else if(!path.isEmpty())
        {
            // Its a disabled font, so we can get the file name and index from the query...
            fileUrl=KUrl::fromPath(path);
            name=url().fileName();
        }

        if(!name.isEmpty())
        {
            displayUrl.setFileName(name.replace("%20", " "));
            displayUrl.setQuery(QString());
        }

        // What query to pass to meta info?
        if(path.isEmpty())
        {
            itsMetaUrl.removeQueryItem(KFI_NAME_QUERY);
            itsMetaUrl.addQueryItem(KFI_NAME_QUERY, name);
            itsMetaUrl.removeQueryItem(KFI_STYLE_QUERY);
            if(KFI_NO_STYLE_INFO!=styleInfo)
                itsMetaUrl.addQueryItem(KFI_STYLE_QUERY, QString().setNum(styleInfo));
        }
        else
        {
            itsMetaUrl.removeQueryItem(KFI_FILE_QUERY);
            itsMetaUrl.addQueryItem(KFI_FILE_QUERY, path);
            itsMetaUrl.removeQueryItem(KFI_KIO_FACE);
            if(fileIndex>0)
                itsMetaUrl.addQueryItem(KFI_KIO_FACE, QString().setNum(fileIndex));
        }

        itsMetaUrl.removeQueryItem(KFI_MIME_QUERY);
        itsMetaUrl.addQueryItem(KFI_MIME_QUERY, mime);
    }
    else
    {
        QString path(url().path());

        // Is this a fonts/package file? If so, extract 1 scalable font...
        if(Misc::isPackage(path))
        {
            KZip zip(path);

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

                        for(; it!=end; ++it)
                        {
                            const KArchiveEntry *entry=zipDir->entry(*it);

                            if(entry && entry->isFile())
                            {
                                delete itsTempDir;
                                itsTempDir=new KTempDir(KStandardDirs::locateLocal("tmp", KFI_TMP_DIR_PREFIX));
                                itsTempDir->setAutoRemove(true);

                                ((KArchiveFile *)entry)->copyTo(itsTempDir->name());

                                QString mime(KMimeType::findByPath(itsTempDir->name()+entry->name())->name());

                                if(mime=="application/x-font-ttf" || mime=="application/x-font-otf" ||
                                   mime=="application/x-font-type1")
                                {
                                    setLocalFilePath(itsTempDir->name()+entry->name());
                                    itsMetaUrl=KUrl::fromPath(localFilePath());
                                    break;
                                }
                                else
                                    ::unlink(QFile::encodeName(itsTempDir->name()+entry->name()).data());
                            }
                        }
                    }
                }
            }
        }
    }

    itsInstallButton->setEnabled(false);
    if(!isFonts)
        stat();

    emit setWindowCaption(Misc::prettyUrl(displayUrl));

    if(isFonts && -1!=fileIndex)
        itsPreview->showFont(fileUrl, QString(), styleInfo, fileIndex);
    else
        itsPreview->showFont(isFonts ? url() : KUrl::fromPath(localFilePath()), isDisabled ? QString() : name, styleInfo);

    if(!isFonts && CFcEngine::instance()->getNumIndexes()>1)
    {
        showFs=true;
        itsFaceSelector->setRange(1, CFcEngine::instance()->getNumIndexes(), 1, false);
        itsFaceSelector->blockSignals(true);
        itsFaceSelector->setValue(1);
        itsFaceSelector->blockSignals(false);
    }

    itsFaceWidget->setVisible(showFs);
}

void CFontViewPart::previewStatus(bool st)
{
    bool printable(false);

    if(st)
        if(KFI_KIO_FONTS_PROTOCOL==url().protocol())
            printable=!Misc::isHidden(url());
#ifdef KFI_PRINT_APP_FONTS
        else
        {
            // TODO: Make this work! Plus, printing of disabled TTF/OTF's should also be possible!
            KMimeType::Ptr mime=KMimeType::findByUrl(KUrl::fromPath(localFilePath()), 0, false, true);

            printable=mime->is("application/x-font-ttf") || mime->is("application/x-font-otf");
        }
#endif

    itsChangeTextAction->setEnabled(st);
    itsExtension->enablePrint(st && printable);
    if(st)
        getMetaInfo(itsFaceSelector->isVisible() && itsFaceSelector->value()>0
                                          ? itsFaceSelector->value()-1 : 0);
    else
        KMessageBox::error(itsFrame, i18n("Could not read font."));
}

void CFontViewPart::install()
{
    if(!itsProc || QProcess::NotRunning==itsProc->state())
    {
        QStringList args;

        if(!itsProc)
            itsProc=new QProcess(this);
        else
            itsProc->kill();

        args << "--embed" <<  QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
             << "--caption" << KGlobal::caption().toUtf8()
             << "--icon" << "kfontview"
             << url().prettyUrl();

        connect(itsProc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(installlStatus()));
        itsProc->start(KFI_INSTALLER, args);
        itsInstallButton->setEnabled(false);
    }
}

void CFontViewPart::installlStatus()
{
    stat();
}

void CFontViewPart::changeText()
{
    bool             status;
    QRegExpValidator validator(QRegExp(".*"), 0L);
    QString          oldStr(CFcEngine::instance()->getPreviewString()),
                     newStr(KInputDialog::getText(i18n("Preview String"),
                                                  i18n("Please enter new string:"),
                                                  oldStr, &status, itsFrame, &validator));

    if(status && newStr!=oldStr)
    {
        CFcEngine::instance()->setPreviewString(newStr);
        CFcEngine::instance()->writeConfig(*itsConfig);
        itsPreview->showFont();
    }
}

void CFontViewPart::print()
{
    QStringList args;

    if(KFI_KIO_FONTS_PROTOCOL==url().protocol())
    {
        Misc::TFont info;

        CFcEngine::instance()->getInfo(url(), 0, info);

        args << "--embed" << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
             << "--caption" << KGlobal::caption().toUtf8()
             << "--icon" << "kfontview"
             << "--size" << "0"
             << "--pfont" << QString(info.family+','+QString().setNum(info.styleInfo));
    }
#ifdef KFI_PRINT_APP_FONTS
    else
        args << "--embed" << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
             << "--caption" << KGlobal::caption().toUtf8()
             << "--icon" << "kfontview"
             << "--size " << "0"
             << localFilePath()
             << QString().setNum(KFI_NO_STYLE_INFO);
#endif

    if(args.count())
        QProcess::startDetached(KFI_PRINTER, args);
}

void CFontViewPart::displayType(const QList<CFcEngine::TRange> &range)
{
    itsPreview->setUnicodeRange(range);
    itsChangeTextAction->setEnabled(0==range.count());
}

void CFontViewPart::statResult(KJob *job)
{
    bool exists=!job->error();

    if(!Misc::root() && !exists && !itsStatName.isEmpty())
    {
        // OK, file does not exist in fonts:/System, try fonts:/Personal
        stat(QString(KFI_KIO_FONTS_PROTOCOL":/")+i18n(KFI_KIO_FONTS_USER)+QChar('/')+itsStatName);
        itsStatName=QString();
        return;
    }

    itsInstallButton->setEnabled(!exists);
}

void CFontViewPart::getMetaInfo(int face)
{
    if(itsMetaInfo[face].isEmpty())
    {
        // Pass as much inofmration as possible to analyzer...
        if(KFI_KIO_FONTS_PROTOCOL!=itsMetaUrl.protocol())
        {
            itsMetaUrl.removeQueryItem(KFI_KIO_FACE);
            if(face>0)
                itsMetaUrl.addQueryItem(KFI_KIO_FACE, QString().setNum(face));
        }

        KFileMetaInfo meta(itsMetaUrl);

        if(meta.isValid() && meta.keys().count())
        {
            QStringList           keys(meta.keys());
            QStringList::Iterator it(keys.begin()),
                                  end(keys.end());

            itsMetaInfo[face]="<table>";
            for(; it!=end; ++it)
            {
                KFileMetaInfoItem mi(meta.item(*it));

                itsMetaInfo[face]+="<tr><td><b>"+mi.name()+"</b></td></tr><tr><td>"+
                                   mi.value().toString()+"</td></tr>";
            }

            itsMetaInfo[face]+="</table>";
            itsMetaLabel->setText(itsMetaInfo[face]);
        }
        else
            itsMetaLabel->setText(i18n("<p>No information</p>"));
    }
    else
        itsMetaLabel->setText(itsMetaInfo[face]);
}

void CFontViewPart::stat(const QString &path)
{
    KUrl statUrl;

    if(path.isEmpty())
    {
        itsStatName=CFcEngine::instance()->getName(url());
        statUrl=Misc::root() ? KUrl(QString(KFI_KIO_FONTS_PROTOCOL":/")+itsStatName)
                             : KUrl(QString(KFI_KIO_FONTS_PROTOCOL":/")+i18n(KFI_KIO_FONTS_SYS)+QChar('/')+itsStatName);
    }
    else
        statUrl=KUrl(path);

    KIO::StatJob * job = KIO::stat(statUrl, !statUrl.isLocalFile());
    job->ui()->setWindow(itsFrame->parentWidget());
    job->setSide(true);
    connect(job, SIGNAL(result (KJob *)), this, SLOT(statResult(KJob *)));
}

BrowserExtension::BrowserExtension(CFontViewPart *parent)
                : KParts::BrowserExtension(parent)
{
    setURLDropHandlingEnabled(true);
}

void BrowserExtension::enablePrint(bool enable)
{
    if(enable!=isActionEnabled("print"))
        emit enableAction("print", enable);
}

void BrowserExtension::print()
{
    static_cast<CFontViewPart*>(parent())->print();
}

}

#include "FontViewPart.moc"
