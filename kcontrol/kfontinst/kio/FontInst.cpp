/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2008 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "KfiConstants.h"
#include "FontInst.h"
#include "fontinstadaptor.h"
#include "kxftconfig.h"
#include "PolicyKitMechanism.h"
#include <QtCore/QCoreApplication>
#include <QtDBus/QDBusConnection>
#include <fontconfig/fontconfig.h>
#include <unistd.h>

static const int    constTimeout=10 * 60 * 1000; // Timeout after 10 minutes of inactivity...
static const char * constActionStr=KFI_IFACE;

#define BEGIN_COMMAND \
    EStatus status(NotAuthorised); \
    itsTimer->stop(); \
    if(POLKIT_RESULT_YES==PolicyKitMechanism::instance()->canDoAction(constActionStr, key)) \
    { \

#define END_COMMAND \
    } \
    itsTimer->start(); \
    return status;

FontInst::FontInst(QObject *parent)
        : QObject(parent)
{
    refreshDirList();

    itsTimer=new QTimer(this);
    connect(itsTimer, SIGNAL(timeout()), SLOT(timeout()));
    itsTimer->start(constTimeout);

    new FontinstAdaptor(this);
    QDBusConnection::systemBus().registerObject("/FontInst", this);
}

FontInst::~FontInst()
{
}

int FontInst::disableFont(uint key, const QString &family, uint style, qulonglong writingSystems,
                          uint face, const QStringList &fileData)
{
    BEGIN_COMMAND

    status=CommandFailed;
    if(fileData.count() && 0==fileData.count()%2)
    {
        CDisabledFonts::TFont font(family, style, writingSystems);
        bool                  ok(true);

        for(int i=0; i<fileData.count() && ok; i+=2)
            if(pathIsOk(fileData[i]))
                font.files.append(CDisabledFonts::TFile(fileData[i], 0, fileData[i+1]));
            else
                ok=false;

        if(ok && font.files.count())
        {
            if(1==font.files.count())
                (*(font.files.begin())).face=face;
            status=itsDisabledFonts.disable(font) ? CommandOk : CommandFailed;
        }
    }

    END_COMMAND
}

int FontInst::enableFont(uint key, const QString &family, uint style)
{
    BEGIN_COMMAND

    status=itsDisabledFonts.enable(family, style) ? CommandOk : CommandFailed;

    END_COMMAND
}

int FontInst::deleteDisabledFont(uint key, const QString &family, uint style)
{
    BEGIN_COMMAND

    CDisabledFonts::TFont               font(family, style);
    CDisabledFonts::TFontList::Iterator it(itsDisabledFonts.items().find(font));

    if(itsDisabledFonts.modifiable() && itsDisabledFonts.items().end()!=it)
    {
        CDisabledFonts::TFileList::ConstIterator fIt((*it).files.begin()),
                                                 fEnd((*it).files.end());

        for(; fIt!=fEnd && CommandFailed!=status; ++fIt)
            if(::unlink(QFile::encodeName((*fIt).path).constData()))
                status=CommandFailed;

        if(CommandFailed!=status)
        {
            itsDisabledFonts.remove(it);
            itsDisabledFonts.refresh();
            status=CommandOk;
        }
    }
    else
        status=CommandFailed;

    END_COMMAND
}

int FontInst::reloadDisabledList(uint key)
{
    BEGIN_COMMAND

    status=itsDisabledFonts.save() ? CommandOk : CommandFailed;
    itsDisabledFonts.load();

    END_COMMAND
}

int FontInst::addToFc(uint key, const QString &dir)
{
    BEGIN_COMMAND

    // We should only ever be adding "/usr/local/share/fonts" to font configs dir list...
    if(0==dir.indexOf(KFI_DEFAULT_SYS_FONTS_FOLDER))
    {
        KXftConfig xft(KXftConfig::Dirs, true);

        xft.addDir(dir);
        status=xft.apply() ? CommandOk : CommandFailed;

        if(CommandOk==status)
            refreshDirList();
    }
    else
        status=CommandFailed;

    END_COMMAND
}

int FontInst::configureX(uint key, const QString &dir)
{
    BEGIN_COMMAND

    status=pathIsOk(dir) && Misc::configureForX11(dir) ? CommandOk : CommandFailed;

    END_COMMAND
}

int FontInst::copyFont(uint key, const QString &src, const QString &dest)
{
    BEGIN_COMMAND

    if(pathIsOk(dest) && QFile::copy(src, dest))
    {
        Misc::setFilePerms(dest);
        status=CommandOk;
    }
    else
        status=CommandFailed;

    END_COMMAND
}

int FontInst::moveFont(uint key, const QString &src, const QString &dest, uint uid, uint gid)
{
    BEGIN_COMMAND

    if(pathIsOk(0==uid ? dest : src) && QFile::rename(src, dest))
    {
        Misc::setFilePerms(dest);
        ::chown(QFile::encodeName(dest).constData(), uid, gid);
        status=CommandOk;
    }
    else
        status=CommandFailed;

    END_COMMAND
}

int FontInst::deleteFont(uint key, const QString &font)
{
    BEGIN_COMMAND

    status=pathIsOk(font) && QFile::remove(font) ? CommandOk : CommandFailed;

    END_COMMAND
}

int FontInst::createDir(uint key, const QString &dir)
{
    BEGIN_COMMAND

    status=pathIsOk(dir) && KFI::Misc::createDir(dir) ? CommandOk : CommandFailed;

    END_COMMAND
}

int FontInst::createAfm(uint key, const QString &font)
{
    BEGIN_COMMAND

    status=pathIsOk(font) && Misc::doCmd("pf2afm", QFile::encodeName(font)) ? CommandOk : CommandFailed;

    END_COMMAND
}

int FontInst::fcCache(uint key)
{
    BEGIN_COMMAND

    status=Misc::doCmd("fc-cache") ? CommandOk : CommandFailed;

    END_COMMAND
}

void FontInst::timeout()
{
    QCoreApplication::quit();
}

void FontInst::refreshDirList()
{
    FcStrList *list=FcConfigGetFontDirs(FcInitLoadConfig());
    FcChar8   *dir;
    QString   main;

    itsFcDirs.clear();
    while((dir=FcStrListNext(list)))
    {
        QString d(Misc::dirSyntax((const char *)dir));

        if(0!=d.indexOf(QDir::homePath()))
            if(KFI_DEFAULT_SYS_FONTS_FOLDER==d)
                main=d;
            else
                itsFcDirs.append(d);
    }

    // Place main folder at the top of the list - to speed things up!
    if(!main.isEmpty())
        itsFcDirs.prepend(main);
}

// Check that we are operating on a file withing a fonts folder...
bool FontInst::pathIsOk(const QString &path)
{
    QStringList::ConstIterator it(itsFcDirs.begin()),
                               end(itsFcDirs.end());

    for(; it!=end; ++it)
        if(0==path.indexOf(*it))
            return true;

    // Perhaps dir was added? Refresh fontconfig list and try again...
    refreshDirList();

    it=itsFcDirs.begin();
    end=itsFcDirs.end();

    for(; it!=end; ++it)
        if(0==path.indexOf(*it))
            return true;

    return false;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    if(QDBusConnection::systemBus().registerService(KFI_IFACE))
    {
        new FontInst(0L);
        return app.exec();
    }
    else
        return -1;
}

#include "FontInst.moc"
