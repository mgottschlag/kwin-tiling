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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef KCONTROL_NIC_H
#define KCONTROL_NIC_H

#include <qtabwidget.h>
#include <kglobal.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlistview.h>

#include <kcmodule.h>
#include <kaboutdata.h>

class KCMNic:public KCModule
{
   Q_OBJECT
   public:
      KCMNic(QWidget *parent=0, const char * name=0, const QStringList &list = QStringList( ));
      virtual ~KCMNic() {};
      const KAboutData* aboutData() const;

   protected slots:
      void update();
   protected:
      QListView *m_list;
      QPushButton *m_updateButton;
};

#endif

