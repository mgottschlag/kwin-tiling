/* This file is part of the KDE Display Manager Configuration package
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/  

#ifndef __KDMUSERS_H__
#define __KDMUSERS_H__

#include <qwidget.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qimage.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qwidgetstack.h>

#include <klineedit.h>
#include <klistview.h>
#include <kcombobox.h>
#include <kurl.h>

#include <pwd.h>


class KDMUsersWidget : public QWidget
{
	Q_OBJECT

public:
	KDMUsersWidget( QWidget *parent = 0, const char *name = 0 );

	void load();
	void save();
	void defaults();
	void makeReadOnly();

	bool eventFilter( QObject *o, QEvent *e );

public slots:
	void slotClearUsers();
	void slotAddUsers( const QMap<QString,int> & );
	void slotDelUsers( const QMap<QString,int> & );

signals:
	void changed( bool state );
	void setMinMaxUID( int, int );

private slots:
	void slotMinMaxChanged();
	void slotShowOpts();
	void slotUpdateOptIn( QListViewItem *item );
	void slotUpdateOptOut( QListViewItem *item );
	void slotUserSelected();
	void slotUnsetUserPix();
	void slotFaceOpts();
	void slotUserButtonClicked();
	void slotChanged();

private:
	void updateOptList( QListViewItem *item, QStringList &list );
	void userButtonDropEvent( QDropEvent *e );
	void changeUserPix( const QString & );

	QGroupBox	*minGroup;	// top left
	QLineEdit	*leminuid, *lemaxuid;

	QButtonGroup	*usrGroup; // right below
	QCheckBox	*cbshowlist, *cbcomplete, *cbinverted, *cbusrsrt;

	QLabel		*s_label; // middle
	QWidgetStack	*wstack;
	KListView	*optoutlv, *optinlv;

	QButtonGroup	*faceGroup; // right
	QRadioButton	*rbadmonly, *rbprefadm, *rbprefusr, *rbusronly;

	KComboBox	*usercombo; // right below
	QPushButton	*userbutton;
	QPushButton	*rstuserbutton;

	QString		m_userPixDir;
	QString		m_defaultText;
	QStringList	hiddenUsers, selectedUsers;
	QString		defminuid, defmaxuid;

	bool		m_notFirst;
};

#endif


