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

#include "Misc.h"
#include "FontInfo.h"
#include "Installer.h"
#include "FcEngine.h"
#include "KfiPrint.h"
#include "kxftconfig.h"
#include <fontconfig/fontconfig.h>
#include <QFile>
#include <QList>
#include <QTextStream>
#include <ksavefile.h>
#include <kglobal.h>
#include <klocale.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

#include <X11/Xlib.h>
#include <fixx11h.h>

#define KFI_ICON    "fonts"
#define KFI_CAPTION I18N_NOOP("Font Installer")

//
// *Very* hacky way to get some KDE dialogs to appear to be transient
// for 'xid'
//
// Create's a QWidget with size 0/0 and no border, makes this transient
// for xid, and all other widgets can use this as their parent...
static QWidget * createParent(int xid)
{
    if(!xid)
        return NULL;

    QWidget *parent=new QWidget;

    parent->resize(0, 0);
    parent->setParent(NULL, Qt::FramelessWindowHint);
    parent->show();

    XWindowAttributes attr;
    int               rx, ry;
    Window            junkwin;

    XSetTransientForHint(QX11Info::display(), parent->winId(), xid);
    if(XGetWindowAttributes(QX11Info::display(), xid, &attr))
    {
        XTranslateCoordinates(QX11Info::display(), xid, attr.root,
                              -attr.border_width, -16,
                              &rx, &ry, &junkwin);

        rx=(rx+(attr.width/2));
        if(rx<0)
            rx=0;
        ry=(ry+(attr.height/2));
        if(ry<0)
            ry=0;
        parent->move(rx, ry);
    }
    parent->setWindowOpacity(0);

    return parent;
}

static void usage(char *app)
{
    std::cerr << "Usage: " << app << " [OPTIONS]..." << std::endl
              << std::endl
              << "  Helper application for KDE's fonts:/ ioslave." << std::endl
              << "  (Please note this application is not intended to be useful by itself)."
                 << std::endl
              << std::endl
              << "    -x <folder>                                     Configure folder for regular x - i.e." << std::endl
              << "                                                    create fonts.dir and fonts.scale" << std::endl
              << std::endl
              << "    -f <folder>                                     Add folder to fontconfig config files."
                 << std::endl
              << std::endl
              << std::endl
              << "  The following options *must* be used by themselves, and not combined."
                 << std::endl
              << std::endl
              << std::endl
              << "    Font installtion:" << std::endl
              << std::endl
              << "      -i <x id> <font file> [<font file...>]        Install font files, if non-root will be " << std::endl
              << "                                                    prompted for destination." << std::endl
              << std::endl
              << std::endl
              << "    Disabled font handling:" << std::endl
              << std::endl
              << "      -h <font> [<index>] <files..>                 Disable the selected font, and hide its files."
                 << std::endl
              << "      -H <family> <style info> [<index>] <files..>" << std::endl
              << std::endl
              << "      -e <font>                                     Enable the selected font, and unhide its files."
                 << std::endl
              << "      -E <family> <style info>" << std::endl
              << std::endl
              << "      -d <font>                                     Remove font from list of disabled fonts, and"
                 << std::endl
              << "                                                    delete any associated files." << std::endl
              << "      -D <family> <style info>" << std::endl
              << std::endl
              << std::endl
              << "    Font Printing:" << std::endl
              << std::endl
              << "       -P <x id> <size> <family> <style info> [...] Print fonts" << std::endl
              << "       -p <x id> <size> <xml file> <y/n>            Print fonts, *removes* xml file if last param==y"
                 << std::endl
              << std::endl
              << std::endl
              << "    <x id>       is a hexadecimal number, starting with 0x, that is the X window ID of " << std::endl
              << "                 the window that any dialogs should be a child of." << std::endl
              << "    <style info> is a 24-bit decimal number composed as: <weight><width><slant>" << std::endl
              << std::endl
              << std::endl
              << "  (C) Craig Drummond, 2003-2006" << std::endl
              << std::endl;

    exit(-1);
}

static int installFonts(int argc, char **argv)
{
    if(argc>3 && '0'==argv[2][0] && 'x'==argv[2][1])
    {
        KAboutData aboutData(KFI_NAME, KFI_CAPTION,
                            "1.0", I18N_NOOP("fonts:/ helper" ),
                            KAboutData::License_GPL,
                            "(C) Craig Drummond, 2003-2006");

        char *dummyArgv[]={ argv[0], (char *)"--icon", (char *)KFI_ICON};

        //KApplication::disableAutoDcopRegistration();
        KCmdLineArgs::init(3, dummyArgv, &aboutData);

        KApplication app;
        QStringList  fonts;
        int          embedId(strtol(argv[2], NULL, 16));

        KLocale::setMainCatalog(KFI_CATALOGUE);
        KGlobal::iconLoader()->addAppDir(KFI_NAME);

        for(int i=3; i<argc; ++i)
            fonts.append(argv[i]);

        KFI::CInstaller inst(argv[0], embedId, createParent(embedId));

        return KFI::CInstaller::INSTALLING==inst.install(fonts) && 0==app.exec() ? 0 : -1;
    }
    return -1;
}

KFI::CFontInfo::TFont getFont(char **argv, int &optind, bool style)
{
    if(style)
        return KFI::CFontInfo::TFont(QString::fromUtf8(argv[optind]), atoi(argv[++optind]));
    else
    {
        QString font(QString::fromUtf8(argv[optind]));
        int     commaPos=font.find(',');

        return KFI::CFontInfo::TFont(-1==commaPos ? font : font.left(commaPos),
                                     KFI::CFcEngine::createStyleVal(font));
    }
}

static int disableFont(int argc, char **argv, int optind, bool style)
{
    if((!style && argc-optind>=2) || (style && argc-optind>=3))
    {
        KInstance           instance(KFI_NAME);
        KFI::CDisabledFonts inf;

        if(inf.modifiable())
        {
            KFI::CFontInfo::TFont font(getFont(argv, optind, style));
            int                   index=-1;

            for(int i=0; i<(argc-optind)-1; ++i)
                if(argv[optind+1+i][0]!='/')
                    index=atoi(argv[optind+1+i]);
                else
                    font.files.append(KFI::CFontInfo::TFile(QString::fromUtf8(argv[optind+1+i]),
                                                            index));

            if(inf.disable(font))
            {
                inf.save();
                return 0;
            }
        }
    }
    return -1;
}

static int enableFont(int argc, char **argv, int optind, bool style)
{
    if((!style && 1==argc-optind) || (style && 2==argc-optind))
    {
        KInstance             instance(KFI_NAME);
        KFI::CDisabledFonts   inf;
        KFI::CFontInfo::TFont font(getFont(argv, optind, style));

        if(inf.modifiable() && inf.enable(font))
        {
            inf.save();
            return 0;
        }
    }
    return -1;
}

static int deleteDisabledFont(int argc, char **argv, int optind, bool style)
{
    if((!style && 1==argc-optind) || (style && 2==argc-optind))
    {
        KInstance                           instance(KFI_NAME);
        KFI::CFontInfo::TFont               font(getFont(argv, optind, style));
        KFI::CDisabledFonts                 inf;
        KFI::CFontInfo::TFontList::Iterator it=inf.items().find(font);

        if(inf.modifiable() && inf.items().end()!=it)
        {
            KFI::CFontInfo::TFileList::ConstIterator fIt((*it).files.begin()),
                                                     fEnd((*it).files.end());

            for(; fIt!=fEnd; ++fIt)
                if(::unlink(QFile::encodeName((*fIt).path).data()))
                    break;

            if(fIt==fEnd)
            {
                inf.remove(it);
                inf.save();
                return 0;
            }
        }
    }
    return -1;
}

static int printFonts(int argc, char **argv, bool xml)
{
    if(argc>3 && '0'==argv[2][0] && 'x'==argv[2][1] &&
       ( (!xml && argc>=6 && 0==(argc%2)) ||
         (xml && '/'==argv[4][0] && ('y'==argv[5][0] || 'n'==argv[5][0]))) ) 
    {
        QList<KFI::Misc::TFont> fonts;
        int                     size(atoi(argv[3]));

        if(size>-1 && size<256)
        {
            if(xml)
            {
                KInstance                                 instance(KFI_NAME);
                KFI::CFontGroups                          grps(argv[4], true, false);
                KFI::CFontInfo::TGroupList::ConstIterator it(grps.items().begin()),
                                                          end(grps.items().end());

                for(; it!=end; ++it)
                    if((*it).name==KFI_PRINT_GROUP)
                    {
                        KFI::CFontInfo::TFontList::ConstIterator fit((*it).fonts.begin()),
                                                                 fend((*it).fonts.end());

                        for(; fit!=fend; ++fit)
                            fonts.append(KFI::Misc::TFont((*fit).family, (*fit).styleInfo));
                        break;
                    }

                if('y'==argv[5][0])
                    ::unlink(argv[4]);
            }
            else
                for(int i=4; i<argc; i+=2)
                    fonts.append(KFI::Misc::TFont(QString::fromUtf8(argv[i]),
                                                  atoi(argv[i+1])));

            if(fonts.count())
            {
                KAboutData aboutData(KFI_NAME, KFI_CAPTION,
                                    "1.0", I18N_NOOP("fonts:/ helper" ),
                                    KAboutData::License_GPL,
                                    "(C) Craig Drummond, 2003-2006");

                char *dummyArgv[]={ argv[0], (char *)"--icon", (char *)KFI_ICON};

//                KApplication::disableAutoDcopRegistration();
                KCmdLineArgs::init(3, dummyArgv, &aboutData);

                KApplication app;

                KLocale::setMainCatalog(KFI_CATALOGUE);
                KFI::Print::printItems(fonts, size, createParent(strtol(argv[2], NULL, 16)));

                return 0;
            }
        }
    }
    return -1;
}

//
// mkfontscale doesnt ingore hidden files :-(
void removeHiddenEntries(const QString &file)
{
    QStringList lines;
    QFile       f(file);

    if(f.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&f);
        QString     line;

        int lineCount=stream.readLine().toInt(); // Ignore line count...

        while(!stream.atEnd())
        {
            line=stream.readLine();
            if(line.length() && '.'!=line[0])
                lines.append(line);
        }
        f.close();

        if(lineCount!=lines.count())
        {
            KSaveFile out(file);

            if(out.open())
            {
                QTextStream                stream(&out);
                QStringList::ConstIterator it(lines.begin()),
                                           end(lines.end());

                stream << lines.count() << endl;
                for(; it!=end; ++it)
                    stream << (*it).toLocal8Bit() << endl;
                out.finalize();
            }
        }
    }
}

static QString removeQuotes(const QString &item)
{
    unsigned int len=item.length();

    return (item[0]==QChar('\'') || item[0]==QChar('\"')) &&
           (item[len-1]==QChar('\'') || item[len-1]==QChar('\"'))
        ? item.mid(1, len-2)
        : item;
}

int main(int argc, char *argv[])
{
    int  c=0,
         rv=0;
    bool doX=false,
         addToFc=false;

    while(-1!=(c=getopt(argc, argv, "ixfhedHEDPp")))
        switch(c)
        {
            case 'x':
                doX=true;
                break;
            case 'f':
                addToFc=true;
                break;
            case 'i':
                return installFonts(argc, argv);
            case 'h':
                return disableFont(argc, argv, optind, false);
            case 'H':
                return disableFont(argc, argv, optind, true);
            case 'e':
                return enableFont(argc, argv, optind, false);
            case 'E':
                return enableFont(argc, argv, optind, true);
            case 'd':
                return deleteDisabledFont(argc, argv, optind, false);
            case 'D':
                return deleteDisabledFont(argc, argv, optind, true);
            case 'P':
                return printFonts(argc, argv, false);
            case 'p':
                return printFonts(argc, argv, true);
            case '?':
                usage(argv[0]);
                break;
        }

    int left=argc-optind;

    if ((1!=left || (!doX && !addToFc)))
        usage(argv[0]);
    else
    {
        QString item(KFI::Misc::dirSyntax(removeQuotes(argv[optind])));

        if(!KFI::Misc::dExists(item))
        {
            std::cerr << "ERROR: " << QFile::encodeName(item).data() << " does not exist!"
                      << std::endl;
            rv=-1;
        }
        else
            if(!item.isEmpty())
            {
                if(0==rv && addToFc)
                {
                    //
                    // Only add folder to fontconfig's config if its not already there...
                    FcStrList *list=FcConfigGetFontDirs(FcConfigGetCurrent());
                    FcChar8   *dir;
                    bool      found=false;

                    while((dir=FcStrListNext(list)))
                        if(0==KFI::Misc::dirSyntax((const char *)dir).indexOf(item))
                            found=true;

                    if(!found)
                    {
                        KXftConfig xft(KXftConfig::Dirs, KFI::Misc::root());

                        xft.addDir(item);
                        rv=xft.apply() ? 0 : -2;
                   }
                }

                if(0==rv && doX)
                {
                    //
                    // On systems without mkfontscale, the following will fail, so cant base
                    // return value upon that - hence only check return value of mkfontdir
                    KFI::Misc::doCmd("mkfontscale", QFile::encodeName(item));
                    removeHiddenEntries(item+"fonts.scale");
                    rv=KFI::Misc::doCmd("mkfontdir", QFile::encodeName(item)) ? 0 : -3;
                    removeHiddenEntries(item+"fonts.dir");
                }
            }
    }

    return rv;
}
