////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontViewPart
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 03/08/2002
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include "FontViewPart.h"
#include "FontPreview.h"
#include "FontEngine.h"
#include "Global.h"
#include "KfiConfig.h"
#include "Misc.h"
#include "XConfig.h"
#include <klocale.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qfile.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qregexp.h>
#include <kio/netaccess.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <string.h>

#define DEFAULT_FONT_SIZE 28
#define FONT_SIZE_STEP     4
#define MIN_FONT_SIZE      8
#define MAX_FONT_SIZE     72

static KURL getDest(const QString &fname, bool system)
{
    QString sub;

    if(CMisc::root() || system)
        switch(CGlobal::fe().getType())
        {
            case CFontEngine::TRUE_TYPE:
            case CFontEngine::OPEN_TYPE:
            case CFontEngine::TT_COLLECTION:
                if(!CGlobal::cfg().getSysTTSubDir().isEmpty())
                    sub=CGlobal::cfg().getSysTTSubDir();
                break;
            case CFontEngine::TYPE_1:
                if(!CGlobal::cfg().getSysT1SubDir().isEmpty())
                    sub=CGlobal::cfg().getSysT1SubDir();
                break;
            default:
                break;
        }

    return KURL(QString("fonts:/")+
                      (CMisc::root() ? sub
                                     : QString((system ? i18n(KIO_FONTS_SYS) : i18n(KIO_FONTS_USER)) + QChar('/')+sub)) +
                      fname);
}

CFontViewPart::CFontViewPart(QWidget *parent, const char *name)
{
    bool kcm=0==strcmp(name, "kcmfontinst");
    CGlobal::create(true, false);

    itsFrame=new QFrame(parent, "frame");

    QFrame *previewFrame=new QFrame(itsFrame);

    itsToolsFrame=new QFrame(itsFrame);

    QVBoxLayout *layout=new QVBoxLayout(itsFrame, kcm ? 0 : KDialog::marginHint(), kcm ? 0 : KDialog::spacingHint());
    QGridLayout *previewLayout=new QGridLayout(previewFrame, 1, 1, 1, 1);
    QHBoxLayout *toolsLayout=new QHBoxLayout(itsToolsFrame, 0, KDialog::spacingHint());

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(QWidget::ClickFocus);
    itsToolsFrame->setFrameShape(QFrame::NoFrame);
    previewFrame->setFrameShadow(kcm ? QFrame::Sunken : QFrame::Raised);
    previewFrame->setFrameShape(QFrame::Panel);
    setInstance(new KInstance("kfontview"));

    KConfig cfg(CGlobal::uiCfgFile());

    cfg.setGroup(KFI_PREVIEW_GROUP);

    int  previewSize=cfg.readNumEntry(KFI_PREVIEW_SIZE_KEY, DEFAULT_FONT_SIZE);
    bool wf=cfg.readBoolEntry(KFI_PREVIEW_WATERFALL_KEY, false);

    itsPreview=new CFontPreview(previewFrame, "FontViewPart::Preview",
                                previewSize<MIN_FONT_SIZE || previewSize>MAX_FONT_SIZE ? DEFAULT_FONT_SIZE : previewSize,
                                wf);
    itsPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFaceLabel=new QLabel(i18n("Face:"), itsToolsFrame);
    itsFaceSelector=new KIntNumInput(1, itsToolsFrame);
    itsInstallButton=new QPushButton(i18n("Install..."), itsToolsFrame, "button");
    itsInstallButton->hide();
    previewLayout->addWidget(itsPreview, 0, 0);
    layout->addWidget(previewFrame);
    layout->addWidget(itsToolsFrame);
    toolsLayout->addWidget(itsFaceLabel);
    toolsLayout->addWidget(itsFaceSelector);
    itsFaceLabel->hide();
    itsFaceSelector->hide();
    toolsLayout->addItem(new QSpacerItem(5, 5, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    toolsLayout->addWidget(itsInstallButton);
    itsToolsFrame->hide();
    connect(itsPreview, SIGNAL(status(bool)), SLOT(previewStatus(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), SLOT(install()));
    connect(itsFaceSelector, SIGNAL(valueChanged(int)), itsPreview, SLOT(showFace(int)));

    itsZoomInAction=KStdAction::zoomIn(this, SLOT(zoomIn()), actionCollection(), "zoomIn");
    itsZoomOutAction=KStdAction::zoomOut(this, SLOT(zoomOut()), actionCollection(), "zoomOut");
    itsChangeTextAction=new KAction(i18n("Change Text..."), "text", KShortcut(),
                                    this, SLOT(changeText()), actionCollection(), "changeText");
    itsToggleWaterfallAction=new KToggleAction(i18n("Waterfall"), "textwaterfall", KShortcut(),
                                    this, SLOT(toggleWaterfall()), actionCollection(), "toggleWaterfall");
    itsZoomInAction->setEnabled(false);
    itsZoomOutAction->setEnabled(false);
    itsChangeTextAction->setEnabled(false);
    itsToggleWaterfallAction->setEnabled(false);

    setXMLFile("kfontviewpart.rc");
    setWidget(itsFrame);
}

CFontViewPart::~CFontViewPart()
{
    CGlobal::destroy();

    KConfig cfg(CGlobal::uiCfgFile());

    cfg.setGroup(KFI_PREVIEW_GROUP);
    cfg.writeEntry(KFI_PREVIEW_SIZE_KEY, itsPreview->currentSize());
    cfg.writeEntry(KFI_PREVIEW_WATERFALL_KEY, itsPreview->waterfall());
}

bool CFontViewPart::openURL(const KURL &url)
{
    if (!url.isValid() || !closeURL())
        return false;

    m_url=url;

    if(KIO_FONTS_PROTOCOL==m_url.protocol() || m_url.isLocalFile())
    {
        emit started( 0 );
        m_file = m_url.path();
        bool ret=openFile();
        if (ret)
        {
            emit completed();
            emit setWindowCaption(m_url.prettyURL());
        }
        return ret;
    }
    else
        return ReadOnlyPart::openURL(url);
}

bool CFontViewPart::openFile()
{
    bool showFs=false;

    itsShowInstallButton=false;

    if(CGlobal::fe().openFont(m_url, CFontEngine::XLFD, true))
    {
        if(CGlobal::fe().getNumFaces()>1)
        {
            showFs=true;
            itsFaceSelector->setRange(1, CGlobal::fe().getNumFaces(), 1, false);
        }
    }

    itsFaceLabel->setShown(showFs);
    itsFaceSelector->setShown(showFs);
    itsToolsFrame->hide();

    if(KIO_FONTS_PROTOCOL!=m_url.protocol())
    {
        if(m_url.isLocalFile())
        {
            QString ds(CMisc::dirSyntax(CMisc::getDir(m_url.path())));
            itsShowInstallButton=!CGlobal::sysXcfg().inPath(ds) && (CMisc::root() || !CGlobal::userXcfg().inPath(ds));
        }
        else
            itsShowInstallButton=true;

        if(itsShowInstallButton) // OK so file wasn't in path, or its a non file:/ URL - so try to stat on the filename
        {
            QString       fname(CMisc::getFile(m_url.path()));
            KIO::UDSEntry uds;
            KURL          destUrl(getDest(fname, true));

            if(KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
                itsShowInstallButton=false;
            else if(!CMisc::root())
            {
                destUrl=getDest(fname, false);
                if(KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
                    itsShowInstallButton=false;
            }
        }
    }

    itsPreview->showFont(m_url);

    return true;
}

void CFontViewPart::previewStatus(bool st)
{
    itsInstallButton->setShown(st && itsShowInstallButton);
    itsToolsFrame->setShown(itsInstallButton->isShown()||itsFaceSelector->isShown());

// CPD: Need to handle bitmap only TTFs!!!
    itsToggleWaterfallAction->setChecked(itsPreview->waterfall());
    itsZoomInAction->setEnabled(!itsPreview->waterfall() && st && CGlobal::fe().isScaleable() &&
                                itsPreview->currentSize()<MAX_FONT_SIZE);
    itsZoomOutAction->setEnabled(!itsPreview->waterfall() && st && CGlobal::fe().isScaleable() && 
                                 itsPreview->currentSize()>MIN_FONT_SIZE);
    itsToggleWaterfallAction->setChecked(itsPreview->waterfall() && CGlobal::fe().isScaleable());
    itsToggleWaterfallAction->setEnabled(st && CGlobal::fe().isScaleable());
    itsChangeTextAction->setEnabled(st);
}

void CFontViewPart::install()
{
    int resp=CMisc::root() ? KMessageBox::Yes
                           : KMessageBox::questionYesNoCancel(itsFrame,
                                                              i18n("Where do you wish to install \"%1\" (%2)?\n"
                                                                   "\"%3\" - only accessible to you, or\n"
                                                                   "\"%4\" - accessible to all (requires administrator "
                                                                   "password)")
                                                                   .arg(CGlobal::fe().getFullName()).arg(m_url.fileName())
                                                                   .arg(i18n(KIO_FONTS_USER)).arg(i18n(KIO_FONTS_SYS)),
                                                              i18n("Install"), i18n(KIO_FONTS_USER), i18n(KIO_FONTS_SYS));
    if(KMessageBox::Cancel!=resp)
    {
        KIO::UDSEntry uds;
        KURL          destUrl(getDest(CMisc::getFile(m_url.path()), KMessageBox::No==resp));

        if(KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
            KMessageBox::error(itsFrame, i18n("%1:%2 already installed!").arg(m_url.protocol()).arg(m_url.path()),
                               i18n("Error"));
        else
            if(KIO::NetAccess::copy(KURL(m_file), destUrl, itsFrame->parentWidget()))
            {
                if(CFontEngine::TYPE_1==CGlobal::fe().getType())
                {
                    const char *others[]={ "afm", "AFM", NULL };

                    for(int i=0; others[i]; i++)
                    {
                        KURL          afmUrl(m_url);
                        KIO::UDSEntry uds;

                        afmUrl.setPath(CMisc::changeExt(m_url.path(), others[i]));
                        destUrl.setPath(CMisc::changeExt(destUrl.path(), others[i]));

                        if(KIO::NetAccess::stat(afmUrl, uds, itsFrame->parentWidget()))
                        {
                           if(!KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
                               KIO::NetAccess::copy(afmUrl, destUrl, itsFrame->parentWidget());
                           break;
                        }
                    }
                }

                KMessageBox::information(itsFrame, i18n("%1:%2 successfully installed!").arg(m_url.protocol())
                                                       .arg(m_url.path()), i18n("Success"),
                                         "FontViewPart_DisplayInstallationSuccess");
            }
            else
                KMessageBox::error(itsFrame, i18n("Could not install %1:%2").arg(m_url.protocol()).arg(m_url.path()),
                                   i18n("Error"));
    }
}

void CFontViewPart::zoomIn()
{
    itsPreview->showSize(itsPreview->currentSize()+FONT_SIZE_STEP);
    if(itsPreview->currentSize()>=MAX_FONT_SIZE)
        itsZoomInAction->setEnabled(false);
    itsZoomOutAction->setEnabled(true);
}

void CFontViewPart::zoomOut()
{
    itsPreview->showSize(itsPreview->currentSize()-FONT_SIZE_STEP);
    if(itsPreview->currentSize()<=MIN_FONT_SIZE)
        itsZoomOutAction->setEnabled(false);
    itsZoomInAction->setEnabled(true);
}

void CFontViewPart::changeText()
{
    bool             status;
    QRegExpValidator validator(QRegExp(".*"), 0L);
    QString          oldStr(CGlobal::fe().getPreviewString()),
                     newStr(KInputDialog::getText(i18n("Preview String"), i18n("Please enter new string:"),
                                                  oldStr, &status, itsFrame,
                                                  "preview string dialog", &validator));

    if(status && newStr!=oldStr)
    {
        CGlobal::fe().setPreviewString(newStr);
        itsPreview->showFont();
    }
}

void CFontViewPart::toggleWaterfall()
{
    itsZoomInAction->setEnabled(itsPreview->waterfall() && CGlobal::fe().isScaleable() &&
                                itsPreview->currentSize()<MAX_FONT_SIZE);
    itsZoomOutAction->setEnabled(itsPreview->waterfall() && CGlobal::fe().isScaleable() &&
                                 itsPreview->currentSize()>MIN_FONT_SIZE);
    itsPreview->showWaterfall(!itsPreview->waterfall());
}

#include "FontViewPart.moc"
