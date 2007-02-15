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

#include "SuProc.h"
#include "KfiConstants.h"
#include <QFile>
#include <kprocess.h>
#include <unistd.h>
#include "config.h"

namespace KFI
{

CSuProc::CSuProc(QByteArray &sock, QString &passwd)
#ifdef KFI_SUDO_XTERM
       : itsPasswd(passwd)
#else
       : itsProc(KFI_SYS_USER),
         itsPasswd(passwd)
#endif
{
#ifdef KFI_SUDO_XTERM
    itsProc << "xterm" << "-e" << "sudo" 
            << KDE_DATADIR"/"KFI_NAME"/bin/kio_fonts_helper"
            << sock
            << QString().setNum(getuid()).latin1();
#else
    QByteArray cmd(KDE_DATADIR"/"KFI_NAME"/bin/kio_fonts_helper");

    cmd+=' ';
    cmd+=QFile::encodeName(KProcess::quote(sock));
    cmd+=' ';
    cmd+=QString().setNum(getuid()).latin1();
    itsProc.setCommand(cmd);
#endif
}

void CSuProc::run()
{
#ifdef KFI_SUDO_XTERM
    itsProc.start(KProcess::Block, KProcess::NoCommunication);
#else
    itsProc.exec(itsPasswd.toLocal8Bit());
#endif
}

}

