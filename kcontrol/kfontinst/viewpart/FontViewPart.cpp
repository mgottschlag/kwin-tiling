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

#include "FontViewPart.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "FcEngine.h"
#include <klocale.h>
#include <QBoxLayout>
#include <QGridLayout>
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
#include <kio/netaccess.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandardaction.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kprinter.h>
#include <ktoolbarlabelaction.h>
#include <kactioncollection.h>
#include <kselectaction.h>
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

    QFrame *previewFrame=new QFrame(itsFrame);

    itsToolsFrame=new QFrame(itsFrame);

    itsLayout=new QBoxLayout(QBoxLayout::TopToBottom, itsFrame);

    itsLayout->setMargin(KDialog::marginHint());
    itsLayout->setSpacing(KDialog::spacingHint());

    QBoxLayout *previewLayout=new QBoxLayout(QBoxLayout::LeftToRight, previewFrame),
               *toolsLayout=new QBoxLayout(QBoxLayout::LeftToRight, itsToolsFrame);

    previewLayout->setMargin(0);
    previewLayout->setSpacing(0);
    toolsLayout->setMargin(0);
    toolsLayout->setSpacing(KDialog::spacingHint());

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(Qt::ClickFocus);
    itsToolsFrame->setFrameShape(QFrame::NoFrame);
    previewFrame->setFrameShape(QFrame::StyledPanel);
    previewFrame->setFrameShadow(QFrame::Sunken);
    setInstance(new KInstance(KFI_NAME));

    itsPreview=new CFontPreview(previewFrame);
    itsPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFaceLabel=new QLabel(i18n("Face:"), itsToolsFrame);
    itsFaceSelector=new KIntNumInput(1, itsToolsFrame);
    itsInstallButton=new QPushButton(i18n("Install..."), itsToolsFrame);
    itsInstallButton->setEnabled(false);
    previewLayout->addWidget(itsPreview);
    itsLayout->addWidget(previewFrame);
    itsLayout->addWidget(itsToolsFrame);
    toolsLayout->addWidget(itsFaceLabel);
    toolsLayout->addWidget(itsFaceSelector);
    itsFaceLabel->hide();
    itsFaceSelector->hide();
    toolsLayout->addItem(new QSpacerItem(5, 5, QSizePolicy::MinimumExpanding,
                                         QSizePolicy::Minimum));
    toolsLayout->addWidget(itsInstallButton);
    connect(itsPreview, SIGNAL(status(bool)), SLOT(previewStatus(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), SLOT(install()));
    connect(itsFaceSelector, SIGNAL(valueChanged(int)), itsPreview, SLOT(showFace(int)));

    itsChangeTextAction=actionCollection()->addAction("changeText");
    itsChangeTextAction->setIcon(KIcon("text"));
    itsChangeTextAction->setText(i18n("Change Text..."));
    connect(itsChangeTextAction, SIGNAL(triggered(bool)), SLOT(changeText()));

    KToolBarLabelAction *toolbarLabelAction = new KToolBarLabelAction(i18n("Display:"), this);
    actionCollection()->addAction("displayLabel", toolbarLabelAction);
    itsDisplayTypeAction=new KSelectAction(KIcon("charset"), i18n("Standard preview"), this);
    actionCollection()->addAction("displayType", itsDisplayTypeAction);
    connect(itsDisplayTypeAction, SIGNAL(triggered(int)), SLOT(displayType()));

    QStringList items;
    items.append(i18n("Standard preview"));
    items.append(i18n("All characters"));

    for(int i=0; i<256;++i)
        items.append(i18n("Characters %1 to %2", i*256, (i*256)+255));
    itsDisplayTypeAction->setItems(items);
    itsDisplayTypeAction->setCurrentItem(0);
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

    if(KFI_KIO_FONTS_PROTOCOL==url.protocol() || KIO::NetAccess::mostLocalUrl(url, itsFrame).isLocalFile())
    {
        m_url=url;
        emit started(0);
        m_file = m_url.path();
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
    bool          isFonts=KFI_KIO_FONTS_PROTOCOL==m_url.protocol();
    KUrl          displayUrl(m_url);
    QString       name;
    unsigned long styleInfo=KFI_NO_STYLE_INFO;

    if(isFonts)
    {
        KIO::UDSEntry udsEntry;

        FcInitReinitialize();

        if(KIO::NetAccess::stat(m_url, udsEntry, NULL))
        {
            name=udsEntry.stringValue(KIO::UDS_NAME);
            styleInfo=CFcEngine::styleValFromStr(udsEntry.stringValue(UDS_EXTRA_FC_STYLE));
        }
        if(!name.isEmpty())
            displayUrl.setFileName(name);
    }

    itsInstallButton->setEnabled(!isFonts && !isInstalled());
    emit setWindowCaption(displayUrl.prettyUrl());
    doPreview(isFonts, name, styleInfo);
}

void CFontViewPart::doPreview(bool isFonts, const QString &name, unsigned long styleInfo, int index)
{
    bool showFs=false;

    itsPreview->showFont(isFonts ? m_url : KUrl::fromPath(m_file), name, styleInfo, index);

    if(!isFonts && CFcEngine::instance()->getNumIndexes()>1)
    {
        showFs=true;
        itsFaceSelector->setRange(1, CFcEngine::instance()->getNumIndexes(), 1, false);
        itsFaceSelector->blockSignals(true);
        itsFaceSelector->setValue(index);
        itsFaceSelector->blockSignals(false);
    }

    itsFaceLabel->setVisible(showFs);
    itsFaceSelector->setVisible(showFs);
}

void CFontViewPart::previewStatus(bool st)
{
    bool printable(false);

    if(st)
        if(KFI_KIO_FONTS_PROTOCOL==m_url.protocol())
            printable=!Misc::isHidden(m_url);
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
                << m_url.prettyUrl();
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

        if(KFI_KIO_FONTS_PROTOCOL==m_url.protocol())
        {
            Misc::TFont info;

            CFcEngine::instance()->getInfo(m_url, 0, info);

            *itsProc << KFI_APP
                     << "-P"
                     << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
                     << "0"
                     << info.family.toUtf8()
                     << QString().setNum(info.styleInfo);
        }
#ifdef KFI_PRINT_APP_FONTS
        else
            *itsProc << KFI_APP
                     << "-P"
                     << QString().sprintf("0x%x", (unsigned int)(itsFrame->topLevelWidget()->winId()))
                     << "0"
                     << m_file
                     << QString().setNum(KFI_NO_STYLE_INFO);
#endif

        if(itsProc)
            itsProc->start(KProcess::DontCare);
    }
}

void CFontViewPart::displayType()
{
    switch(itsDisplayTypeAction->currentItem())
    {
        case 0:
            itsPreview->setUnicodeStart(CFcEngine::STD_PREVIEW);
            break;
        case 1:
            itsPreview->setUnicodeStart(CFcEngine::ALL_CHARS);
            break;
        default:
            itsPreview->setUnicodeStart((itsDisplayTypeAction->currentItem()-2)*256);
    }

    itsPreview->showFont();
    itsChangeTextAction->setEnabled(0==itsDisplayTypeAction->currentItem());
}

bool CFontViewPart::isInstalled()
{
    bool installed=false;

    if(KFI_KIO_FONTS_PROTOCOL==m_url.protocol())
        installed=true;
    else
    {
        KUrl destUrl;

        if(Misc::root())
        {
            destUrl=QString(KFI_KIO_FONTS_PROTOCOL":/")+CFcEngine::instance()->getName(m_url);
            installed=KIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget());
        }
        else
        {
            destUrl=QString(KFI_KIO_FONTS_PROTOCOL":/")+i18n(KFI_KIO_FONTS_SYS)+QChar('/')+
                    CFcEngine::instance()->getName(m_url);
            if(KIO::NetAccess::exists(destUrl, true, itsFrame->parentWidget()))
                installed=true;
            else
            {
                destUrl=QString(KFI_KIO_FONTS_PROTOCOL":/")+i18n(KFI_KIO_FONTS_USER)+QChar('/')+
                        CFcEngine::instance()->getName(m_url);
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
