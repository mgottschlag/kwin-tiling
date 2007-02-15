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

#include "FcQuery.h"
#include <QStringList>
#include <kprocess.h>
#include <stdio.h>
namespace KFI
{

CFcQuery::~CFcQuery()
{
    delete itsProc;
}

void CFcQuery::run(const QString &query)
{
    if(itsProc)
        delete itsProc;

    itsFile=QString();
    itsBuffer=QByteArray();
    itsProc=new KProcess;
    *itsProc << "fc-match" << "-v" << query;
    connect(itsProc, SIGNAL(processExited(KProcess *)), SLOT(procExited()));
    connect(itsProc, SIGNAL(receivedStdout(KProcess *, char *, int)),
            SLOT(data(KProcess *, char *, int)));
    itsProc->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

void CFcQuery::procExited()
{
    QStringList results(QString::fromUtf8(itsBuffer, itsBuffer.length()).split('\n'));

    if(results.size())
    {
        QStringList::ConstIterator it(results.begin()),
                                   end(results.end());

        for(; it!=end; ++it)
        {
            QString line((*it).trimmed());

            if(0==line.indexOf("file:"))  // file: "/wibble/wobble.ttf"(s)
            {
                int endPos=line.indexOf("\"(s)");

                if(-1!=endPos)
                    itsFile=line.mid(7, endPos-7);
            }
        }
    }
    emit finished();
}

void CFcQuery::data(KProcess *, char *buffer, int buflen)
{
    int current=itsBuffer.size();

    itsBuffer.resize(itsBuffer.size()+buflen);
    memcpy(&(itsBuffer.data()[current]), buffer, buflen);
}

}

#include "FcQuery.moc"
