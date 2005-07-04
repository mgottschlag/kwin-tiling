/*

chooser widget for KDM

Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
based on the chooser (C) 1999 by Harald Hoyer <Harald.Hoyer@RedHat.de>

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
Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KCHOOSER_H
#define KCHOOSER_H

#include "kgdialog.h"

class QSocketNotifier;
class QPopupMenu;
class QLineEdit;
class QListView;
class QListViewItem;

class ChooserDlg : public KGDialog {
	Q_OBJECT
	typedef KGDialog inherited;

  public:
	ChooserDlg();

  public slots:
	void slotReadPipe();
	void addHostname();
//	void slotHelp();
	void pingHosts();
	void accept();
	void reject();

  private:
	QString recvStr();
	QListViewItem *findItem( int id );

	QListView *host_view;
	QLineEdit *iline;
	QSocketNotifier *sn;
};

#endif /* KCHOOSER_H */
