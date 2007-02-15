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

//
// This file contains code taken from kdebase/runtime/kdesu/kdesud/kdesud.cpp
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <config.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <ksocks.h>
#include "Server.h"
#include "Socket.h"
#include "Misc.h"

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((socklen_t) (((struct sockaddr_un *) 0)->sun_path) \
                 + strlen ((ptr)->sun_path))
#endif

typedef unsigned ksocklen_t;

namespace KFI
{

#define KFI_SOCKET_SUFFIX     "kio_fonts_"
#define KFI_SOCKET_SUFFIX_LEN 10
CServer::CServer()
       : itsFd(-1),
         itsOpen(false)
{
    QString sockName;
    int     thisPid(getpid());

    sockName.sprintf(KFI_SOCKET_SUFFIX"%d", thisPid);

    QString nameAndPath(KStandardDirs::locateLocal("socket", sockName));

    itsName=QFile::encodeName(nameAndPath);

    // Remove any old sockets that are no longer active
    QDir        dir(Misc::getDir(nameAndPath));
    QStringList nameFilters;

    nameFilters.append(KFI_SOCKET_SUFFIX"*");
    dir.setFilter(QDir::System|QDir::CaseSensitive);
    dir.setNameFilters(nameFilters);

    QFileInfoList list(dir.entryInfoList());

    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo(list.at(i));

        if(fileInfo.ownerId()==getuid() && fileInfo.groupId()==getgid())
        {
            int pid=fileInfo.fileName().mid(KFI_SOCKET_SUFFIX_LEN).toInt();

            if(0!=pid && pid!=thisPid && 0!=kill(pid, 0))
                ::unlink(QFile::encodeName(fileInfo.absoluteFilePath()));
        }
    }
}

bool CServer::open()
{
    if(itsOpen)
        return true;

    ksocklen_t  addrlen;
    struct stat s;

    int stat_err=lstat(itsName, &s);
    if(!stat_err && S_ISLNK(s.st_mode))
    {
        kWarning() << "Someone is running a symlink attack on you" << endl;
        if(unlink(itsName))
        {
            kWarning() << "Could not delete symlink" << endl;
            return false;
        }
    }

    if (!access(itsName, R_OK|W_OK))
    {
        kWarning() << "stale socket exists" << endl;
        if (unlink(itsName))
        {
            kWarning() << "Could not delete stale socket" << endl;
            return false;
        }
    }

    itsFd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (itsFd < 0)
    {
        kError() << "socket(): " << strerror(errno) << endl;
        return false;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, itsName, sizeof(addr.sun_path)-1);
    addr.sun_path[sizeof(addr.sun_path)-1] = '\000';
    addrlen = SUN_LEN(&addr);
    if (bind(itsFd, (struct sockaddr *)&addr, addrlen) < 0)
    {
        kError() << "bind(): " << strerror(errno) << endl;
        return false;
    }

    struct linger lin;
    lin.l_onoff = lin.l_linger = 0;
    if (setsockopt(itsFd, SOL_SOCKET, SO_LINGER, (char *) &lin,
                   sizeof(linger)) < 0)
    {
        kError() << "setsockopt(SO_LINGER): " << strerror(errno) << endl;
        return false;
    }

    int opt = 1;
    if (setsockopt(itsFd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
                   sizeof(opt)) < 0)
    {
        kError() << "setsockopt(SO_REUSEADDR): " << strerror(errno) << endl;
        return false;
    }
    opt = 1;
    if (setsockopt(itsFd, SOL_SOCKET, SO_KEEPALIVE, (char *) &opt,
                   sizeof(opt)) < 0)
    {
        kError() << "setsockopt(SO_KEEPALIVE): " << strerror(errno) << endl;
        return false;
    }
    chmod(itsName, 0600);
    if (listen(itsFd, 1) < 0)
    {
        kError() << "listen(): " << strerror(errno) << endl;
        return false;
    }

    itsOpen=true;
    return true;
}

void CServer::close()
{
    if(itsFd>=0)
        ::close(itsFd);
    if(!itsName.isEmpty())
        unlink(itsName);
    itsOpen=false;
}

CSocket * CServer::waitForClient(int timeout)
{
    if(itsOpen)
        for(;;)
        {
            fd_set fdSet;
            struct timeval tv;

            tv.tv_sec = timeout;
            tv.tv_usec = 0;

            FD_ZERO(&fdSet);
            FD_SET(itsFd, &fdSet);

            if(select(itsFd + 1, &fdSet, NULL, NULL, &tv)<0)
                if (errno == EINTR)
                    continue;
                else
                {
                    kError() << "select(): " << strerror(errno) << endl;
                    break;
                }

            if(FD_ISSET(itsFd, &fdSet))
            {
                struct sockaddr_un clientname;
                ksocklen_t         addrlen(64);
                int                fd(-1);

                if((fd=accept(itsFd, (struct sockaddr *) &clientname, &addrlen))>=0)
                    return new CSocket(fd);
            }
            else
            {
                kError() << "select timeout" << endl;
                break;
            }
        }

    return NULL;
}

}


