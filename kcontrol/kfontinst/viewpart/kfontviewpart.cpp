////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFontViewPart
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2002
////////////////////////////////////////////////////////////////////////////////

//
// Define to stop FontPreview.cpp #including stuff not required here...
#define KFI_THUMBNAIL

#include "kfontviewpart.h"
#define KFI_THUMBNAIL
#include "FontPreview.cpp" // CPD: Hack...
#include "Misc.cpp"
#include <kapplication.h>
#include <klocale.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <kprocess.h>

KFontViewPart::KFontViewPart(QWidget *parent, const char *)
{
    QWidget     *widget=new QWidget(parent, "FontViewPart::Widget");
    QGridLayout *layout=new QGridLayout(widget, 2, 2, 11, 6);

    itsInstallButton=new QPushButton(i18n("Install..."), widget);

    widget->setFocusPolicy(QWidget::ClickFocus);
    setInstance(new KInstance("kfontviewpart"));
    itsPreview=new CFontPreview(widget, "FontViewPart::Preview", i18n(" Loading file..."));
    layout->addMultiCellWidget(itsPreview, 0, 0, 0, 1);
    layout->addItem(new QSpacerItem(5, 5, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 0);
    layout->addWidget(itsInstallButton, 1, 1);
    itsInstallButton->setEnabled(false);
    connect(itsPreview, SIGNAL(status(bool)), itsInstallButton, SLOT(setEnabled(bool)));
    connect(itsInstallButton, SIGNAL(clicked()), this, SLOT(installFont()));
    setWidget(widget);
}

KFontViewPart::~KFontViewPart()
{
}

bool KFontViewPart::openFile()
{
    itsInstallButton->setEnabled(false);
    itsPreview->showFont(m_file);
    return true;
}

void KFontViewPart::installFont()
{
    KProcess proc;

    proc << "kcmfontinst_install_fonts.sh" << m_file;
    proc.start(KProcess::DontCare);
}

#include "kfontviewpart.moc"
