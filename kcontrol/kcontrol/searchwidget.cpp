/*
  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 
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

#include <qlistbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>

#include "searchwidget.h"
#include "searchwidget.moc"

SearchWidget::SearchWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  QVBoxLayout * l = new QVBoxLayout(this, 2, 2);

  // input
  _input = new QLineEdit(this);
  QLabel *inputl = new QLabel(_input, i18n("Se&arch:"), this);

  l->addWidget(inputl);
  l->addWidget(_input);

  // keyword list
  _keyList = new QListBox(this);
  QLabel *keyl = new QLabel(_keyList, i18n("&Keywords:"), this);  

  l->addWidget(keyl);
  l->addWidget(_keyList);

  // result list
  _resultList = new QListBox(this);
  QLabel *resultl = new QLabel(_keyList, i18n("&Results:"), this);  

  l->addWidget(resultl);
  l->addWidget(_resultList);
}
