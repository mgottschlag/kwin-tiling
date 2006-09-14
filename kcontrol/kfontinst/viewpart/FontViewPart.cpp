////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontViewPart
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#include "FontViewPart.h"
#include "FontPreview.h"
#include "Misc.h"
#include "KfiConstants.h"
#include "KfiPrint.h"
#include <klocale.h>
#include <QLayout>
#include <QPushButton>
#include <qframe.h>
#include <QFile>
#include <QLabel>
#include <QPainter>
#include <QValidator>
#include <QRegExp>
#include <qsettings.h>
#include <QStringList>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <kio/netaccess.h>
#include <kicon.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstdaction.h>
#include <kaction.h>
#include <kinputdialog.h>
#include <kdialog.h>
#include <kprinter.h>
#include <string.h>
#include <fontconfig/fontconfig.h>

static KUrl getDest(const KUrl &url, bool system)
{
    return KUrl(KFI::Misc::root()
                  ? QString("fonts:/") + url.fileName()
                  : QString("fonts:/") + QString(system ? i18n(KFI_KIO_FONTS_SYS) : i18n(KFI_KIO_FONTS_USER))
                                       + QChar('/') + url.fileName());
}

namespace KFI
{

CFontViewPart::CFontViewPart(QWidget *parent, const char *name)
{
    bool kcm = false;
    if (name) {
        kcm = (0 == strcmp(name, "kcmfontinst"));
    }

    itsFrame=new QFrame(parent);

    itsFrame->setObjectName("frame");

    QFrame *previewFrame=new QFrame(itsFrame);

    itsToolsFrame=new QFrame(itsFrame);

    QVBoxLayout *layout=new QVBoxLayout(itsFrame);
    layout->setSpacing(kcm ? 0 : KDialog::spacingHint());
    layout->setMargin(kcm ? 0 : KDialog::marginHint());
    QGridLayout *previewLayout=new QGridLayout(previewFrame);
    previewLayout->setSpacing(1);
    previewLayout->setMargin(1);
    QHBoxLayout *toolsLayout=new QHBoxLayout(itsToolsFrame);
    toolsLayout->setSpacing(KDialog::spacingHint());
    toolsLayout->setMargin(0);

    itsFrame->setFrameShape(QFrame::NoFrame);
    itsFrame->setFocusPolicy(Qt::ClickFocus);
    itsToolsFrame->setFrameShape(QFrame::NoFrame);
    previewFrame->setFrameShadow(kcm ? QFrame::Sunken : QFrame::Raised);
    previewFrame->setFrameShape(QFrame::Panel);
    setInstance(new KInstance("kfontview"));

    itsPreview=new CFontPreview(previewFrame);
    itsPreview->setObjectName("FontViewPart::Preview");
    itsPreview->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    itsFaceLabel=new QLabel(i18n("Face:"), itsToolsFrame);
    itsFaceSelector=new KIntNumInput(1, itsToolsFrame);
    itsInstallButton=new QPushButton(i18n("Install..."), itsToolsFrame);
    itsInstallButton->setObjectName("button");
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

    itsChangeTextAction = new KAction(KIcon("text"), i18n("Change Text..."), actionCollection(), "changeText");
    connect(itsChangeTextAction, SIGNAL(triggered(bool)), SLOT(changeText()));
    itsChangeTextAction->setEnabled(false);
    itsPrintAction=KStdAction::print(this, SLOT(print()), actionCollection(), "print");
    itsPrintAction->setEnabled(false);

    setXMLFile("kfontviewpart.rc");
    setWidget(itsFrame);
}

bool CFontViewPart::openUrl(const KUrl &url)
{
    if (!url.isValid() || !closeUrl())
        return false;

    if(KFI_KIO_FONTS_PROTOCOL==url.protocol() || url.isLocalFile())
    {
        m_url=url;
        emit started(0);
        m_file = m_url.path();
        bool ret=openFile();
        if (ret)
        {
            emit completed();
            emit setWindowCaption(m_url.prettyUrl());
        }
        return ret;
    }
    else
        return ReadOnlyPart::openUrl(url);
}

bool CFontViewPart::openFile()
{
    bool showFs=false,
         isFonts=KFI_KIO_FONTS_PROTOCOL==m_url.protocol();

    if(isFonts)
        FcInitReinitialize();

    itsPreview->showFont(KUrl(isFonts ? m_url.url() :m_file));

    if(!isFonts && itsPreview->engine().getNumIndexes()>1)
    {
        showFs=true;
        itsFaceSelector->setRange(1, itsPreview->engine().getNumIndexes(), 1, false);
    }

    itsShowInstallButton=false;
    itsFaceLabel->setVisible(showFs);
    itsFaceSelector->setVisible(showFs);
    itsToolsFrame->hide();

    if(!isFonts)
    {
        KUrl          destUrl;
        KIO::UDSEntry uds;

        //
        // Not from fonts:/, so try to see if font is already installed...
        if(Misc::root())
        {
            destUrl=QString("fonts:/")+itsPreview->engine().getName(m_url);
            itsShowInstallButton=KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()) ? false : true;
        }
        else
        {
            destUrl=QString("fonts:/")+i18n(KFI_KIO_FONTS_SYS)+QChar('/')+itsPreview->engine().getName(m_url);
            if(KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()))
                itsShowInstallButton=false;
            else
            {
                destUrl=QString("fonts:/")+i18n(KFI_KIO_FONTS_USER)+QChar('/')+itsPreview->engine().getName(m_url);
                itsShowInstallButton=KIO::NetAccess::stat(destUrl, uds, itsFrame->parentWidget()) ? false : true;
            }
        }
    }

    return true;
}

void CFontViewPart::previewStatus(bool st)
{
    itsInstallButton->setVisible(st && itsShowInstallButton);
    itsToolsFrame->setVisible(!itsInstallButton->isHidden()||!itsFaceSelector->isHidden());
    itsChangeTextAction->setEnabled(st);
    itsPrintAction->setEnabled(st && KFI_KIO_FONTS_PROTOCOL==m_url.protocol());
}

void CFontViewPart::install()
{
    int resp=Misc::root() ? KMessageBox::Yes
                           : KMessageBox::questionYesNoCancel(itsFrame,
                                                              i18n("Where do you wish to install \"%1\" (%2)?\n"
                                                                   "\"%3\" - only accessible to you, or\n"
                                                                   "\"%4\" - accessible to all (requires administrator "
                                                                   "password)",
                                                                    itsPreview->engine().getName(m_url),
                                                                    m_url.fileName(),
                                                                    i18n(KFI_KIO_FONTS_USER),
                                                                    i18n(KFI_KIO_FONTS_SYS)),
                                                              i18n("Install"), KGuiItem(i18n(KFI_KIO_FONTS_USER)),
                                                              KGuiItem(i18n(KFI_KIO_FONTS_SYS)));

    if(KMessageBox::Cancel!=resp)
    {
        KUrl destUrl(getDest(m_url, KMessageBox::No==resp));

        if(KIO::NetAccess::file_copy(m_url, destUrl, itsFrame->parentWidget()))
        {
            //
            // OK file copied, now look for any AFM or PFM file...
            KUrl::List urls;

            Misc::getAssociatedUrls(m_url, urls);

            if(urls.count())
            {
                KUrl::List::Iterator it,
                                     end=urls.end();

                for(it=urls.begin(); it!=end; ++it)
                {
                    destUrl=getDest(*it, KMessageBox::No==resp);
                    KIO::NetAccess::file_copy(*it, destUrl, itsFrame->parentWidget());
                }
            }

            KMessageBox::information(itsFrame, i18n("%1:%2 successfully installed.", m_url.protocol(),
                                                    m_url.path()), i18n("Success"),
                                     "FontViewPart_DisplayInstallationSuccess");
            itsShowInstallButton=false;
            itsInstallButton->setVisible(itsShowInstallButton);
        }
        else
            KMessageBox::error(itsFrame, i18n("Could not install %1:%2", m_url.protocol(), m_url.path()),
                               i18n("Error"));
    }
}

void CFontViewPart::changeText()
{
    bool             status;
    QRegExpValidator validator(QRegExp(".*"), 0L);
    QString          oldStr(itsPreview->engine().getPreviewString()),
                     newStr(KInputDialog::getText(i18n("Preview String"), i18n("Please enter new string:"),
                                                  oldStr, &status, itsFrame, &validator));

    if(status && newStr!=oldStr)
    {
        itsPreview->engine().setPreviewString(newStr);
        itsPreview->showFont();
    }
}

void CFontViewPart::print()
{
    QStringList items;

    items.append(itsPreview->engine().getName(m_url));

    Print::printItems(items, 0, itsFrame->parentWidget(), itsPreview->engine());
}

}

#include "FontViewPart.moc"
