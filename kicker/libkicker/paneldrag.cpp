/*****************************************************************
Copyright (c) 2004 Aaron J. Seigo <aseigo@kde.org>
              2004 Stephen Depooter <sbdep@woot.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <sys/types.h>
#include <unistd.h>

#include <qbuffer.h>

#include "paneldrag.h"

#define PANELDRAG_BUFSIZE sizeof(BaseContainer*) + sizeof(pid_t)

PanelDrag::PanelDrag(BaseContainer* container, QWidget* dragSource)
    : Q3DragObject(dragSource, 0)
{
    pid_t source_pid = getpid();

    a.resize(PANELDRAG_BUFSIZE);
    memcpy(a.data(), &container, sizeof(BaseContainer*));
    memcpy(a.data() + sizeof(BaseContainer*), &source_pid, sizeof(pid_t));
}

PanelDrag::~PanelDrag()
{
}

bool PanelDrag::decode(const QMimeSource* e, BaseContainer** container)
{
    QByteArray a = e->encodedData("application/basecontainerptr");

    if (a.size() != PANELDRAG_BUFSIZE)
    {
        return false;
    }

    pid_t target_pid = getpid();
    pid_t source_pid;
    memcpy(&source_pid, a.data() + sizeof(QObject*), sizeof(pid_t));

    if (source_pid == target_pid)
    {
        memcpy(container, a.data(), sizeof(QObject*));
        return true;
    }

    return false;
}

bool PanelDrag::canDecode(const QMimeSource *e)
{
    if (!e->provides("application/basecontainerptr"))
    {
        return false;
    }

    QByteArray a = e->encodedData("application/basecontainerptr");
    if (a.size() != PANELDRAG_BUFSIZE)
    {
        return false;
    }

/*    pid_t target_pid = getpid();
    pid_t source_pid;
    memcpy(&source_pid, a.data() + sizeof(void*), sizeof(pid_t));

    if (source_pid != target_pid)
    {
        return true;
    } */

    return true;
}

QByteArray PanelDrag::encodedData(const char * mimeType) const
{
    if (QString("application/basecontainerptr") == mimeType &&
        a.size() == PANELDRAG_BUFSIZE)
    {
        return a;
    }

    return QByteArray();
}

const char * PanelDrag::format(int i) const
{
    if (i == 0)
    {
        return "application/basecontainerptr";
    }

    return 0;
}


AppletInfoDrag::AppletInfoDrag(const AppletInfo& info, QWidget *dragSource)
    : Q3DragObject(dragSource, 0)
{
    QDataStream s(&a, QIODevice::WriteOnly);

    s.setVersion(QDataStream::Qt_3_1);
    s << info.desktopFile() << info.configFile() << info.type();
}

AppletInfoDrag::~AppletInfoDrag()
{
}

const char * AppletInfoDrag::format(int i) const
{
    if (i == 0)
    {
        return "application/appletinfo";
    }

    return 0;
}

QByteArray AppletInfoDrag::encodedData(const char* mimeType) const
{
    if (QString("application/appletinfo") == mimeType)
    {
        return a;
    }

    return QByteArray();
}

bool AppletInfoDrag::canDecode(const QMimeSource * e)
{
    if (!e->provides("application/appletinfo"))
    {
        return false;
    }

    return true;
}

bool AppletInfoDrag::decode(const QMimeSource* e, AppletInfo& container)
{
    QByteArray a = e->encodedData("application/appletinfo");

    if (a.isEmpty())
    {
        return false;
    }

    QBuffer buff(&a);
    buff.open(QIODevice::ReadOnly);
    QDataStream s(&buff);

    QString desktopFile;
    QString configFile;
    int type;
    s >> desktopFile >> configFile >> type;
    AppletInfo info(desktopFile, configFile, (AppletInfo::AppletType)type);
    container = info;
    return true;
}

