    /*

    Shutdown dialog. Class KDMShutdown
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


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
 

#ifndef KDMSHUTDOWN_H
#define KDMSHUTDOWN_H

#include "kdm-config.h"

#define QT_CLEAN_NAMESPACE
#include <qglobal.h>

#include <X11/Xmd.h>

#include <sys/param.h>	// for BSD
#include <stdlib.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include "kfdialog.h"

class KDMShutdown : public FDialog {
     Q_OBJECT
public:
     KDMShutdown( int mode, QWidget* _parent=0, const char* _name=0,
		  const QString &_shutdown = QString::fromLatin1(SHUTDOWN_CMD),
		  const QString &_restart  = QString::fromLatin1(REBOOT_CMD),
#ifndef BSD
		  const QString &_console = QString::fromLatin1("/sbin/init 3"),
#endif
		  bool _lilo = FALSE,
		  const QString &_lilocmd = QString::null,
                  const QString &_lilomap = QString::null);
private slots:
     void rb_clicked(int);
     void pw_entered();
     void bye_bye();
     void target_changed(int);
private:
     QLabel*       label;
     QButtonGroup* btGroup;
     QPushButton*  okButton;
     QPushButton*  cancelButton;
     QLineEdit*    pswdEdit;
     QString       cur_action;
     QString       shutdown;
     QString       restart;
#ifndef BSD
     QString       console;
#endif
     QRadioButton  *restart_rb;
     bool          lilo;
     int           liloTarget;
     QString       liloCmd, liloMap;
};

#endif /* KDMSHUTDOWN_H */








