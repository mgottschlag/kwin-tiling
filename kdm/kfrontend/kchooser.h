    /*

    chooser widget for KDM
    $Id$

    Copyright (C) 2002 Oswald Buddenhagen <ossi@kde.org>
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#ifndef KCHOOSER_H
#define KCHOOSER_H

#include "kfdialog.h"

#include <qlistview.h>
#include <qlineedit.h>

/* should be in kgreet_local.h */
#define ex_exit		1
#define ex_greet	2
#define ex_choose	3

class QSocketNotifier;
class QPopupMenu;

class ChooserDlg : public FDialog {
    Q_OBJECT
    typedef FDialog inherited;

  public:
    ChooserDlg(QWidget * parent = 0, const char *name = 0);

  public slots:
    void slotReadPipe();
    void addHostname();
//    void slotHelp();
    void pingHosts();
    void accept();
    void reject();
    void quit_button_clicked();
    void local_button_clicked();
    void console_button_clicked();
    void shutdown_button_clicked();

  private:
    void Inserten( QPopupMenu *mnu, const QString& txt, const char *member );

    QString recvStr ();
    QListViewItem *findItem (int id);

    QListView *host_view;
    QLineEdit *iline;
    QSocketNotifier *sn;
};

#endif /* KCHOOSER_H */
