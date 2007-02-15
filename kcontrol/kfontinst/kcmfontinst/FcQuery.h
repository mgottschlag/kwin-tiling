#ifndef __FC_QUERY_H__
#define __FC_QUERY_H__

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

#include <QObject>
#include <QByteArray>

class KProcess;

namespace KFI
{

class CFcQuery : public QObject
{
    Q_OBJECT

    public:

    CFcQuery(QObject *parent) : QObject(parent), itsProc(NULL) { }
    ~CFcQuery();

    void run(const QString &query);

    const QString & file() const    { return itsFile; }

    private Q_SLOTS:

    void procExited();
    void data(KProcess *proc, char *buffer, int buflen);

    Q_SIGNALS:

    void finished();

    private:

    KProcess   *itsProc;
    QByteArray itsBuffer;
    QString    itsFile;
};

}

#endif

