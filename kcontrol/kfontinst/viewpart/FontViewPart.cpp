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
#include <klocale.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qfile.h>
#include <qlabel.h>
#include <kio/netaccess.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <knuminput.h>

CFontViewPart::CFontViewPart(QWidget *parent, const char *)
{
    CGlobal::create(true, false);

    itsFrame=new QFrame(parent, "frame");

    QFrame      *previewFrame=new QFrame(itsFrame);
    QGridLayout *layout=new QGridLayout(itsFrame, 2, 2, 11, 6),
                *previewLayout=new QGridLayout(previewFrame, 1, 1, 1, 1);

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(QWidget::ClickFocus);
    previewFrame->setFrameShadow(QFrame::Raised);
    previewFrame->setFrameShape(QFrame::Panel);
    setInstance(new KInstance("kfontviewpart"));
    itsPreview=new CFontPreview(previewFrame, "FontViewPart::Preview", i18n("Loading file..."));
    itsPreview->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0,
                              itsPreview->sizePolicy().hasHeightForWidth()));
    itsFaceLabel=new QLabel(i18n("Face:"), itsFrame);
    itsFaceSelector=new KIntNumInput(1, itsFrame);
    itsInstallButton=new QPushButton(i18n("Install..."), itsFrame, "button");
    itsInstallButton->hide();
    previewLayout->addWidget(itsPreview, 0, 0);
    layout->addMultiCellWidget(previewFrame, 0, 0, 0, 3);
    layout->addWidget(itsFaceLabel, 1, 0);
    layout->addWidget(itsFaceSelector, 1, 1);
    itsFaceLabel->hide();
    itsFaceSelector->hide();
    layout->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 2);
    layout->addWidget(itsInstallButton, 1, 3);
    connect(itsPreview, SIGNAL(status(bool)), SLOT(previewStatus(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), SLOT(install()));
    connect(itsFaceSelector, SIGNAL(valueChanged(int)), SLOT(showFace(int)));
    setWidget(itsFrame);
}

CFontViewPart::~CFontViewPart()
{
    CGlobal::destroy();
}

bool CFontViewPart::openFile()
{
    if(CFontEngine::isATtc(QFile::encodeName(m_url.path())) && CGlobal::fe().openFont(m_url, CFontEngine::TEST, true))
    {
        if(CGlobal::fe().getNumFaces()>1)
        {
            itsFaceLabel->show();
            itsFaceSelector->show();
            itsFaceSelector->setRange(1, CGlobal::fe().getNumFaces(), 1, false);
        }
        CGlobal::fe().closeFont();
    }

    if(KIO_FONTS_PROTOCOL!=m_url.protocol())
        itsInstallButton->show();

    itsPreview->showFont(m_url);

    return true;
}

void CFontViewPart::previewStatus(bool st)
{
    if(st && KIO_FONTS_PROTOCOL!=m_url.protocol())
        itsInstallButton->show();
    else
        itsInstallButton->hide();
}

void CFontViewPart::install()
{
    int resp=CMisc::root() ? KMessageBox::Yes
                           : KMessageBox::questionYesNoCancel(itsFrame, i18n("Where do you wish to install %1:%2?\n"
                                                                             "\"%3\" - only accessible to you, or\n"
                                                                             "\"%4\" - accessible to all (requires administrator password)")
                                                                             .arg(m_url.protocol()).arg(m_url.path())
                                                                             .arg(i18n(KIO_FONTS_USER)).arg(i18n(KIO_FONTS_SYS)),
                                            i18n("Install"), i18n(KIO_FONTS_USER), i18n(KIO_FONTS_SYS));

    if(KMessageBox::Cancel!=resp)
    {
        QString sub("");

        if(CMisc::root() || KMessageBox::No==resp)
            switch(CFontEngine::getType(QFile::encodeName(m_url.path())))
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

        KIO::UDSEntry uds;
        KURL          destUrl(QString("fonts:/")+
                                      (CMisc::root()
                                           ? sub
                                           : QString((KMessageBox::Yes==resp ? i18n(KIO_FONTS_USER) : i18n(KIO_FONTS_SYS))+QChar('/')+sub)
                                      )+
                                      CMisc::getFile(m_url.path()));

        if(KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
            KMessageBox::error(itsFrame, i18n("%1:%2 already installed!").arg(m_url.protocol()).arg(m_url.path()), i18n("Error"));
        else
            if(KIO::NetAccess::copy(KURL( m_file ), destUrl, itsFrame->parentWidget()))
            {
                if(CFontEngine::isAType1(QFile::encodeName(m_url.path())))
                {
                    KURL          afmUrl(m_url);
                    KIO::UDSEntry uds;

                    afmUrl.setPath(CMisc::changeExt(m_url.path(), "afm"));
                    destUrl.setPath(CMisc::changeExt(destUrl.path(), "afm"));

                    if(KIO::NetAccess::stat(afmUrl, uds, itsFrame->parentWidget()) && !KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
                        KIO::NetAccess::copy(afmUrl, destUrl, itsFrame->parentWidget());
                }

                KMessageBox::information(itsFrame, i18n("%1:%2 successfully installed!").arg(m_url.protocol()).arg(m_url.path()), i18n("Success"),
                                         "FontViewPart_DisplayInstallationSuccess");
            }
            else
                KMessageBox::error(itsFrame, i18n("Could not install %1:%2").arg(m_url.protocol()).arg(m_url.path()), i18n("Error"));
    }
}

void CFontViewPart::showFace(int face)
{
    itsPreview->showFace(face);
}

#include "FontViewPart.moc"
