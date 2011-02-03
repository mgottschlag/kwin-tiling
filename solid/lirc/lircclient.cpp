/*************************************************************************
 * Copyright            : (C) 2002 by Gav Wood <gav@kde.org>             *
 *                                                                       *
 * This program is free software; you can redistribute it and/or         *
 * modify it under the terms of the GNU General Public License as        *
 * published by the Free Software Foundation; either version 2 of        *
 * the License or (at your option) version 3 or any later version        *
 * accepted by the membership of KDE e.V. (or its successor approved     *
 * by the membership of KDE e.V.), which shall act as a proxy            *
 * defined in Section 14 of version 3 of the license.                    *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *************************************************************************/


/**
  * @author Gav Wood
  * @author Michael Zanetti
  */

#include "lircclient.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>

// #include <QWidget>

#include <QObject>
#include <QLocalSocket>
#include <QFile>

#include <kdebug.h>
#include <kglobal.h>

class LircClientPrivate
{
public:
     LircClient instance;
};

K_GLOBAL_STATIC(LircClientPrivate, theInstancePrivate)

LircClient::LircClient() : theSocket(0)
{
}

LircClient *LircClient::self() {
   return &theInstancePrivate->instance;
}

bool LircClient::connectToLirc()
{
    int sock = ::socket(PF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) return false;

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    // Try to open lircd socket for lirc >= 0.8.6
    strcpy(addr.sun_path, "/var/run/lirc/lircd");
    if (::connect(sock, (struct sockaddr *)(&addr), sizeof(addr)) == -1) {
        ::close(sock);
        // for lirc < 0.8.6
        sock = ::socket(PF_UNIX, SOCK_STREAM, 0);
        strcpy(addr.sun_path, "/dev/lircd");
        if (::connect(sock, (struct sockaddr *)(&addr), sizeof(addr)) == -1) {
            ::close(sock);
            // in case of mandrake...
            sock = ::socket(PF_UNIX, SOCK_STREAM, 0);
            strcpy(addr.sun_path, "/tmp/.lircd");
            if (::connect(sock, (struct sockaddr *)(&addr), sizeof(addr)) == -1) {
                ::close(sock);
                kDebug() << "no lircd socket found...";
                return false;
            } else {
                kDebug() << "Mandrake lircd socket found...";
            }
        } else {
            kDebug() << "lircd < 0.8.6 socket found...";
        }
    } else {
        kDebug() << "lircd >= 0.8.6 socket found...";
    }

    if(!theSocket){
        theSocket = new QLocalSocket();
    }
    theSocket->setSocketDescriptor(sock);
    kDebug() << "updating remotes";
    updateRemotes();
    kDebug() << "waiting for lirc";
    theSocket->waitForReadyRead();
    kDebug() << "reading...";
    slotRead();
    connect(theSocket, SIGNAL(readyRead()), SLOT(slotRead()));
    connect(theSocket, SIGNAL(disconnected()), SLOT(slotClosed()));
    return true;
}

LircClient::~LircClient()
{
    kDebug() << "deleting theSocket";
    delete theSocket;
}

void LircClient::slotClosed()
{
    kDebug() << "connection closed";
    theRemotes.clear();
    emit connectionClosed();
}

const QStringList LircClient::remotes() const
{
    return theRemotes.keys();
}

const QStringList LircClient::buttons(const QString &theRemote) const
{
    return theRemotes.value(theRemote);
}

void LircClient::slotRead()
{
    while (theSocket->bytesAvailable()) {
        QString line = readLine();
        if (line == "BEGIN") {
            // BEGIN
            // <command>
            // [SUCCESS|ERROR]
            // [DATA
            // n
            // n lines of data]
            // END
            line = readLine();
kDebug() << line;
            if (line == "SIGHUP") {
                // Configuration changed
                do line = readLine();
                while (!line.isEmpty() && line != "END");
                updateRemotes();
                theSocket->waitForReadyRead();
                slotRead();
                return;
            } else if (line == "LIST") {
                // remote control list
                if (readLine() != "SUCCESS" || readLine() != "DATA") {
                    do line = readLine();
                    while (!line.isEmpty() && line != "END");
                    return;
                }
                QStringList remotes;
                int count = readLine().toInt();
                for (int i = 0; i < count; ++i)
                    remotes.append(readLine());
kDebug() << remotes;
                do line = readLine();
                while (!line.isEmpty() && line != "END");
                if (line.isEmpty())
                    return; // abort on corrupt data
                for (QStringList::ConstIterator it = remotes.constBegin(); it != remotes.constEnd(); ++it) {
                    sendCommand("LIST " + *it);
                theSocket->waitForReadyRead();
                slotRead();
            }
                return;
            } else if (line.left(4) == "LIST") {
                // button list
                if (readLine() != "SUCCESS" || readLine() != "DATA") {
                    do line = readLine();
                    while (!line.isEmpty() && line != "END");
                    return;
                }
                QString remote = line.mid(5);
                QStringList buttons;
                int count = readLine().toInt();
                for (int i = 0; i < count; ++i) {
                    // <code> <name>
                    QString btn = readLine().mid(17);
                    if (btn.isNull()) break;
                    if (btn.startsWith('\'') && btn.endsWith('\''))
                        btn = btn.mid(1, btn.length() - 2);
                    buttons.append(btn);
                }
                theRemotes.insert(remote, buttons);
            }
            do line = readLine();
            while (!line.isEmpty() && line != "END");
            kDebug() << "Remotes read!";
            emit newRemoteList(theRemotes.keys());
        } else {
            // <code> <repeat> <button name> <remote control name>
            line.remove(0, 17); // strip code
            int pos = line.indexOf(' ');
            if (pos < 0) return;
            bool ok;
            int repeat = line.left(pos).toInt(&ok, 16);
            if (!ok) return;
            line.remove(0, pos + 1);

            pos = line.indexOf(' ');
            if (pos < 0) return;
            QString btn = line.left(pos);
            if (btn.startsWith('\'') && btn.endsWith('\''))
                btn = btn.mid(1, btn.length() - 2);
            line.remove(0, pos + 1);
            kDebug() << "Command received!";
            emit commandReceived(line, btn, repeat);
        }
    }
}

void LircClient::updateRemotes()
{
    theRemotes.clear();
    sendCommand("LIST");
}

bool LircClient::isConnected() const
{
    kDebug() << "theSocket" << theSocket;
    if (!theSocket) return false;
    kDebug()  << "state:" << theSocket->state();
    return theSocket->state() == QLocalSocket::ConnectedState;
}

const QString LircClient::readLine()
{
    if (!theSocket->canReadLine()) {
        theSocket->waitForReadyRead(500);
        if (!theSocket->canReadLine()) { // Still nothing :(
            return QString();
        }
    }
    QString line = theSocket->readLine();
    line.truncate(line.length() - 1);
    return line;
}

void LircClient::sendCommand(const QString &command)
{
    QByteArray cmd = QFile::encodeName(command + '\n');
    theSocket->write(cmd);
}


#include "lircclient.moc"
