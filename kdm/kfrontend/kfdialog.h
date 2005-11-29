/*

Dialog class that handles input focus in absence of a wm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#ifndef FDIALOG_H
#define FDIALOG_H

#include <qdialog.h>
#include <qmessagebox.h>
//Added by qt3to4:
#include <QFrame>
#include <QResizeEvent>

class QFrame;

class FDialog : public QDialog {
	typedef QDialog inherited;

  public:
	FDialog( QWidget *parent = 0, bool framed = true );
	virtual int exec();

	static void box( QWidget *parent, QMessageBox::Icon type,
	                 const QString &text );
#define errorbox QMessageBox::Critical
#define sorrybox QMessageBox::Warning
#define infobox QMessageBox::Information
	void MsgBox( QMessageBox::Icon typ, const QString &msg ) { box( this, typ, msg ); }

  protected:
	virtual void resizeEvent( QResizeEvent *e );
	void adjustGeometry();

  private:
	QFrame *winFrame;
};

class KFMsgBox : public FDialog {
	typedef FDialog inherited;

  public:
	KFMsgBox( QWidget *parent, QMessageBox::Icon type, const QString &text );
};

#endif /* FDIALOG_H */
