    /*

    Shutdown dialog. Class KDMShutdown
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>


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

#ifndef QT_CLEAN_NAMESPACE
# define QT_CLEAN_NAMESPACE
#endif
#include <qglobal.h>

#include <sys/param.h>	// for BSD
#include <stdlib.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <kpassdlg.h>
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
    void bye_bye();
    void target_changed(int);

private:
    QLabel		*label;
    QButtonGroup	*btGroup;
    QPushButton		*okButton;
    QPushButton		*cancelButton;
    KPasswordEdit	*pswdEdit;
    QString		cur_action;
    QString		shutdown, restart;
#ifndef BSD
    QString		console;
#endif
    QRadioButton	*restart_rb;
    bool		lilo;
    int			liloTarget;
    QString		liloCmd, liloMap;
};

#endif /* KDMSHUTDOWN_H */
