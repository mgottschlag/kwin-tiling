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

//
// This file contains code taken from kdelibs/kdesu/client.cpp
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <QVariant>
#include <QByteArray>
#include "Socket.h"
#include "config.h"

#define KFI_TEST

#ifdef KFI_TEST
#include <iostream>
using namespace std;
#define KFI_WARNING_STREAM cout
#define KDE_struct_stat    struct stat
#define KDE_lstat          ::lstat
#define k_lineinfo         "[" << __FILE__ << ":" << __LINE__ << "] "
#else
#include <config.h>
#include <kdebug.h>
#define KFI_WARNING_STREAM kWarning()
#endif

#ifndef SUN_LEN
#define SUN_LEN(ptr) ((socklen_t) (((struct sockaddr_un *) 0)->sun_path) \
                 + strlen ((ptr)->sun_path))
#endif

namespace KFI
{

CSocket::CSocket(int fd)
       :  itsFd(fd)
{
}

CSocket::~CSocket()
{
    if (itsFd>=0)
        ::close(itsFd);
}

bool CSocket::read(QVariant &var, int timeout)
{
    if(itsFd<0)
        return false;

    int type;

    if(readBlock((char *)&type, sizeof(type), timeout))
    {
        switch(type)
        {
            case QVariant::Int:
            {
                int val;
                if(readBlock((char *)(&val), sizeof(int), timeout))
                {
                    var=QVariant(val);
                    return true;
                }
                break;
            }
            case QVariant::Bool:
            {
                unsigned char val;
                if(readBlock((char *)(&val), sizeof(unsigned char), timeout))
                {
                    var=QVariant((bool)val);
                    return true;
                }
                break;
            }
            case QVariant::String:
            {
                int len;

                if(readBlock((char *)&len, sizeof(unsigned int), timeout))
                {
                    QByteArray data(len, '\0');

                    if(readBlock(data.data(), len, timeout))
                    {
                        var=QVariant(QString::fromUtf8(data));
                        return true;
                    }
                }
                break;
            }
            default:
                ;
        }
    }
    return false;
}

bool CSocket::read(QString &str, int timeout)
{
    QVariant var;

    if(read(var, timeout) && QVariant::String==var.type())
    {
        str=var.toString();
        return true;
    }

    return false;
}

bool CSocket::read(int &i, int timeout)
{
    QVariant var;

    if(read(var, timeout) && QVariant::Int==var.type())
    {
        i=var.toInt();
        return true;
    }

    return false;
}

bool CSocket::read(bool &b, int timeout)
{
    QVariant var;

    if(read(var, timeout) && QVariant::Bool==var.type())
    {
        b=var.toBool();
        return true;
    }

    return false;
}

bool CSocket::write(const QVariant &var, int timeout)
{
    if(itsFd<0)
        return false;

    int type(var.type());

    switch(type)
    {
        case QVariant::Int:
        {
            int val(var.toInt());
            return writeBlock((const char *)(&type), sizeof(int), timeout) &&
                   writeBlock((const char *)(&val), sizeof(int), timeout);
        }
        case QVariant::Bool:
        {
            unsigned char val(var.toBool());

            return writeBlock((const char *)(&type), sizeof(int), timeout) &&
                   writeBlock((const char *)(&val), sizeof(unsigned char), timeout);
            break;
        }
        case QVariant::String:
        {
            QByteArray data(var.toString().toUtf8());
            int        len(data.size());

            return writeBlock((const char *)(&type), sizeof(int), timeout) &&
                   writeBlock((const char *)(&len), sizeof(int), timeout) &&
                   writeBlock(data.constData(), len, timeout);
            break;
        }
        default:
            ;
    }

    return false;
}

bool CSocket::connectToServer(const QByteArray &sock, unsigned int socketUid)
{
    if (itsFd >= 0)
        ::close(itsFd);
    itsFd=-1;
    if (access(sock, R_OK|W_OK))
        return false;

    itsFd =::socket(PF_UNIX, SOCK_STREAM, 0);
    if (itsFd < 0)
    {
        KFI_WARNING_STREAM << k_lineinfo << "socket(): " << errno << endl;
        return false;
    }
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock);

    if (::connect(itsFd, (struct sockaddr *) &addr, SUN_LEN(&addr)) < 0)
    {
        KFI_WARNING_STREAM << k_lineinfo << "connect():" << errno << endl;
        ::close(itsFd);
        itsFd = -1;
        return false;
    }

#if !defined(SO_PEERCRED) || !defined(HAVE_STRUCT_UCRED)
# if defined(HAVE_GETPEEREID)
    uid_t euid;
    gid_t egid;
    // Security: if socket exists, we must own it
    if (getpeereid(itsFd, &euid, &egid) == 0 && euid != getuid())
    {
        KFI_WARNING_STREAM << "socket not owned by me! socket uid = " << euid << endl;
        ::close(itsFd);
        itsFd = -1;
        return false;
    }
# else
#  ifdef __GNUC__
#   warning "Using sloppy security checks"
#  endif
    // We check the owner of the socket after we have connected.
    // If the socket was somehow not ours an attacker will be able
    // to delete it after we connect but shouldn't be able to
    // create a socket that is owned by us.
    KDE_struct_stat s;
    if (KDE_lstat(sock, &s)!=0)
    {
        KFI_WARNING_STREAM << "stat failed (" << sock.data() << ")" << endl;
        ::close(itsFd);
        itsFd = -1;
        return false;
    }
    if (s.st_uid != socketUid)
    {
        KFI_WARNING_STREAM << "socket not owned by " << socketUid << "! socket uid = " << s.st_uid << endl;
        ::close(itsFd);
        itsFd = -1;
        return false;
    }
    if (!S_ISSOCK(s.st_mode))
    {
        KFI_WARNING_STREAM << "socket is not a socket (" << sock.data() << ")" << endl;
        ::close(itsFd);
        itsFd = -1;
        return false;
    }
# endif
#else
    struct ucred cred;
    socklen_t siz = sizeof(cred);

    // Security: if socket exists, we must own it
    if (getsockopt(itsFd, SOL_SOCKET, SO_PEERCRED, &cred, &siz) == 0 && cred.uid != socketUid)
    {
        KFI_WARNING_STREAM << "socket not owned by " << socketUid << "! socket uid = " << cred.uid << endl;
        ::close(itsFd);
        itsFd = -1;
        return false;
    }
#endif

    return true;
}

bool CSocket::readBlock(char *data, int size, int timeout)
{
    int bytesToRead=size;

    do
    {
        if(waitForReadyRead(timeout))
        {
            int bytesRead=::read(itsFd, &data[size-bytesToRead], bytesToRead);

            if (bytesRead>0)
                bytesToRead-=bytesRead;
            else
                return false;
        }
        else
            return false;
    }
    while(bytesToRead>0);

    return true;
}

bool CSocket::writeBlock(const char *data, int size, int timeout)
{
    int bytesToWrite=size;

    do
    {
        if(waitForReadyWrite(timeout))
        {
            int bytesWritten=::write(itsFd, (char *)&data[size-bytesToWrite], bytesToWrite);

            if (bytesWritten>0)
                bytesToWrite-=bytesWritten;
            else
                return false;
        }
        else
            return false;
    }
    while(bytesToWrite>0);

    return true;
}

bool CSocket::waitForReadyRead(int timeout)
{
    fd_set         fdSet;
    struct timeval tv;

    FD_ZERO(&fdSet);
    FD_SET(itsFd, &fdSet);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    for(;;)
        if(select(itsFd + 1, &fdSet, NULL, NULL, -1==timeout ? NULL : &tv)<0)
        {
            if(errno == EINTR)
                continue;
            else
                return false;
        }
        else
            return FD_ISSET(itsFd, &fdSet);

    return false;
}

bool CSocket::waitForReadyWrite(int timeout)
{
    fd_set         fdSet;
    struct timeval tv;

    FD_ZERO(&fdSet);
    FD_SET(itsFd, &fdSet);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    for(;;)
        if(select(itsFd + 1, NULL, &fdSet, NULL, -1==timeout ? NULL : &tv)<0)
        {
            if(errno == EINTR)
                continue;
            else
                return false;
        }
        else
            return FD_ISSET(itsFd, &fdSet);

    return false;
}

}
