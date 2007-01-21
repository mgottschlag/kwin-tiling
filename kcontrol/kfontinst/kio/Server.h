#ifndef __SERVER_H__
#define __SERVER_H__

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

#include <QByteArray>

namespace KFI
{

class CSocket;

class CServer
{
    public:

    CServer();
    ~CServer()                  { close(); }

    bool         open();
    void         close();
    bool         isOpen() const { return itsOpen; }
    CSocket *    waitForClient(int timeout=5);
    QByteArray & name()         { return itsName; }

    private:

    int        itsFd;
    QByteArray itsName;
    bool       itsOpen;
};

}

#endif
