    /*

    xconsole widget for KDM

    Copyright (C) 2002 Oswald Buddenhagen <ossi@kde.org>


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

#ifndef KCONSOLE_H
#define KCONSOLE_H

#include <qtextedit.h>

class QSocketNotifier;
class KPty;

class KConsole : public QTextEdit {
    Q_OBJECT
    typedef QTextEdit inherited;

public:
    KConsole( QWidget *_parent = 0, const QString &src = QString::null );
    ~KConsole();

private slots:
    void slotData();

private:
    int OpenConsole();
    void CloseConsole();

    KPty *pty;
    QSocketNotifier *notifier;
    QString source, leftover;
    int fd;
};

#endif // KCONSOLE_H
