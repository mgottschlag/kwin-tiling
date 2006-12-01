#ifndef __UPDATE_DIALOG_H__
#define __UPDATE_DIALOG_H__

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

#include <QPixmap>
#include <kdialog.h>

class QTimer;
class QLabel;

namespace KFI
{

class CUpdateDialog : public KDialog
{
    Q_OBJECT

    static const int constNumIcons=8;

    public:

    CUpdateDialog(QWidget *parent, int xid=0);

    void start();
    void stop();

    private Q_SLOTS:

    void increment();
    void closeEvent(QCloseEvent *e);

    private:

    QLabel  *itsPixmapLabel;
    QTimer  *itsTimer;
    int     itsCount;
    QPixmap itsIcons[constNumIcons];
};

}

#endif
