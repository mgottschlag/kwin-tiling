/*

  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/                                                                            



#include <qlistview.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qheader.h>


#include <klocale.h>


#include "closedlg.h"
#include "closedlg.moc"


CloseDialog::CloseDialog(QWidget *parent)
  : QDialog(parent, 0, true)
{
  setCaption(i18n("Unsaved changes"));

  QVBoxLayout *vbox = new QVBoxLayout(this,6,6);

  QLabel *l = new QLabel(i18n("There are unsaved changes in the following "
			      "modules. The changes will be lost if you quit!"), this);
  vbox->addWidget(l);

  _unsaved = new QListView(this);
  _unsaved->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);   
  _unsaved->addColumn("");   
  _unsaved->header()->hide();
  vbox->addWidget(_unsaved);

  QHBoxLayout *hbox = new QHBoxLayout(vbox,4);
  hbox->addStretch();

  QPushButton *btn = new QPushButton(i18n("Quit"), this);
  hbox->addWidget(btn);
  connect(btn, SIGNAL(clicked()), this, SLOT(accept()));

  btn = new QPushButton(i18n("Cancel"), this);
  hbox->addWidget(btn);
  connect(btn, SIGNAL(clicked()), this, SLOT(reject()));
}


void CloseDialog::addUnsaved(QPixmap icon, QString text)
{
  QListViewItem *item = new QListViewItem(_unsaved, text);
  item->setPixmap(0,icon);
}
