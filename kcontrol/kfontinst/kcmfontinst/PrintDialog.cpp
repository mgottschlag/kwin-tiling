////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CPrintDialog
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 12/05/2005
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
// (C) Craig Drummond, 2005
////////////////////////////////////////////////////////////////////////////////

#include "PrintDialog.h"
#include <QLayout>
#include <qframe.h>
#include <QLabel>
//Added by qt3to4:
#include <QGridLayout>
#include <klocale.h>

namespace KFI
{

CPrintDialog::CPrintDialog(QWidget *parent)
            : KDialogBase(Plain, i18n("Print Font Samples"), Ok|Cancel, Ok, parent, NULL, true, false)
{
    QFrame      *page=plainPage();
    QGridLayout *layout=new QGridLayout(page);
    layout->setSpacing(spacingHint());
    layout->setMargin(0);

    layout->addWidget(new QLabel(i18n("Output:"), page), 0, 0);
    itsOutput=new QComboBox(page);
    itsOutput->insertItem(0, i18n("All Fonts"));
    itsOutput->insertItem(1, i18n("Selected Fonts"));
    layout->addWidget(itsOutput, 0, 1);
    layout->addWidget(new QLabel(i18n("Font size:"), page), 1, 0);
    itsSize=new QComboBox(page);
    itsSize->insertItem(0, i18n("Waterfall"));
    itsSize->insertItem(1, i18n("12pt"));
    itsSize->insertItem(2, i18n("18pt"));
    itsSize->insertItem(3, i18n("24pt"));
    itsSize->insertItem(4, i18n("36pt"));
    itsSize->insertItem(5, i18n("48pt"));
    layout->addWidget(itsSize, 1, 1);
    layout->addItem(new QSpacerItem(2, 2), 2, 1);
}

bool CPrintDialog::exec(bool select, int size)
{
    if(!select)
    {
        itsOutput->setCurrentIndex(0);
        itsOutput->setEnabled(false);
    }
    else
        itsOutput->setCurrentIndex(1);
    itsSize->setCurrentIndex(size);
    return QDialog::Accepted==QDialog::exec();
}

}
