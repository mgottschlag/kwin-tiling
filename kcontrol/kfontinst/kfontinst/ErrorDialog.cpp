////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CErrorDialogDialog
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 02/05/2001
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
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "ErrorDialog.h"
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlayout.h>
#include <klocale.h>

CErrorDialog::CErrorDialog(QWidget *parent, const char *name)
             : KDialog(parent, name, true)
{
    resize(312, 239);
    setCaption(i18n("Errors"));

    QGridLayout *dialogLayout = new QGridLayout(this, 1, 1, 11, 6);
    QHBoxLayout *layout = new QHBoxLayout(0, 0, 6);
    QSpacerItem *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QPushButton *buttonOk = new QPushButton(i18n("&OK"), this);

    layout->addItem(spacer);
    layout->addWidget(buttonOk);

    dialogLayout->addLayout(layout, 1, 0);

    itsGroupBox = new QGroupBox(this);
    itsGroupBox->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)5, 0, 0, itsGroupBox->sizePolicy().hasHeightForWidth()));
    itsGroupBox->setTitle("12345678901234567890123456789012345678901234567890");
    itsGroupBox->setColumnLayout(0, Qt::Vertical);
    itsGroupBox->layout()->setSpacing(6);
    itsGroupBox->layout()->setMargin(11);
    QGridLayout *groupBoxLayout = new QGridLayout(itsGroupBox->layout());
    groupBoxLayout->setAlignment(Qt::AlignTop);

    itsListView = new QListView(itsGroupBox);
    itsListView->addColumn(i18n("Item"));
    itsListView->addColumn(i18n("Reason"));
    itsListView->setSelectionMode(QListView::NoSelection);

    groupBoxLayout->addWidget(itsListView, 0, 0);
    dialogLayout->addWidget(itsGroupBox, 0, 0);

    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
}

CErrorDialog::~CErrorDialog()
{
}

void CErrorDialog::add(const QString &file, const QString &reason)
{
    new QListViewItem(itsListView, file, reason);
}

void CErrorDialog::open(const QString &str)
{
    itsGroupBox->setTitle(str);
    exec();
}

#include "ErrorDialog.moc"
