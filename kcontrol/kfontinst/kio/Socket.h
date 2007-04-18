#ifndef __SOCKET_H__
#define __SOCKET_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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

class QVariant;
class QByteArray;
class QString;

namespace KFI
{

class CSocket
{
    public:

    static const int constTimeout=1;

    CSocket(int fd=-1);
    ~CSocket();

    bool read(QVariant &var, int timeout=constTimeout);
    bool read(QString &str, int timeout=constTimeout);
    bool read(int &i, int timeout=constTimeout);
    bool read(qulonglong &i, int timeout=constTimeout);
    bool read(bool &b, int timeout=constTimeout);
    bool write(const QVariant &var, int timeout=constTimeout);
    bool connectToServer(const QByteArray &sock, unsigned int socketUid);

    private:

    bool readBlock(char *data, int size, int timeout);
    bool writeBlock(const char *data, int size, int timeout);
    bool waitForReadyRead(int timeout);
    bool waitForReadyWrite(int timeout);

    protected:

    int itsFd;
};

}

#endif
