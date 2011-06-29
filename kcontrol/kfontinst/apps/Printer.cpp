/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
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

#include "FcEngine.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QFontDatabase>
#include <QtGui/QWidget>
#include <QtCore/QFile>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

#include <KDE/KCmdLineArgs>
#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <kdeprintdialog.h>

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include "CreateParent.h"

// Enable the following to allow printing of non-installed fonts. Does not seem to work :-(
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

#if defined(Q_WS_X11) || defined(Q_WS_QWS)
static QString getChars(FT_Face face)
{
    QString newStr;

    for(int cmap=0; cmap<face->num_charmaps; ++cmap)
        if(face->charmaps[cmap] && FT_ENCODING_ADOBE_CUSTOM==face->charmaps[cmap]->encoding)
        {
            FT_Select_Charmap(face, FT_ENCODING_ADOBE_CUSTOM);
            break;
        }

    for(unsigned int i=1; i<65535; ++i)
        if(FT_Get_Char_Index(face, i))
        {
            newStr+=QChar(i);
            if(newStr.length()>255)
                break;
        }
    
    return newStr;
}

static QString usableStr(FT_Face face, const QString &str)
{
    unsigned int slen=str.length(),
                 ch;
    QString      newStr;

    for(ch=0; ch<slen; ++ch)
        if(FT_Get_Char_Index(face, str[ch].unicode()))
            newStr+=str[ch];
    return newStr;
}

static QString usableStr(QFont &font, const QString &str)
{
    FT_Face face=font.freetypeFace();
    return face ? usableStr(face, str) : str;
}

static bool hasStr(QFont &font, const QString &str)
{
    FT_Face face=font.freetypeFace();

    if(!face)
        return true;

    for(int ch=0; ch<str.length(); ++ch)
        if(!FT_Get_Char_Index(face, str[ch].unicode()))
            return false;
    return true;
}

static QString previewString(QFont &font, const QString &text, bool onlyDrawChars)
{
    FT_Face face=font.freetypeFace();

    if(!face)
        return text;

    QString valid(usableStr(face, text));
    bool    drawChars=onlyDrawChars ||
                      (!hasStr(font, CFcEngine::getLowercaseLetters()) &&
                       !hasStr(font, CFcEngine::getUppercaseLetters()));
                       
    return valid.length()<(text.length()/2) || drawChars ? getChars(face) : valid; 
}
#else
static QString usableStr(QFont &font, const QString &str)
{
    Q_UNUSED(font)
    return str;
}

static bool hasStr(QFont &font, const QString &str)
{
    Q_UNUSED(font)
    return true;
}

static QString previewString(QFont &font, const QString &text, bool onlyDrawChars)
{
    Q_UNUSED(font)
    Q_UNUSED(onlyDrawChars)
    return text;
}
#endif

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
    QPrinter     printer;
    QPrintDialog *dialog = KdePrint::createPrintDialog(&printer, parent);

    dialog->setWindowTitle(i18n("Print"));

    if(dialog->exec())
    {
        QPainter   painter;
        QFont      sans("sans", 12, QFont::Bold);
        bool       changedFontEmbeddingSetting(false);
        QString    str(CFcEngine(false).getPreviewString());

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
            QFont        font;

#ifdef KFI_PRINT_APP_FONTS
            QString      family;

            if(-1!=appFont[(*it).family])
            {
                family=QFontDatabase::applicationFontFamilies(appFont[(*it).family]).first();
                font=QFont(family);
            }
#endif
            painter.setFont(sans);
            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 0);

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
            
            bool onlyDrawChars=false;

            if(0==size)
            {
#ifdef KFI_PRINT_APP_FONTS
                if(family.isEmpty())
#endif
                    font=CFcEngine::getQFont((*it).family, (*it).styleInfo,
                                             CFcEngine::constDefaultAlphaSize);
#ifdef KFI_PRINT_APP_FONTS
                else
                    font.setPointSize(CFcEngine::constDefaultAlphaSize);
#endif
                painter.setFont(font);

                bool lc=hasStr(font, CFcEngine::getLowercaseLetters()),
                     uc=hasStr(font, CFcEngine::getUppercaseLetters());

                onlyDrawChars=!lc && !uc;
                
                if(lc || uc)
                    y+=CFcEngine::constDefaultAlphaSize;
                
                if(lc)
                {
                    painter.drawText(margin, y, CFcEngine::getLowercaseLetters());
                    y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
                }
                
                if(uc)
                {
                    painter.drawText(margin, y, CFcEngine::getUppercaseLetters());
                    y+=constMarginFont+CFcEngine::constDefaultAlphaSize;
                }
                
                if(lc || uc)
                {
                    QString validPunc(usableStr(font, CFcEngine::getPunctuation()));
                    if(validPunc.length()>=(CFcEngine::getPunctuation().length()/2))
                    {
                        painter.drawText(margin, y, CFcEngine::getPunctuation());
                        y+=constMarginFont+constMarginLineBefore;
                    }
                    painter.drawLine(margin, y, margin+pageWidth, y);
                    y+=constMarginLineAfter;
                }
            }
            
            for(; sizes[s]; ++s)
            {
                y+=sizes[s];
#ifdef KFI_PRINT_APP_FONTS
                if(family.isEmpty())
#endif
                    font=CFcEngine::getQFont((*it).family, (*it).styleInfo, sizes[s]);
#ifdef KFI_PRINT_APP_FONTS
                else
                    font.setPointSize(sizes[s]);
#endif
                painter.setFont(font);

                if(sufficientSpace(y, pageHeight, sizes[s]))
                {
                    painter.drawText(margin, y, previewString(font, str, onlyDrawChars));
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

    delete dialog;
}

static KAboutData aboutData("kfontprint", KFI_CATALOGUE, ki18n("Font Printer"), "1.0", ki18n("Simple font printer"),
                            KAboutData::License_GPL, ki18n("(C) Craig Drummond, 2007"));

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("embed <winid>", ki18n("Makes the dialog transient for an X app specified by winid"));
    options.add("size <index>", ki18n("Size index to print fonts"));
    options.add("pfont <font>", ki18n("Font to print, specified as \"Family,Style\" where Style is a 24-bit decimal number composed as: <weight><width><slant>")); //krazy:exclude=i18ncheckarg
    options.add("listfile <file>", ki18n("File containing list of fonts to print"));
    options.add("deletefile", ki18n("Remove file containing list of fonts to print"));
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication       app;
    KCmdLineArgs       *args(KCmdLineArgs::parsedArgs());
    QList<Misc::TFont> fonts;
    int                size(args->getOption("size").toInt());

    if(size>-1 && size<256)
    {
        QString listFile(args->getOption("listfile"));

        if(listFile.size())
        {
            QFile f(listFile);

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
                ::unlink(listFile.toLocal8Bit().constData());
        }
        else
        {
            QStringList                fl(args->getOptionList("pfont"));
            QStringList::ConstIterator it(fl.begin()),
                                       end(fl.end());

            for(; it!=end; ++it)
            {
                QString f(*it);

                int commaPos=f.lastIndexOf(',');

                if(-1!=commaPos)
                    fonts.append(Misc::TFont(f.left(commaPos), f.mid(commaPos+1).toUInt()));
            }
        }

        if(fonts.count())
        {
            KLocale::setMainCatalog(KFI_CATALOGUE);
            printItems(fonts, size, createParent(args->getOption("embed").toInt(0, 16)));

            return 0;
        }
    }

    return -1;
}
