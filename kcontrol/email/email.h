/**
 * email.h
 *
 * Copyright (c) 1999 Preston Brown <pbrown@redhat.com>
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

#ifndef _EMAIL_H
#define _EMAIL_H

#include <kcmodule.h>

class QLineEdit;
class QRadioButton;
class QButtonGroup;

class KEmailConfig : public KCModule
{
  Q_OBJECT
public:
  KEmailConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KEmailConfig();

  void load();
  void save();
  void defaults();
  
  int buttons();
  
public slots:
  void configChanged();
      
private:
 QLineEdit *fullName, *organization;
 QLineEdit *emailAddr, *replyAddr;

 QLineEdit *userName, *password, *inServer, *outServer;
 QButtonGroup *bGrp;
 QRadioButton *pop3Button, *imapButton, *localButton;

};

#endif
