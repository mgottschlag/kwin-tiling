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

#ifndef LIRCCLIENT_H
#define LIRCCLIENT_H

#include <QObject>
#include <QMap>
#include <QStringList>

class QLocalSocket;
class QSocketNotifier;

/**
  * @author Malte Starostik
  * @author Gav Wood
  * @author Michael Zanetti
  */

class LircClient: public QObject
{
    Q_OBJECT

private:
    QLocalSocket *theSocket;
    QSocketNotifier *theNotifier;
    QMap<QString, QStringList> theRemotes;
    
    void updateRemotes();
    void sendCommand(const QString &command);
    const QString readLine();

private slots:
    void slotRead();
    void slotClosed();

signals:
    /**
     * Emitted when the list of controls / buttons was changed by lircd (SIGHUP)
     */
     void newRemoteList(const QStringList &remoteList);

    /**
     * Emitted when a IR command was received
     *
     * The arguments are the name of the remote control used,
     * the name of the button pressed and the repeat counter.
     *
     * The signal is emitted repeatedly as long as the button
     * on the remote control remains pressed.
     * The repeat counter starts with 0 and increases
     * every time this signal is emitted.
     */
    void commandReceived(const QString &remote, const QString &button, int repeatCounter);

    /**
     * Emitted when the Lirc connection is closed.
     */
    void connectionClosed();

public:
    /**
     * Get the instance,
     *
     * @returns Instance to the LircClient
     */
    static LircClient *self();
    
    /**
     * Query status of connection.
     *
     * @returns true if connected to lircd.
     */
    bool isConnected() const;

    /**
     * Retrieve list of remote controls.
     *
     * @returns said list.
     */
    const QStringList remotes() const;

    /**
     * Retrieve list of buttons of a praticular remote control.
     *
     * @returns said list.
     */
    const QStringList buttons(const QString &theRemote) const;

    /**
     * Connects to lirc.
     *
     * @returns true if connection is made.
     */
    bool connectToLirc();

    LircClient();

    ~LircClient();
};

#endif
