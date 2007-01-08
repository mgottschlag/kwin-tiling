/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "UpdateDialog.h"
#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <QLabel>
#include <QTimer>
#include <QCloseEvent>
#include <QBoxLayout>
#include <QX11Info>
#include <X11/Xlib.h>
#include <fixx11h.h>

#include <stdio.h>
namespace KFI
{

CUpdateDialog::CUpdateDialog(QWidget *parent, int xid)
             : KDialog(parent)
{
    setModal(true);
    setCaption(i18n("Configuring"));
    setButtons(None);
    if(NULL==parent && 0!=xid)
        XSetTransientForHint(QX11Info::display(), winId(), xid);

    QFrame *page = new QFrame(this);
    setMainWidget(page);

    QBoxLayout *layout=new QBoxLayout(QBoxLayout::LeftToRight, page);
    layout->setMargin(KDialog::marginHint());
    layout->setSpacing(KDialog::spacingHint());
    itsPixmapLabel=new QLabel(page);

    for(int i=0; i<constNumIcons; ++i)
    {
        QString name;

        name.sprintf("font_cfg_update%d", i+1);
        itsIcons[i]=KIconLoader::global()->loadIcon(name, K3Icon::NoGroup, 48);
    }
    itsPixmapLabel->setPixmap(itsIcons[0]);
    layout->addWidget(itsPixmapLabel);
    layout->addWidget(new QLabel(i18n("Updating font configuration. Please wait..."), page));

    itsTimer=new QTimer(this);
    connect(itsTimer, SIGNAL(timeout()), SLOT(increment()));
}

void CUpdateDialog::start()
{
    itsCount=0;
    itsPixmapLabel->setPixmap(itsIcons[0]);
    itsTimer->start(1000/constNumIcons);
    show();
}

void CUpdateDialog::stop()
{
    itsTimer->stop();
    hide();
}

void CUpdateDialog::increment()
{
    if(++itsCount==constNumIcons)
        itsCount=0;

    // For some reason override of closeEvent is not working for Qt4 :-( TODO: Fix?
    if(isHidden())
        show();

    itsPixmapLabel->setPixmap(itsIcons[itsCount]);
}

void CUpdateDialog::closeEvent(QCloseEvent *)
{
}

}

#include "UpdateDialog.moc"
