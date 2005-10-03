/**
 * smartcard.h
 *
 * Copyright (c) 2001 George Staikos <staikos@kde.org>
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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _KCM_SMARTCARD_H
#define _KCM_SMARTCARD_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <dcopobject.h>

#include <kcmodule.h>

#include "smartcardbase.h"
#include "nosmartcardbase.h"

class KConfig;
class KCardDB;
class KMenu;
class KListViewItem;

class KSmartcardConfig : public KCModule, public DCOPObject
{
  K_DCOP
    Q_OBJECT


public:
  KSmartcardConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KSmartcardConfig();

  SmartcardBase *base;

  void load();
  void save();
  void defaults();

  int buttons();
  QString quickHelp() const;

 k_dcop:


 void updateReadersState (QString readerName,
                          bool isCardPresent,
                          QString atr);
 void loadReadersTab (QStringList lr);

  private slots:

  void slotShowPopup(QListViewItem * item ,const QPoint & _point,int i);
  void slotLaunchChooser();



private:

  KConfig *config;
  bool _ok;
  KCardDB * _cardDB;
  KMenu * _popUpKardChooser;

  void loadSmartCardSupportTab();
  void getSupportingModule( KListViewItem * ant,
                            QString & cardATR) const ;


};

#endif

