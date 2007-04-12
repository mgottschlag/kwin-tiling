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

#include "FcEngine.h"
#include <QCoreApplication>
#include <QPainter>
#include <QStringList>
#include <QFontDatabase>
#include <QWidget>
#include <QFile>
#include <kprinter.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "CreateParent.h"

// Enable the following to allow printing of non-installed fonts. Doesnt seem to work :-(
//#define KFI_PRINT_APP_FONTS

using namespace KFI;

static const int constMarginLineBefore=1;
static const int constMarginLineAfter=2;
static const int constMarginFont=4;

inline bool sufficientSpace(int y, int pageHeight, int size)
{
    return (y+constMarginFont+size)<pageHeight;
}

static bool sufficientSpace(int y, int titleFontHeight, const int *sizes, int pageHeight, int size)
{
    int required=titleFontHeight+constMarginLineBefore+constMarginLineAfter;

    for(unsigned int s=0; sizes[s]; ++s)
    {
        required+=sizes[s];
        if(sizes[s+1])
            required+=constMarginFont;
    }

    if(0==size)
        required+=(3*(constMarginFont+CFcEngine::constDefaultAlphaSize))+
                  constMarginLineBefore+constMarginLineAfter;
    return (y+required)<pageHeight;
}

static void printItems(const QList<Misc::TFont> &items, int size, QWidget *parent)
{
#ifdef HAVE_LOCALE_H
    char *oldLocale=setlocale(LC_NUMERIC, "C"),
#endif

    QList<Misc::TFont>::ConstIterator it(items.begin()),
                                      end(items.end());
#ifdef KFI_PRINT_APP_FONTS
    QHash<QString, int>               appFont;

    // Check for font files...
    for(; it!=end; ++it)
    {
        if('/'==(*it).family[0] && KFI_NO_STYLE_INFO==(*it).styleInfo &&
           Misc::fExists((*it).family))
            appFont[(*it).family]=QFontDatabase::addApplicationFont((*it).family);
        else
            appFont[(*it).family]=-1;
    }
#endif
    KPrinter printer;

    printer.setFullPage(true);

    if(printer.setup(parent))
    {
        QPainter   painter;
        QFont      sans("sans", 12, QFont::Bold);
        bool       changedFontEmbeddingSetting(false);
        QString    str(CFcEngine::instance()->getPreviewString());

        if(!printer.fontEmbeddingEnabled())
        {
            printer.setFontEmbeddingEnabled(true);
            changedFontEmbeddingSetting=true;
        }

        printer.setResolution(72);
        painter.begin(&printer);

        int       margin=(int)((2/2.54)*painter.device()->logicalDpiY()), // 2 cm margins
                  pageWidth=painter.device()->width()-(2*margin),
                  pageHeight=painter.device()->height()-(2*margin),
                  y=margin,
                  oneSize[2]={size, 0};
        const int *sizes=oneSize;
        bool      firstFont(true);

        if(0==size)
            sizes=CFcEngine::constScalableSizes;

        painter.setClipping(true);
        painter.setClipRect(margin, margin, pageWidth, pageHeight);

        for(it=items.begin(); it!=end; ++it)
        {
            unsigned int s=0;
#ifdef KFI_PRINT_APP_FONTS
            QString      family;
            QFont        font;

            if(-1!=appFont[(*it).family])
            {
                family=QFontDatabase::applicationFontFamilies(appFont[(*it).family]).first();
                font=QFont(family);
            }
#endif
            painter.setFont(sans);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInput, 0);

            if(!firstFont && !sufficientSpace(y, painter.fontMetrics().height(), sizes, pageHeight, size))
            {
                printer.newPage();
                y=margin;
            }
            painter.setFont(sans);
            y+=painter.fontMetrics().height();

#ifdef KFI_PRINT_APP_FONTS
            if(family.isEmpty())
#endif
                painter.drawText(margin, y, FC::createName((*it).family, (*it).styleInfo));
#ifdef KFI_PRINT_APP_FONTS
            else
                painter.drawText(margin, y, family);
#endif

            y+=constMarginLineBefore;
            painter.drawLine(margin, y, margin+pageWidth, y);
            y+=constMarginLineAfter;

            if(0==size)
            {
#ifdef KFI_PRINT_APP_FONTS
                if(family.isEmpty())
#endif
                    painter.setFont(CFcEngine::getQFont((*it).family, (*it).styleInfo,
                                                        CFcEngine::constDefaultAlphaSize));
#ifdef KFI_PRINT_APP_FONTS
                else
                {
                    font.setPointSize(CFcEngine::constDefaultAlphaSize);
                    painter.setFont(font);
                }
#endif

                y+=CFcEngine::constDefaultAlphaSize;
                painter.drawText(margin, y, CFcEngine::getLowercaseLetters());
                y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
                painter.drawText(margin, y, CFcEngine::getUppercaseLetters());
                y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
                painter.drawText(margin, y, CFcEngine::getPunctuation());
                y+=constMarginFont+constMarginLineBefore;
                painter.drawLine(margin, y, margin+pageWidth, y);
                y+=constMarginLineAfter;
            }
            for(; sizes[s]; ++s)
            {
                y+=sizes[s];
#ifdef KFI_PRINT_APP_FONTS
                if(family.isEmpty())
#endif
                    painter.setFont(CFcEngine::getQFont((*it).family, (*it).styleInfo, sizes[s]));
#ifdef KFI_PRINT_APP_FONTS
                else
                {
                    font.setPointSize(sizes[s]);
                    painter.setFont(font);
                }
#endif
                if(sufficientSpace(y, pageHeight, sizes[s]))
                {
                    painter.drawText(margin, y, str);
                    if(sizes[s+1])
                        y+=constMarginFont;
                }
                else
                    break;
            }
            y+=(s<1 || sizes[s-1]<25 ? 14 : 28);
            firstFont=false;
        }

        painter.end();

        //
        // Did we change the users font settings? If so, reset to their previous values...
        if(changedFontEmbeddingSetting)
            printer.setFontEmbeddingEnabled(false);
    }
#ifdef HAVE_LOCALE_H
    if(oldLocale)
        setlocale(LC_NUMERIC, oldLocale);
#endif
}

static KCmdLineOptions options[] =
{
    { "embed <winid>",   I18N_NOOP("Makes the dialog transient for an X app specified by winid"), 0 },
    { "size <index>",    I18N_NOOP("Size index to print fonts"), 0 },
    { "pfont <font>",    I18N_NOOP("Font to print, specified as \"Family,Style\" where Style is a 24-bit decimal number composed as: <weight><width><slant>"), 0},
    { "listfile <file>", I18N_NOOP("File containing list of fonts to print"), 0 },
    { "deletefile",      I18N_NOOP("Remove file containing list of fonts to print"), 0},
    KCmdLineLastOption
};

static KAboutData aboutData("kfontprint", I18N_NOOP("Font Printer"), "1.0", I18N_NOOP("Simple font printer"),
                            KAboutData::License_GPL, I18N_NOOP("(C) Craig Drummond, 2007"));

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication       app;
    KCmdLineArgs       *args(KCmdLineArgs::parsedArgs());
    QList<Misc::TFont> fonts;
    int                size(atoi(args->getOption("size").constData()));

    if(size>-1 && size<256)
    {
        QByteArray listFile(args->getOption("listfile"));

        if(listFile.size())
        {
            QFile f(QFile::decodeName(listFile));

            if(f.open(QIODevice::ReadOnly))
            {
                QTextStream str(&f);

                while (!str.atEnd())
                {
                    QString family(str.readLine()),
                            style(str.readLine());

                    if(!family.isEmpty() && !style.isEmpty())
                        fonts.append(Misc::TFont(family, style.toUInt()));
                    else
                        break;
                }
                f.close();
            }

            if(args->isSet("deletefile"))
                ::unlink(listFile.constData());
        }
        else
        {
            QByteArrayList                fl(args->getOptionList("pfont"));
            QByteArrayList::ConstIterator it(fl.begin()),
                                            end(fl.end());

            for(; it!=end; ++it)
            {
                QString f(QString::fromUtf8(*it));

                int commaPos=f.lastIndexOf(',');

                if(-1!=commaPos)
                    fonts.append(Misc::TFont(f.left(commaPos), f.mid(commaPos+1).toUInt()));
            }
        }

        if(fonts.count())
        {
            KLocale::setMainCatalog(KFI_CATALOGUE);
            printItems(fonts, size, createParent(strtol(args->getOption("embed").constData(), NULL, 16)));

            return 0;
        }
    }

    return -1;
}
