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

#include "Helper.h"
#include "Misc.h"
#include "DisabledFonts.h"
#include "KfiConstants.h"
#include "Socket.h"
#include "kxftconfig.h"
#include <QFile>
#include <QDataStream>
#include <QVariant>
#include <kglobal.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <stdio.h>
#include <stdlib.h>

namespace KFI
{

class CHelper : public CSocket
{
    public:

    CHelper(char *sock, int uid) : itsSock(sock), itsUid(uid), itsKComponentData(KFI_NAME) { }

    int run();

    private:

    bool getFile(CDisabledFonts::TFont &font);
    bool disableFont();
    bool enableFont();
    bool deleteDisabledFont();
    bool addToFc();
    bool configureX();
    bool copyFile();
    bool moveFile();
    bool deleteFile();
    bool createDir();
    bool createAfm();

    private:

    const char     *itsSock;
    int            itsUid;
    KComponentData itsKComponentData; // Required by KLockFile (which is used in CDisabledFonts)
    CDisabledFonts itsDisabledFonts;
};

int CHelper::run()
{
    if(connectToServer(itsSock, itsUid))
    {
        for(;;)
        {
            QVariant cmd;
            bool     res(false);

            kDebug() << "Waiting for cmd..." << endl;
            if(!read(cmd, -1))
                break;

            switch(cmd.type())
            {
                case QVariant::Int:
                    switch(cmd.toInt())
                    {
                        case KFI::CMD_ENABLE_FONT:
                            res=enableFont();
                            break;
                        case KFI::CMD_DISABLE_FONT:
                            res=disableFont();
                            break;
                        case KFI::CMD_DELETE_DISABLED_FONT:
                            res=deleteDisabledFont();
                            break;
                        case KFI::CMD_RELOAD_DISABLED_LIST:
                            kDebug() << "reloadDisabled" << endl;
                            res=itsDisabledFonts.save();
                            itsDisabledFonts.load();
                            break;
                        case KFI::CMD_COPY_FILE:
                            res=copyFile();
                            break;
                        case KFI::CMD_MOVE_FILE:
                            res=moveFile();
                            break;
                        case KFI::CMD_DELETE_FILE:
                            res=deleteFile();
                            break;
                        case KFI::CMD_CREATE_DIR:
                            res=createDir();
                            break;
                        case KFI::CMD_CREATE_AFM:
                            res=createAfm();
                            break;
                        case KFI::CMD_FC_CACHE:
                            kDebug() << "fc-cache" << endl;
                            Misc::doCmd("fc-cache");
                            res=true;
                            break;
                        case KFI::CMD_ADD_DIR_TO_FONTCONFIG:
                            res=addToFc();
                            break;
                        case KFI::CMD_CFG_DIR_FOR_X:
                            res=configureX();
                            break;
                        case KFI::CMD_QUIT:
                            write(true);
                            return 0;
                            break;
                        default:
                            kError() << "Unknown command:" << cmd.toInt() << endl;
                            return -1;
                    }
                    break;
                case QVariant::Invalid:
                    kDebug() << "Finished" << endl;
                    return 0; // finished!
                default:
                    kError() << "Invalid type received when expecting command int, " << cmd.type() << endl;
                    break;
            }

            if(!write(res))
            {
                kError() << " Could not write result to kio_fonts" << endl;
                break;
            }
        }
    }
    else
        kError() << " Could not connect to kio_fonts" << endl;

    return -1;
}

bool CHelper::getFile(CDisabledFonts::TFont &font)
{
    kDebug() << "getFile" << endl;

    QString file;

    if(read(file))
    {
        kDebug() << "file:" << file << endl;
        font.files.append(CDisabledFonts::TFile(file, 0));
        return true;
    }

    kError() << " Invalid parameters for getFile" << endl;

    return false;
}

bool CHelper::disableFont()
{
    kDebug() << "disableFont" << endl;

    QString    family;
    int        style,
               face,
               numFiles;
    qulonglong writingSystems;

    if(read(family) && read(style) && read(writingSystems) && read(face) && read(numFiles))
    {
        kDebug() << "family:" << family << " style:" << style << " face:" << face
                 << " numFiles:" << numFiles << endl;

        CDisabledFonts::TFont font(family, style, writingSystems);

        for(int i=0; i<numFiles; ++i)
            if(!getFile(font))
                return false;

        if(font.files.count())
        {
            if(1==font.files.count())
                (*(font.files.begin())).face=face;
            return itsDisabledFonts.disable(font);
        }
        else
            return false;
    }

    kError() << " Invalid parameters for disableFont" << endl;
    return false;
}

bool CHelper::enableFont()
{
    kDebug() << "enableFont" << endl;

    QString family;
    int     style;

    if(read(family) && read(style))
    {
        kDebug() << "family:" << family << " style:" << style << endl;

        return itsDisabledFonts.enable(family, style);
    }

    kError() << " Invalid parameters for enableFont" << endl;
    return false;
}

bool CHelper::deleteDisabledFont()
{
    kDebug() << "deleteDisabledFont" << endl;

    QString family;
    int     style;

    if(read(family) && read(style))
    {
        kDebug() << "family:" << family << " style:" << style << endl;

        CDisabledFonts::TFont               font(family, style);
        CDisabledFonts::TFontList::Iterator it(itsDisabledFonts.items().find(font));

        if(itsDisabledFonts.modifiable() && itsDisabledFonts.items().end()!=it)
        {
            CDisabledFonts::TFileList::ConstIterator fIt((*it).files.begin()),
                                                     fEnd((*it).files.end());

            for(; fIt!=fEnd; ++fIt)
                if(::unlink(QFile::encodeName((*fIt).path).constData()))
                    return false;

            itsDisabledFonts.remove(it);
            itsDisabledFonts.refresh();
            return true;
        }
        else
            return false;
    }

    kError() << " Invalid parameters for disableFont" << endl;
    return false;
}

bool CHelper::addToFc()
{
    kDebug() << "addToFc" << endl;

    QString dir;

    if(read(dir))
    {
        kDebug() << "dir:" << dir << endl;

        KXftConfig xft(KXftConfig::Dirs, KFI::Misc::root());

        xft.addDir(dir);
        return xft.apply();
    }

    kError() << " Invalid parameters for addToFc" << endl;
    return false;
}

bool CHelper::configureX()
{
    kDebug() << "configureX" << endl;

    QString dir;

    if(read(dir))
    {
        kDebug() << "dir:" << dir << endl;
        return Misc::configureForX11(dir);
    }

    kError() << " Invalid parameters for configureX" << endl;
    return false;
}

bool CHelper::copyFile()
{
    kDebug() << "copyFile" << endl;

    QString from,
            to;

    if(read(from) && read(to))
    {
        kDebug() << "from:" << from << " to:" << to << endl;

        bool rv=QFile::copy(from, to);
        if(rv)
            ::chmod(QFile::encodeName(to).constData(), Misc::FILE_PERMS);
        return rv;
    }

    kError() << " Invalid parameters for copyFile" << endl;
    return false;
}

bool CHelper::moveFile()
{
    kDebug() << "moveFile" << endl;

    QString from,
            to;
    int     user,
            group;

    if(read(from) && read(to) && read(user) && read(group))
    {
        kDebug() << "from:" << from << " to:" << to << " user:" << user << " group:" << group << endl;

        QByteArray toC(QFile::encodeName(to));

        bool res(0==::rename(QFile::encodeName(from).constData(), toC.constData()));

        if(res)
        {
            ::chmod(toC.constData(), Misc::FILE_PERMS);
            ::chown(toC.constData(), user, group);
        }

        return res;
    }

    kError() << " Invalid parameters for moveFile" << endl;
    return false;
}

bool CHelper::deleteFile()
{
    kDebug() << "deleteFile" << endl;

    QString file;

    if(read(file))
    {
        kDebug() << "file:" << file << endl;
        return 0==::unlink(QFile::encodeName(file).constData());
    }

    kError() << " Invalid parameters for deleteFile" << endl;
    return false;
}

bool CHelper::createDir()
{
    kDebug() << "createDir" << endl;

    QString dir;

    if(read(dir))
    {
        kDebug() << "dir:" << dir << endl;

        return KFI::Misc::createDir(dir);
    }

    kError() << " Invalid parameters for createDir" << endl;
    return false;
}

bool CHelper::createAfm()
{
    kDebug() << "createAfm" << endl;

    QString font;

    if(read(font))
    {
        kDebug() << "font:" << font << endl;

        return Misc::doCmd("pf2afm", QFile::encodeName(font));
    }

    kError() << " Invalid parameters for createAfm" << endl;
    return false;
}

}

int main(int argc, char *argv[])
{
    if(argc==3)
        return KFI::CHelper(argv[1], atoi(argv[2])).run();
    else
        fprintf(stderr, "Usage: %s <socket> <uid>\n", argv[0]);
    return -1;
}
