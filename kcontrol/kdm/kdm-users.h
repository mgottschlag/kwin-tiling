/*
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KDMUSERS_H__
#define __KDMUSERS_H__

#include <QWidget>

class K3ListView;
class Q3ListViewItem;
class KComboBox;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QStackedWidget;

class KDMUsersWidget : public QWidget {
	Q_OBJECT

  public:
	KDMUsersWidget( QWidget *parent = 0 );

	void load();
	void save();
	void defaults();
	void makeReadOnly();

	bool eventFilter( QObject *o, QEvent *e );

  public Q_SLOTS:
	void slotClearUsers();
	void slotAddUsers( const QMap<QString,int> & );
	void slotDelUsers( const QMap<QString,int> & );

  Q_SIGNALS:
	void changed();
	void setMinMaxUID( int, int );

  private Q_SLOTS:
	void slotMinMaxChanged();
	void slotShowOpts();
	void slotUpdateOptIn( Q3ListViewItem *item );
	void slotUpdateOptOut( Q3ListViewItem *item );
	void slotUserSelected();
	void slotUnsetUserPix();
	void slotFaceOpts();
	void slotUserButtonClicked();

  private:
	void updateOptList( Q3ListViewItem *item, QStringList &list );
	void userButtonDropEvent( QDropEvent *e );
	void changeUserPix( const QString & );

	QGroupBox *minGroup; // top left
	QLineEdit *leminuid, *lemaxuid;

	QGroupBox *usrGroup; // right below
	QCheckBox *cbshowlist, *cbcomplete, *cbinverted, *cbusrsrt;

	QLabel *s_label; // middle
	QStackedWidget *wstack;
	K3ListView *optoutlv, *optinlv;

	QGroupBox *faceGroup; // right
	QRadioButton *rbadmonly, *rbprefadm, *rbprefusr, *rbusronly;

	KComboBox *usercombo; // right below
	QPushButton *userbutton;
	QPushButton *rstuserbutton;

	QString m_userPixDir;
	QString m_defaultText;
	QStringList hiddenUsers, selectedUsers;
	QString defminuid, defmaxuid;

	bool m_notFirst;
};

#endif


