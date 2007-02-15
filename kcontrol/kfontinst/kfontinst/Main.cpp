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

#include "Misc.h"
#include "Installer.h"
#include "Viewer.h"
#include "Fc.h"
#include "KfiPrint.h"
#include "kxftconfig.h"
#include <fontconfig/fontconfig.h>
#include <QFile>
#include <QList>
#include <QTextStream>
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

#define KFI_ICON          "fonts"
#define KFI_INST_CAPTION  I18N_NOOP("Font Installer")
#define KFI_PRINT_CAPTION I18N_NOOP("Font Printer")
#define KFI_VIEW_CAPTION  I18N_NOOP("Font Viewer")

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
              << "  Helper application for KDE's font utilities." << std::endl
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
              << "      -i <x id> <caption> <font file> [<font file...>] Install font files, if non-root will be " << std::endl
              << "                                                       prompted for destination." << std::endl
              << "      -I <font file> [<font file...>]                  As above, but not xid/caption." << std::endl
              << std::endl
              << std::endl
              << "    Font Printing:" << std::endl
              << std::endl
              << "       -P <x id> <caption> <size> <family> <style info> [...] Print fonts" << std::endl
              << "       -p <x id> <caption> <size> <list file> <y/n>           Print fonts, *removes* file if last param==y"
                 << std::endl
              << std::endl
              << std::endl
              << "    Font Viewing:" << std::endl
              << std::endl
              << "       -v <url>                                     View file" << std::endl
                 << std::endl
              << std::endl
              << std::endl
              << "    <x id>       is a hexadecimal number, starting with 0x, that is the X window ID of " << std::endl
              << "                 the window that any dialogs should be a child of." << std::endl
              << "    <style info> is a 24-bit decimal number composed as: <weight><width><slant>" << std::endl
              << std::endl
              << std::endl
              << "  (C) Craig Drummond, 2003-2007" << std::endl
              << std::endl;

    exit(-1);
}

static int installFonts(int argc, char **argv, bool plain)
{
    if(plain
        ? argc>1
        : (argc>4 && '0'==argv[2][0] && 'x'==argv[2][1]))
    {
        const char *caption(plain || argv[3][0]=='\0' ? KFI_INST_CAPTION : argv[3]);

        KAboutData aboutData(KFI_NAME, caption,
                            "1.0", KFI_INST_CAPTION,
                            KAboutData::License_GPL,
                            "(C) Craig Drummond, 2003-2007");

        char *dummyArgv[]={ argv[0], (char *)"--icon", (char *)KFI_ICON};

        //KApplication::disableAutoDcopRegistration();
        KCmdLineArgs::init(3, dummyArgv, &aboutData);

        KApplication app;
        QStringList  fonts;
        int          embedId(plain ? 0 : strtol(argv[2], NULL, 16));

        KLocale::setMainCatalog(KFI_CATALOGUE);
        KIconLoader::global()->addAppDir(KFI_NAME);

        for(int i=plain ? 2 : 4; i<argc; ++i)
            fonts.append(argv[i]);

        KFI::CInstaller inst(createParent(embedId));

        return inst.install(fonts);
    }
    return -1;
}

static int printFonts(int argc, char **argv, bool listFile)
{
    if(argc>6 && '0'==argv[2][0] && 'x'==argv[2][1] &&
       ( (!listFile && argc>=7 && (argc%2)) ||
         (listFile && '/'==argv[5][0] && ('y'==argv[6][0] || 'n'==argv[6][0]))) )
    {
        QList<KFI::Misc::TFont> fonts;
        int                     size(atoi(argv[4]));

        if(size>-1 && size<256)
        {
            if(listFile)
            {
                QFile f(QFile::decodeName(argv[5]));

                if(f.open(QIODevice::ReadOnly))
                {
                    QTextStream str(&f);
                    QString     family,
                                style;

                    for(;;)
                    {
                        family=str.readLine();
                        style=str.readLine();

                        if(!family.isEmpty() && !style.isEmpty())
                            fonts.append(KFI::Misc::TFont(family, style.toUInt()));
                    }
                    f.close();
                }

                if('y'==argv[6][0])
                    ::unlink(argv[5]);
            }
            else
                for(int i=5; i<argc; i+=2)
                    fonts.append(KFI::Misc::TFont(QString::fromUtf8(argv[i]),
                                                  atoi(argv[i+1])));

            if(fonts.count())
            {
                QByteArray caption(argv[3]);
                KAboutData aboutData(KFI_NAME, caption.isEmpty() ? KFI_PRINT_CAPTION : caption,
                                     "1.0", KFI_PRINT_CAPTION,
                                     KAboutData::License_GPL,
                                     "(C) Craig Drummond, 2003-2007");

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

static int viewFont(int argc, char **argv)
{
    if(3==argc)
    {
        KUrl url(QString::fromUtf8(argv[2]));

        if(url.isValid())
        {
            KAboutData aboutData(KFI_NAME, KFI_VIEW_CAPTION,
                                 "1.0", KFI_VIEW_CAPTION,
                                 KAboutData::License_GPL,
                                 "(C) Craig Drummond, 2003-2007");

            char *dummyArgv[]={ argv[0], (char *)"--icon", (char *)"kfontview"};

//                KApplication::disableAutoDcopRegistration();
            KCmdLineArgs::init(3, dummyArgv, &aboutData);

            KApplication app;

            KLocale::setMainCatalog(KFI_CATALOGUE);
            KFI::CViewer *viewer=new KFI::CViewer (url);

            app.setMainWidget(viewer);
            viewer->show();
            return app.exec();
        }
    }
    return -1;
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
    int  c(0),
         rv(0);
    bool doX(false),
         addToFc(false);

    while(-1!=(c=getopt(argc, argv, "xfiIPpv")))
        switch(c)
        {
            case 'x':
                doX=true;
                break;
            case 'f':
                addToFc=true;
                break;
            case 'i':
                return installFonts(argc, argv, false);
            case 'I':
                return installFonts(argc, argv, true);
            case 'P':
                return printFonts(argc, argv, false);
            case 'p':
                return printFonts(argc, argv, true);
            case 'v':
                return viewFont(argc, argv);
            case '?':
                usage(argv[0]);
                break;
        }

    int left(argc-optind);

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
                    FcStrList *list(FcConfigGetFontDirs(FcConfigGetCurrent()));
                    FcChar8   *dir;
                    bool      found(false);

                    while((dir=FcStrListNext(list)))
                    {
                        QString ds(KFI::Misc::dirSyntax((const char *)dir));

                        if(0==ds.indexOf(item) && ds.length()<=item.length())
                            found=true;
                    }

                    if(!found)
                    {
                        KXftConfig xft(KXftConfig::Dirs, KFI::Misc::root());

                        xft.addDir(item);
                        rv=xft.apply() ? 0 : -1;
                   }
                }

                if(0==rv && doX)
                    rv=KFI::Misc::configureForX11(item) ? 0 : -1;
            }
    }

    return rv;
}
