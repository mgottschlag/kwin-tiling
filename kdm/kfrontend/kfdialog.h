    /*

    Dialog class that handles input focus in absence of a wm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000, 2001 Oswald Buddenhagen <ossi@kde.org>


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
 

#ifndef FDIALOG_H
#define FDIALOG_H

#include <qdialog.h>
#include <qframe.h>
#include <qmessagebox.h>

class FDialog : public QDialog {
    Q_OBJECT

public:
    FDialog( QWidget *parent = 0, const char* name = 0, bool modal = true );
    virtual int exec();

protected:
    QFrame *winFrame;
};

class KFMsgBox : public FDialog {

private:
    KFMsgBox( QWidget *parent, QMessageBox::Icon type, const QString &text );

public:
    static void box( QWidget *parent, QMessageBox::Icon type, 
		     const QString &text );
};

#endif /* FDIALOG_H */
