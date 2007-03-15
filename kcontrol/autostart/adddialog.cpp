/***************************************************************************
 *   Copyright (C) 2007 by Stephen Leaf                                    *
 *   smileaf@smileaf.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "adddialog.h"

AddDialog::AddDialog (QWidget* parent) : QDialog( parent ) {
	widget = new Ui_AddDialog();
	widget->setupUi(this);

	connect( widget->btnImport, SIGNAL(clicked()), SLOT(importPrg()) );
	connect( widget->btnAdd, SIGNAL(clicked()), SLOT(addPrg()) );
	connect( widget->btnCancel, SIGNAL(clicked()), SLOT(reject()) );
}

AddDialog::~AddDialog()
{}

void AddDialog::importPrg() {
	done(3);
}
void AddDialog::addPrg() {
	done(4);
}

#include "adddialog.moc"
