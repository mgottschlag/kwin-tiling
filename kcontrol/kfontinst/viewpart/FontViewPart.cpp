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
#include "Config.h"
#include "Misc.h"
#include <klocale.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include "../kio/KioFonts.h"

CFontViewPart::CFontViewPart(QWidget *parent, const char *)
{
    CGlobal::create(true, false);

    itsFrame=new QFrame(parent, "frame");

    QGridLayout *layout=new QGridLayout(itsFrame, 2, 2, 11, 6);

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(QWidget::ClickFocus);
    setInstance(new KInstance("kfontviewpart"));
    itsPreview=new CFontPreview(itsFrame, "FontViewPart::Preview", i18n("Loading file..."));
    itsPreview->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)3, 0, 0,
                              itsPreview->sizePolicy().hasHeightForWidth()));
    itsInstallButton=new QPushButton(i18n("Install..."), itsFrame, "button");
    itsInstallButton->hide();
    layout->addMultiCellWidget(itsPreview, 0, 0, 0, 1);
    layout->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 0);
    layout->addWidget(itsInstallButton, 1, 1);
    connect(itsPreview, SIGNAL(status(bool)), SLOT(previewStatus(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), SLOT(install()));
    setWidget(itsFrame);
}

CFontViewPart::~CFontViewPart()
{
    CGlobal::destroy();
}

bool CFontViewPart::openFile()
{
    itsPreview->showFont(m_file);
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
                                                           KIO_FONTS_USER_DIR" - only accessible to you, or\n"
                                                           KIO_FONTS_SYS_DIR" - accessible to all (requires administrator password)").arg(m_url.protocol()).arg(m_url.path()),
                                            i18n("Install"), i18n(KIO_FONTS_USER_DIR), i18n(KIO_FONTS_SYS_DIR));

    if(KMessageBox::Cancel!=resp)
    {
        QString sub("/");

        if(CMisc::root() || KMessageBox::No==resp)
            switch(CFontEngine::getType(QFile::encodeName(m_url.path())))
            {
                case CFontEngine::TRUE_TYPE:
                case CFontEngine::OPEN_TYPE:
                case CFontEngine::TT_COLLECTION:
                    sub=CGlobal::cfg().getSysTTSubDir()+QString("/");
                    break;
                case CFontEngine::TYPE_1:
                    sub=CGlobal::cfg().getSysT1SubDir()+QString("/");
                    break;
                default:
                    break;
            }

        QString       destDir(CMisc::root() ? sub : QString((KMessageBox::Yes==resp ? KIO_FONTS_USER_DIR : KIO_FONTS_SYS_DIR))+sub);
        KURL          destUrl(QString("fonts:/")+destDir+CMisc::getFile(m_url.path()));
        KIO::UDSEntry uds;

        if(KIO::NetAccess::stat(destUrl, uds))
            KMessageBox::error(itsFrame, i18n("%1:%2 already installed!").arg(m_url.protocol()).arg(m_url.path()), i18n("Error"));
        else
            if(KIO::NetAccess::copy(m_file, destUrl))
            {
                if(CFontEngine::isAType1(QFile::encodeName(m_url.path())))
                {
                    KURL          afmUrl(m_url);
                    KIO::UDSEntry uds;

                    afmUrl.setPath(CMisc::changeExt(m_url.path(), "afm"));
                    destUrl.setPath(CMisc::changeExt(destUrl.path(), "afm"));

                    if(KIO::NetAccess::stat(afmUrl, uds) && !KIO::NetAccess::stat(destUrl, uds))
                        KIO::NetAccess::copy(afmUrl, destUrl);
                }

                KMessageBox::information(itsFrame, i18n("%1:%2 successfully installed!").arg(m_url.protocol()).arg(m_url.path()), i18n("Success"),
                                         "FontViewPart_DisplayInstallationSuccess");
            }
            else
                KMessageBox::error(itsFrame, i18n("Could not install %1:%2").arg(m_url.protocol()).arg(m_url.path()), i18n("Error"));
    }
}

#include "FontViewPart.moc"
