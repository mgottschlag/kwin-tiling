/*
 * nic.h
 *
 *  Copyright (C) 2001 Alexander Neundorf <neundorf@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef KCONTROL_NIC_H
#define KCONTROL_NIC_H

#include <qtabwidget.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include <kcmodule.h>

class KCMNic:public KCModule
{
   Q_OBJECT
   public:
      KCMNic(QWidget *parent=0, const char * name=0);
      virtual ~KCMNic() {};

   protected slots:
      void update();
   protected:
      QListView *m_list;
      QPushButton *m_updateButton;
};

#endif

