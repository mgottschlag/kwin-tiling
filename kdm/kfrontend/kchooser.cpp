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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kchooser.h"
#include "kconsole.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <klocale.h>

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSocketNotifier>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <stdlib.h> // for free()

#define CHOOSE_TO 20 // inactivity timeout for reserve displays
#define SECONDS 1000 // reduce to 100 to speed up testing


class ChooserListViewItem : public QTreeWidgetItem {
  public:
    ChooserListViewItem(QTreeWidget *parent, int _id, const QString &nam, const QString &sts)
        : QTreeWidgetItem(parent, QStringList() << nam << sts) { id = _id; }

    int id;
};


ChooserDlg::ChooserDlg()
    : inherited()
{
    completeMenu(LOGIN_REMOTE_ONLY, ex_greet, i18nc("@action:inmenu", "&Local Login"), Qt::ALT + Qt::Key_L);

    QBoxLayout *vbox = new QVBoxLayout(this);

    QLabel *title = new QLabel(i18n("XDMCP Host Menu"), this);
    title->setAlignment(Qt::AlignCenter);
    vbox->addWidget(title);

    host_view = new QTreeWidget(this);
    host_view->setRootIsDecorated(false);
    host_view->setUniformRowHeights(true);
    host_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    host_view->setColumnCount(2);
    host_view->setHeaderLabels(QStringList()
        << i18nc("@title:column", "Hostname")
        << i18nc("@title:column ... of named host", "Status"));
    host_view->setColumnWidth(0, fontMetrics().width("login.domain.com"));
    host_view->setMinimumWidth(fontMetrics().width("login.domain.com Display not authorized to connect this server"));
    host_view->setAllColumnsShowFocus(true);
    vbox->addWidget(host_view);

    iline = new QLineEdit(this);
    iline->setEnabled(true);
    QLabel *itxt = new QLabel(i18nc("XDMCP server", "Hos&t:"), this);
    itxt->setBuddy(iline);
    QPushButton *addButton = new QPushButton(i18nc("@action:button", "A&dd"), this);
    connect(addButton, SIGNAL(clicked()), SLOT(addHostname()));
    QBoxLayout *hibox = new QHBoxLayout();
    vbox->addLayout(hibox);
    hibox->addWidget(itxt);
    hibox->addWidget(iline);
    hibox->addWidget(addButton);

    // Buttons
    QPushButton *acceptButton = new QPushButton(i18nc("@action:button", "&Accept"), this);
    acceptButton->setDefault(true);
    QPushButton *pingButton = new QPushButton(i18nc("@action:button", "&Refresh"), this);

    QBoxLayout *hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    hbox->setSpacing(20);
    hbox->addWidget(acceptButton);
    hbox->addWidget(pingButton);
    hbox->addStretch(1);

    if (optMenu) {
        QPushButton *menuButton = new QPushButton(i18nc("@action:button", "&Menu"), this);
        menuButton->setMenu(optMenu);
        hbox->addWidget(menuButton);
        hbox->addStretch(1);
    }

//    QPushButton *helpButton = new QPushButton(i18nc("@action:button", "&Help"), this);
//    hbox->addWidget(helpButton);

#ifdef WITH_KDM_XCONSOLE
    if (consoleView)
        vbox->addWidget(consoleView);
#endif

    sn = new QSocketNotifier(rfd, QSocketNotifier::Read, this);
    connect(sn, SIGNAL(activated(int)), SLOT(slotReadPipe()));

    connect(pingButton, SIGNAL(clicked()), SLOT(pingHosts()));
    connect(acceptButton, SIGNAL(clicked()), SLOT(accept()));
//    connect(helpButton, SIGNAL(clicked()), SLOT(slotHelp()));
    connect(host_view, SIGNAL(itemActivated(QTreeWidgetItem*,int)), SLOT(accept()));

    if (_isReserve) {
        connect(&timer, SIGNAL(timeout()), SLOT(slotTimeout()));
        connect(qApp, SIGNAL(activity()), SLOT(slotActivity()));
        slotActivity();
    }
}

void
ChooserDlg::slotTimeout()
{
    ::exit(EX_RESERVE);
}

void
ChooserDlg::slotActivity()
{
    timer.start(CHOOSE_TO * SECONDS);
}

/*
void ChooserDlg::slotHelp()
{
    KMessageBox::information(0,
                              i18n("Choose a host, you want to work on,\n"
                                   "in the list or add one.\n\n"
                                   "After this box, you must press cancel\n"
                                   "in the Host Menu to enter a host. :("));
    iline->setFocus();
}
*/

void ChooserDlg::addHostname()
{
    if (!iline->text().isEmpty()) {
        gSendInt(G_Ch_RegisterHost);
        gSendStr(iline->text().toLatin1());
        iline->clear();
    }
}

void ChooserDlg::pingHosts()
{
    gSendInt(G_Ch_Refresh);
}

void ChooserDlg::accept()
{
    if (focusWidget() == iline) {
        if (!iline->text().isEmpty()) {
            gSendInt(G_Ch_DirectChoice);
            gSendStr(iline->text().toLatin1());
            iline->clear();
        }
        return;
    } else /*if (focusWidget() == host_view)*/ {
        QTreeWidgetItem *item = host_view->currentItem();
        if (item) {
            gSendInt(G_Ready);
            gSendInt(((ChooserListViewItem *)item)->id);
            ::exit(EX_NORMAL);
        }
    }
}

void ChooserDlg::reject()
{
}

QString ChooserDlg::recvStr()
{
    char *arr = gRecvStr();
    if (arr) {
        QString str = QString::fromLatin1(arr);
        free(arr);
        return str;
    } else
        return i18nc("hostname or status", "<unknown>"); //krazy:exclude=i18ncheckarg
}

ChooserListViewItem *ChooserDlg::findItem(int id)
{
    for (int i = 0, rc = host_view->model()->rowCount(); i < rc; i++) {
        ChooserListViewItem *itm = static_cast<ChooserListViewItem *>(
            host_view->topLevelItem(i));
        if (itm->id == id)
            return itm;
    }
    return 0;
}

void ChooserDlg::slotReadPipe()
{
    int id;
    QString nam, sts;

    int cmd = gRecvInt();
    switch (cmd) {
    case G_Ch_AddHost:
    case G_Ch_ChangeHost:
        id = gRecvInt();
        nam = recvStr();
        sts = recvStr();
        gRecvInt(); /* swallow willing for now */
        if (cmd == G_Ch_AddHost) {
            new ChooserListViewItem(host_view, id, nam, sts);
        } else {
            QTreeWidgetItem *itm = findItem(id);
            itm->setText(0, nam);
            itm->setText(1, sts);
        }
        break;
    case G_Ch_RemoveHost:
        delete findItem(gRecvInt());
        break;
    case G_Ch_BadHost:
        KFMsgBox::box(this, QMessageBox::Warning, i18n("Unknown host %1", recvStr()));
        break;
    case G_Ch_Exit:
        done(ex_exit);
        break;
    default: /* XXX huuh ...? */
        break;
    }
}

#include "kchooser.moc"
