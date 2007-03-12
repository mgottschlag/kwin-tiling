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
#include <klocale.h>
#include <QGridLayout>
#include <QBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QValidator>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QTimer>
#include <QApplication>
#include <QGroupBox>
#include <kio/netaccess.h>
#include <kio/metainfojob.h>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kprinter.h>
#include <ktoolbarlabelaction.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <kprocess.h>
#include <kmimetype.h>
#include <fontconfig/fontconfig.h>

// Enable the following to allow printing of non-installed fonts. Doesnt seem to work :-(
//#define KFI_PRINT_APP_FONTS

namespace KFI
{

CFontViewPart::CFontViewPart(QWidget *parent)
             : itsConfig(KGlobal::config()),
               itsProc(NULL)
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
    connect(itsFaceSelector, SIGNAL(valueChanged(int)), SLOT(showFace(int)));

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
    emit enablePrintAction(false);
}

CFontViewPart::~CFontViewPart()
{
    delete itsProc;
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
    // NOTE: Cant do the real open here, as dont seem to be able to use KIO::NetAccess functions
    // during initial start-up. Bug report 111535 indicates that calling "konqueror <font>" crashes.
    itsInstallButton->setEnabled(false);
    QTimer::singleShot(0, this, SLOT(timeout()));
    return true;
}

void CFontViewPart::timeout()
{
    bool          isFonts=KFI_KIO_FONTS_PROTOCOL==url().protocol(),
                  isDisabled(false),
                  showFs=false;
    KUrl          displayUrl(url());
    QString       name;
    unsigned long styleInfo=KFI_NO_STYLE_INFO;

    if(isFonts)
    {
        KIO::UDSEntry udsEntry;

        FcInitReinitialize();

        if(KIO::NetAccess::stat(url(), udsEntry, NULL))
        {
            name=udsEntry.stringValue(KIO::UDS_NAME);
            styleInfo=FC::styleValFromStr(udsEntry.stringValue(UDS_EXTRA_FC_STYLE));
            isDisabled=udsEntry.numberValue(KIO::UDS_HIDDEN, 0) ? true : false;
        }
        if(!name.isEmpty())
            displayUrl.setFileName(name);
    }

    itsInstallButton->setEnabled(!isFonts && !isInstalled());
    emit setWindowCaption(Misc::prettyUrl(displayUrl));

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
    getMetaInfo();
}

void CFontViewPart::previewStatus(bool st)
{
    bool printable(false);

    if(st)
        if(KFI_KIO_FONTS_PROTOCOL==url().protocol())
            printable=!Misc::isHidden(url());
#ifdef KFI_PRINT_APP_FONTS
        {
            KMimeType::Ptr mime=KMimeType::findByUrl(KUrl::fromPath(m_file), 0, false, true);

            printable=mime->is("application/x-font-ttf") ||
                      mime->is("application/x-font-otf") ||
                      mime->is("application/x-font-ttc");
        }
#endif

    itsChangeTextAction->setEnabled(st);
    itsExtension->enablePrint(st && printable);
    emit enablePrintAction(st && printable);
}

void CFontViewPart::install()
{
    if(!itsProc || !itsProc->isRunning())
    {
        if(!itsProc)
            itsProc=new KProcess;
        else
            itsProc->clearArguments();

        *itsProc << KFI_APP
                 << "-i"
                 << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
                 << KGlobal::caption().toUtf8()
                 << url().prettyUrl();
        itsProc->start(KProcess::NotifyOnExit);
        connect(itsProc, SIGNAL(processExited(KProcess *)), SLOT(installlStatus(KProcess *)));
        itsInstallButton->setEnabled(false);
    }
}

void CFontViewPart::installlStatus(KProcess *)
{
    itsInstallButton->setEnabled(!isInstalled());
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
    if(!itsProc || !itsProc->isRunning())
    {
        if(!itsProc)
            itsProc=new KProcess;
        else
            itsProc->clearArguments();

        if(KFI_KIO_FONTS_PROTOCOL==url().protocol())
        {
            Misc::TFont info;

            CFcEngine::instance()->getInfo(url(), 0, info);

            *itsProc << KFI_APP
                     << "-P"
                     << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
                     << KGlobal::caption().toUtf8()
                     << "0"
                     << info.family.toUtf8()
                     << QString().setNum(info.styleInfo);
        }
#ifdef KFI_PRINT_APP_FONTS
        else
            *itsProc << KFI_APP
                     << "-P"
                     << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
                     << KGlobal::caption().toUtf8()
                     << "0"
                     << m_file
                     << QString().setNum(KFI_NO_STYLE_INFO);
#endif

        if(itsProc)
            itsProc->start(KProcess::DontCare);
    }
}

void CFontViewPart::displayType(const QList<CFcEngine::TRange> &range)
{
    itsPreview->setUnicodeRange(range);
    itsChangeTextAction->setEnabled(0==range.count());
}

void CFontViewPart::showFace(int f)
{
    itsPreview->showFace(f);
    itsMetaLabel->setText(itsMetaInfo[itsFaceSelector->isVisible() && itsFaceSelector->value()>0
                                          ? itsFaceSelector->value()-1 : 0]);
}

void CFontViewPart::getMetaInfo()
{
    KFileMetaInfo meta(m_url);

    if(meta.isValid())
    {
        QStringList           keys(meta.preferredKeys());
        QStringList::Iterator it(keys.begin()),
                              end(keys.end());

        //
        // Decode meta info. In the case of TTC fonts, kfile_font will separate each face's
        // details with "; ". However, version and foundry are listed for only the 1st face...
        for(; it!=end; ++it)
        {
            KFileMetaInfoItem          mi(meta.item(*it));
            QString                    tk(mi.name());
            QStringList                list(mi.value().toString().split("; "));
            QStringList::ConstIterator sit(list.begin()),
                                       send(list.end());

            for(int i=0; sit!=send; ++sit, ++i)
                itsMetaInfo[i]+="<tr><td><b>"+tk+"</b></td></tr><tr><td>"+
                                (*sit)+"</td></tr>";

            if(itsMetaInfo.count()>1 && 1==list.count())
                for(int i=1; i<itsMetaInfo.count(); ++i)
                    itsMetaInfo[i]+="<tr><td><b>"+tk+"</b></td></tr><tr><td>"+
                                    list.first()+"</td></tr>";
        }

        for(int i=0; i<itsMetaInfo.count(); ++i)
            itsMetaInfo[i]="<table>"+itsMetaInfo[i]+"</table>";
        itsMetaLabel->setText(itsMetaInfo[itsFaceSelector->isVisible() && itsFaceSelector->value()>0
                                            ? itsFaceSelector->value()-1 : 0]);
    }

    if(0==itsMetaInfo.size())
        itsMetaLabel->setText(i18n("<p>No information</p>"));
}

bool CFontViewPart::isInstalled()
{
    bool installed=false;

    if(KFI_KIO_FONTS_PROTOCOL==url().protocol())
        installed=true;
    else
    {
        KUrl destUrl;

        if(Misc::root())
        {
            destUrl=QString(KFI_KIO_FONTS_PROTOCOL":/")+CFcEngine::instance()->getName(url());
            installed=KIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget());
        }
        else
        {
            destUrl=QString(KFI_KIO_FONTS_PROTOCOL":/")+i18n(KFI_KIO_FONTS_SYS)+QChar('/')+
                CFcEngine::instance()->getName(url());
            if(KIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget()))
                installed=true;
            else
            {
                destUrl=QString(KFI_KIO_FONTS_PROTOCOL":/")+i18n(KFI_KIO_FONTS_USER)+QChar('/')+
                    CFcEngine::instance()->getName(url());
                installed=KIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget());
            }
        }
    }

    return installed;
}

BrowserExtension::BrowserExtension(CFontViewPart *parent)
                : KParts::BrowserExtension(parent)
{
    setURLDropHandlingEnabled(true);
}

void BrowserExtension::enablePrint(bool enable)
{
    emit enableAction("print", enable);
}

void BrowserExtension::print()
{
    static_cast<CFontViewPart*>(parent())->print();
}

}

#include "FontViewPart.moc"
