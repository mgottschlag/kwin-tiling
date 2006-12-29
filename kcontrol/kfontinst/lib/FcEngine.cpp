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

#include <QPainter>
#include <QPixmap>
#include <QFile>
#include <QTextStream>
#include <QX11Info>
#include <kurl.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kio/netaccess.h>
#include <kglobal.h>
#include <klocale.h>
#include <math.h>
#include "FcEngine.h"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <fixx11h.h>

//#define KFI_FC_DEBUG

#ifdef KFI_FC_DEBUG
#define KFI_DBUG kDebug() << '[' << (int)(getpid()) << "] CFcEngine - "
#endif

//
// KDE font chooser always seems to use Italic - for both Oblique, and Italic. So I guees
// the fonts:/ should do too - so as to appear more unified.
//
// ditto with respect to Medium/Regular
#define KFI_HAVE_OBLIQUE         // Do we differentiate between Italic and Oblique when comparing slants?
//#define KFI_DISPLAY_OBLIQUE      // Do we want to list "Oblique"? Or always use Italic?
#define KFI_HAVE_MEDIUM_WEIGHT   // Do we differentiate between Medium and Regular weights when comparing weights?
//#define KFI_DISPLAY_MEDIUM      // Do we want to list "Medium"? Or always use Regular? 

#define KFI_PREVIEW_GROUP      "KFontInst Preview Settings"
#define KFI_PREVIEW_STRING_KEY "String"

namespace KFI
{

static CFcEngine *theInstance=NULL;

const int CFcEngine::constScalableSizes[]={8, 10, 12, 24, 36, 48, 64, 72, 96, 0 };
const int CFcEngine::constDefaultAlphaSize=24;

QColor CFcEngine::theirBgndCol(Qt::white);
QColor CFcEngine::theirTextCol(Qt::black);

static int fcWeight(int weight)
{
    if(KFI_NULL_SETTING==weight)
#ifdef KFI_HAVE_MEDIUM_WEIGHT
        return FC_WEIGHT_MEDIUM;
#else
        return FC_WEIGHT_REGULAR;
#endif

    if(weight<FC_WEIGHT_EXTRALIGHT)
        return FC_WEIGHT_THIN;

    if(weight<(FC_WEIGHT_EXTRALIGHT+FC_WEIGHT_LIGHT)/2)
        return FC_WEIGHT_EXTRALIGHT;

    if(weight<(FC_WEIGHT_LIGHT+FC_WEIGHT_REGULAR)/2)
        return FC_WEIGHT_LIGHT;

#ifdef KFI_HAVE_MEDIUM_WEIGHT
    if(weight<(FC_WEIGHT_REGULAR+FC_WEIGHT_MEDIUM)/2)
        return FC_WEIGHT_REGULAR;

    if(weight<(FC_WEIGHT_MEDIUM+FC_WEIGHT_DEMIBOLD)/2)
        return FC_WEIGHT_MEDIUM;
#else
    if(weight<(FC_WEIGHT_REGULAR+FC_WEIGHT_DEMIBOLD)/2)
        return FC_WEIGHT_REGULAR;
#endif

    if(weight<(FC_WEIGHT_DEMIBOLD+FC_WEIGHT_BOLD)/2)
        return FC_WEIGHT_DEMIBOLD;

    if(weight<(FC_WEIGHT_BOLD+FC_WEIGHT_EXTRABOLD)/2)
        return FC_WEIGHT_BOLD;

    if(weight<(FC_WEIGHT_EXTRABOLD+FC_WEIGHT_BLACK)/2)
        return FC_WEIGHT_EXTRABOLD;

    return FC_WEIGHT_BLACK;
}

static int fcToQtWeight(int weight)
{
    switch(weight)
    {
        case FC_WEIGHT_THIN:
            return 0;
        case FC_WEIGHT_EXTRALIGHT:
            return QFont::Light>>1;
        case FC_WEIGHT_LIGHT:
            return QFont::Light;
        default:
        case FC_WEIGHT_REGULAR:
            return QFont::Normal;
        case FC_WEIGHT_MEDIUM:
#ifdef KFI_HAVE_MEDIUM_WEIGHT
            return (QFont::Normal+QFont::DemiBold)>>1;
#endif
            return QFont::Normal;
        case FC_WEIGHT_DEMIBOLD:
            return QFont::DemiBold;
        case FC_WEIGHT_BOLD:
            return QFont::Bold;
        case FC_WEIGHT_EXTRABOLD:
            return (QFont::Bold+QFont::Black)>>1;
        case FC_WEIGHT_BLACK:
            return QFont::Black;
    }
}

static int fcWidth(int width)
{
    if(KFI_NULL_SETTING==width)
        return KFI_FC_WIDTH_NORMAL;

    if(width<KFI_FC_WIDTH_EXTRACONDENSED)
        return KFI_FC_WIDTH_EXTRACONDENSED;

    if(width<(KFI_FC_WIDTH_EXTRACONDENSED+KFI_FC_WIDTH_CONDENSED)/2)
        return KFI_FC_WIDTH_EXTRACONDENSED;

    if(width<(KFI_FC_WIDTH_CONDENSED+KFI_FC_WIDTH_SEMICONDENSED)/2)
        return KFI_FC_WIDTH_CONDENSED;

    if(width<(KFI_FC_WIDTH_SEMICONDENSED+KFI_FC_WIDTH_NORMAL)/2)
        return KFI_FC_WIDTH_SEMICONDENSED;

    if(width<(KFI_FC_WIDTH_NORMAL+KFI_FC_WIDTH_SEMIEXPANDED)/2)
        return KFI_FC_WIDTH_NORMAL;

    if(width<(KFI_FC_WIDTH_SEMIEXPANDED+KFI_FC_WIDTH_EXPANDED)/2)
        return KFI_FC_WIDTH_SEMIEXPANDED;

    if(width<(KFI_FC_WIDTH_EXPANDED+KFI_FC_WIDTH_EXTRAEXPANDED)/2)
        return KFI_FC_WIDTH_EXPANDED;

    if(width<(KFI_FC_WIDTH_EXTRAEXPANDED+KFI_FC_WIDTH_ULTRAEXPANDED)/2)
        return KFI_FC_WIDTH_EXTRAEXPANDED;

    return KFI_FC_WIDTH_ULTRAEXPANDED;
}

#ifndef KFI_FC_NO_WIDTHS
static int fcToQtWidth(int weight)
{
    switch(weight)
    {
        case KFI_FC_WIDTH_ULTRACONDENSED:
            return QFont::UltraCondensed;
        case KFI_FC_WIDTH_EXTRACONDENSED:
            return QFont::ExtraCondensed;
        case KFI_FC_WIDTH_CONDENSED:
            return QFont::Condensed;
        case KFI_FC_WIDTH_SEMICONDENSED:
            return QFont::SemiCondensed;
        default:
        case KFI_FC_WIDTH_NORMAL:
            return QFont::Unstretched;
        case KFI_FC_WIDTH_SEMIEXPANDED:
            return QFont::SemiExpanded;
        case KFI_FC_WIDTH_EXPANDED:
            return QFont::Expanded;
        case KFI_FC_WIDTH_EXTRAEXPANDED:
            return QFont::ExtraExpanded;
        case KFI_FC_WIDTH_ULTRAEXPANDED:
            return QFont::UltraExpanded;
    }
}
#endif

static int fcSlant(int slant)
{
    if(KFI_NULL_SETTING==slant || slant<FC_SLANT_ITALIC)
        return FC_SLANT_ROMAN;

#ifdef KFI_HAVE_OBLIQUE
    if(slant<(FC_SLANT_ITALIC+FC_SLANT_OBLIQUE)/2)
        return FC_SLANT_ITALIC;

    return FC_SLANT_OBLIQUE;
#else
    return FC_SLANT_ITALIC;
#endif
}

static bool fcToQtSlant(int slant)
{
    return FC_SLANT_ROMAN==slant ? false : true;
}

static int fcSpacing(int spacing)
{
    if(spacing<FC_MONO)
        return FC_PROPORTIONAL;

    if(spacing<(FC_MONO+FC_CHARCELL)/2)
        return FC_MONO;

    return FC_CHARCELL;
}

static int strToWeight(const QString &str, QString &newStr)
{
    if(0==str.indexOf(i18n(KFI_WEIGHT_THIN), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_THIN).length());
        return FC_WEIGHT_THIN;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_EXTRALIGHT), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_EXTRALIGHT).length());
        return FC_WEIGHT_EXTRALIGHT;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_ULTRALIGHT), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_ULTRALIGHT).length());
    //    return FC_WEIGHT_ULTRALIGHT;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_LIGHT), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_LIGHT).length());
        return FC_WEIGHT_LIGHT;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_REGULAR), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_REGULAR).length());
        return FC_WEIGHT_REGULAR;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_NORMAL), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_NORMAL).length());
    //    return FC_WEIGHT_NORMAL;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_MEDIUM), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_MEDIUM).length());
        return FC_WEIGHT_MEDIUM;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_DEMIBOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_DEMIBOLD).length());
        return FC_WEIGHT_DEMIBOLD;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_SEMIBOLD), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_SEMIBOLD).length());
    //    return FC_WEIGHT_SEMIBOLD;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_BOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BOLD).length());
        return FC_WEIGHT_BOLD;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_EXTRABOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_EXTRABOLD).length());
        return FC_WEIGHT_EXTRABOLD;
    }
    //if(0==str.indexOf(i18n(KFI_WEIGHT_ULTRABOLD), 0, Qt::CaseInsensitive))
    //{
    //    newStr=str.mid(i18n(KFI_WEIGHT_ULTRABOLD).length());
    //    return FC_WEIGHT_ULTRABOLD;
    //}
    if(0==str.indexOf(i18n(KFI_WEIGHT_BLACK), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BLACK).length());
        return FC_WEIGHT_BLACK;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_BLACK), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BLACK).length());
        return FC_WEIGHT_BLACK;
    }

    newStr=str;
    return FC_WEIGHT_REGULAR;
}

static int strToWidth(const QString &str, QString &newStr)
{
    if(0==str.indexOf(i18n(KFI_WIDTH_ULTRACONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_ULTRACONDENSED).length());
        return KFI_FC_WIDTH_ULTRACONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXTRACONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXTRACONDENSED).length());
        return KFI_FC_WIDTH_EXTRACONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_CONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_CONDENSED).length());
        return KFI_FC_WIDTH_CONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_SEMICONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_SEMICONDENSED).length());
        return KFI_FC_WIDTH_SEMICONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_NORMAL), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_NORMAL).length());
        return KFI_FC_WIDTH_NORMAL;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_SEMIEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_SEMIEXPANDED).length());
        return KFI_FC_WIDTH_SEMIEXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXPANDED).length());
        return KFI_FC_WIDTH_EXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXTRAEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXTRAEXPANDED).length());
        return KFI_FC_WIDTH_EXTRAEXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_ULTRAEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_ULTRAEXPANDED).length());
        return KFI_FC_WIDTH_ULTRAEXPANDED;
    }

    newStr=str;
    return KFI_FC_WIDTH_NORMAL;
}

static int strToSlant(const QString &str)
{
    if(-1!=str.indexOf(i18n(KFI_SLANT_ITALIC)))
        return FC_SLANT_ITALIC;
    if(-1!=str.indexOf(i18n(KFI_SLANT_OBLIQUE)))
        return FC_SLANT_OBLIQUE;
    return FC_SLANT_ROMAN;
}

static void drawText(QPainter &painter, int x, int y, int width, const QString &str)
{
    QString s(str);
    bool    addedElipses=false;

    width-=x*2;
    while(s.length()>3 && painter.fontMetrics().size(0, s).width()>width)
    {
        if(!addedElipses)
        {
            s.remove(s.length()-2, 2);
            s.append("...");
            addedElipses=true;
        }
        else
            s.remove(s.length()-4, 1);
    }
    painter.drawText(x, y, s);
}

inline bool equal(double d1, double d2)
{
    return (fabs(d1 - d2) < 0.0001);
}

inline bool equalWeight(int a, int b)
{
    return a==b || fcWeight(a)==fcWeight(b);
}

#ifndef KFI_FC_NO_WIDTHS
inline bool equalWidth(int a, int b)
{
    return a==b || fcWidth(a)==fcWidth(b);
}
#endif

inline bool equalSlant(int a, int b)
{
    return a==b || fcSlant(a)==fcSlant(b);
}

static bool drawChar(XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, const QString &text, int pos,
                     int &x, int &y, int w, int h, int fSize, int offset)
{
    XGlyphInfo     extents;
    const FcChar16 *str=(FcChar16 *)(&(text.utf16()[pos]));

    XftTextExtents16(QX11Info::display(), xftFont, str, 1, &extents);

    if(x+extents.width+2>w)
    {
        x=offset;
        y+=fSize;
    }

    if(y+offset<h)
    {
        XftDrawString16(xftDraw, xftCol, xftFont, x, y, str, 1);
        x+=extents.width+2;
        return true;
    }
    return false;
}

static bool drawString(XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, const QString &text,
                       int x, int &y, int h, int offset)
{
    XGlyphInfo     extents;
    const FcChar16 *str=(FcChar16 *)(text.utf16());

    XftTextExtents16(QX11Info::display(), xftFont, str, text.length(), &extents);
    if(y+extents.height<h)
        XftDrawString16(xftDraw, xftCol, xftFont, x, y+extents.y, str, text.length());
    if(extents.height>0)
    {
        y+=extents.height+offset;
        return true;
    }
    return false;
}

static bool drawGlyph(XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, FT_UInt i,
                      int &x, int &y, int &w, int &h, int fSize, int offset, bool oneLine)
{
    XGlyphInfo extents;

    XftGlyphExtents(QX11Info::display(), xftFont, &i, 1, &extents);

    if(x+extents.width+2>w)
    {
        if(oneLine)
            return false;

        x=offset;
        y+=fSize+2;
    }

    if(y+offset<h)
    {
        XftDrawGlyphs(xftDraw, xftCol, xftFont, x, y, &i, 1);
        x+=extents.width+2;
        return true;
    }
    return false;
}

static bool drawAllGlyphs(XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol,
                          int size, int &x, int &y, int &w, int &h, int offset, bool oneLine=false)
{
    bool rv=false;
    if(xftFont)
    {
        FT_Face face=XftLockFace(xftFont);

        if(face)
        {
            int space=size/10;

            if(!space)
                space=1;

            rv=true;
            y+=size;
            for(int i=1; i<face->num_glyphs && y<h; ++i)
                if(!drawGlyph(xftDraw, xftFont, xftCol, i, x, y, w, h, size, offset, oneLine))
                    break;

            if(oneLine)
                x=offset;
            XftUnlockFace(xftFont);
        }
    }

    return rv;
}

inline int point2Pixel(int point)
{
    return (point*QX11Info::appDpiX()+36)/72;
}

static bool hasStr(XftFont *font, QString &str)
{
    unsigned int slen=str.length(),
                 ch;

    for(ch=0; ch<slen; ++ch)
        if(!FcCharSetHasChar(font->charset, str[ch].unicode()))
            return false;
    return true;
}

static QString usableStr(XftFont *font, QString &str)
{
    unsigned int slen=str.length(),
                 ch;
    QString      newStr;

    for(ch=0; ch<slen; ++ch)
        if(FcCharSetHasChar(font->charset, str[ch].unicode()))
            newStr+=str[ch];
    return newStr;
}

CFcEngine * CFcEngine::instance()
{
    if(!theInstance)
        theInstance=new CFcEngine;
    return theInstance;
}

CFcEngine::CFcEngine()
         : itsFcDirty(true),
           itsIndex(-1),
           itsIndexCount(1),
           itsPreviewString(getDefaultPreviewString())
{
    reinit();
}

CFcEngine::~CFcEngine()
{
    // Clear any fonts that may have been added...
    FcConfigAppFontClear(FcConfigGetCurrent());
}

void CFcEngine::readConfig(KConfig &cfg)
{
    QString old(cfg.group());

    cfg.setGroup(KFI_PREVIEW_GROUP);
    itsPreviewString=cfg.readEntry(KFI_PREVIEW_STRING_KEY, getDefaultPreviewString());
    cfg.setGroup(old);
}

void CFcEngine::writeConfig(KConfig &cfg)
{
    QString old(cfg.group());

    cfg.setGroup(KFI_PREVIEW_GROUP);
    cfg.writeEntry(KFI_PREVIEW_STRING_KEY, itsPreviewString);
    cfg.setGroup(old);
}

const QString & CFcEngine::getName(const KUrl &url, int faceNo)
{
    if(url!=itsLastUrl || faceNo!=itsIndex)
        parseUrl(url, faceNo);

    return itsDescriptiveName;
}

bool CFcEngine::drawPreview(const QString &item, QPixmap &pix, int h, unsigned long style,
                            int face)
{
    bool rv=false;

    if(!item.isEmpty())
    {
        static const int constOffset=2;
        static const int constInitialWidth=1536;

        bool ok=true;

        if(QChar('/')==item[0])  // Then add to fontconfig's list, so that Xft can display it...
        {
            KUrl url("file://"+item);

            ok=parseUrl(url, face);
            addFontFile(item);
        }
        else
        {
            parseName(item, 0, style);
            itsInstalled=true;
        }

        if(ok)
        {
            itsLastUrl=KUrl();
            getSizes();

            if(itsSizes.size())
            {
                //
                // Calculate size of text...
                int  fSize=((int)(h*0.75))-2,
                     offset=0;
                bool needResize(false);

                if(!itsScalable) // Then need to get nearest size...
                {
                    int bSize=0;

                    for(int s=0; s<itsSizes.size(); ++s)
                        if (itsSizes[s]<=fSize || 0==bSize)
                            bSize=itsSizes[s];
                    fSize=bSize;

                    if(bSize>h)
                    {
                        pix=QPixmap(constInitialWidth, bSize+8);
                        pix.fill(theirBgndCol);
                        needResize=true;
                    }
                    else
                        offset=(fSize-bSize)/2;
                }

                if(!needResize)
                {
                    pix=QPixmap(constInitialWidth, h);
                    pix.fill(theirBgndCol);
                }

                const QX11Info &x11Info(pix.x11Info());
                XRenderColor   xrenderCol;
                XftColor       xftCol;

                xrenderCol.red=theirTextCol.red()<<8;
                xrenderCol.green=theirTextCol.green()<<8;
                xrenderCol.blue=theirTextCol.blue()<<8;
                xrenderCol.alpha=0xffff;

                XftColorAllocValue(QX11Info::display(), DefaultVisual(QX11Info::display(),
                                   x11Info.screen()),
                                   DefaultColormap(QX11Info::display(), x11Info.screen()),
                                   &xrenderCol, &xftCol);

                XftDraw *xftDraw=XftDrawCreate(QX11Info::display(), (Pixmap)(pix.handle()),
                                               (Visual*)(x11Info.visual()), x11Info.colormap());

                if(xftDraw)
                {
                    XftFont *xftFont=NULL;
                    QString text(itsPreviewString);

                    //
                    // Calculate size of text...
                    int fSize=((int)(h*0.75))-2,
                        offset=0;

                    if(!itsScalable) // Then need to get nearest size...
                    {
                        int bSize=0;

                        for(int s=0; s<itsSizes.size(); ++s)
                            if (itsSizes[s]<=fSize || 0==bSize)
                                bSize=itsSizes[s];
                        fSize=bSize;

                        if(bSize>h)
/*
                        {
#ifdef KFI_FC_DEBUG
                            KFI_DBUG << "use size:" << bSize+8 << endl;
#endif
                            pix=QPixmap(constInitialWidth, bSize+8);
                            offset=4;
                            pix.fill(theirBgndCol);
                            needResize=true;
                        }
                        else
*/
                            offset=(fSize-bSize)/2;
                    }

                    xftFont=getFont(fSize);

                    if(xftFont)
                    {
                        if(hasStr(xftFont, text) || hasStr(xftFont, text=text.toUpper()) ||
                           hasStr(xftFont, text=text.toLower()))
                        {
                            XGlyphInfo     extents;
                            const FcChar16 *str=(FcChar16 *)(text.utf16());

                            XftTextExtents16(QX11Info::display(), xftFont, str, text.length(),
                                             &extents);

                            int y=((h-extents.y)/2)+extents.y;
                            XftDrawString16(xftDraw, &xftCol, xftFont, constOffset, y, str,
                                            text.length());
                            if(needResize)
                                pix=pix.scaledToHeight(h);
                            pix.resize(extents.width+(2*constOffset)<constInitialWidth
                                           ? extents.width+(2*constOffset)
                                           : constInitialWidth,
                                       h);
                            rv=true;
                        }
                        else
                        {
                            FT_Face face=XftLockFace(xftFont);

                            if(face)
                            {
                                int x=constOffset,
                                    y=constOffset+fSize;

                                for(FT_UInt i=1; i<(unsigned int)face->num_glyphs &&
                                                 i<(unsigned int)text.length()+1; ++i)
                                {
                                    XGlyphInfo extents;

                                    XftGlyphExtents(QX11Info::display(), xftFont, &i, 1,
                                                    &extents);

                                    if(x+extents.width+2>constInitialWidth)  // Only want 1 line
                                        break;

                                    XftDrawGlyphs(xftDraw, &xftCol, xftFont, x, y, &i, 1);
                                    x+=extents.width+2;
                                }
                                if(needResize)
                                    pix=pix.scaledToHeight(h);
                                pix.resize(x+constOffset<constInitialWidth
                                               ? x+constOffset
                                               : constInitialWidth,
                                           h);
                                XftUnlockFace(xftFont);
                                rv=true;
                            }
                        }
                    }
                }
            }
        }
    }

    return rv;
}

bool CFcEngine::draw(const KUrl &url, int w, int h, QPixmap &pix, int faceNo, bool thumb,
                     int unicodeStart, const QString &name, unsigned long style)
{
    bool rv=false;

    if((url==itsLastUrl && faceNo==itsIndex) ||
        (!name.isEmpty() && parseName(name, faceNo, style, false, url)) ||
        parseUrl(url, faceNo))
    {
        if(!name.isEmpty())
            itsInstalled=true;

        if(!itsInstalled)  // Then add to fontconfig's list, so that Xft can display it...
            addFontFile(itsFileName);

        //
        // We allow kio_thumbnail to cache our thumbs. Normal is 128x128, and large is 256x256
        // ...if kio_thumbnail asks us for a bigger size, then its probably the file info dialog, in
        // which case treat it as a normal preview...
        if(thumb && (h>256 || w!=h))
            thumb=false;

        int offset=thumb
                       ? h<=32
                             ? 2
                             : 3
                       : 4,
            x=offset, y=offset;

        pix=QPixmap(w, h);
        pix.fill(theirBgndCol);

        QPainter painter(&pix);

        getSizes();

        if(itsSizes.size())
        {
            const QX11Info &x11Info(pix.x11Info());
            XRenderColor   xrenderCol;
            XftColor       xftCol;

            xrenderCol.red=theirTextCol.red()<<8;
            xrenderCol.green=theirTextCol.green()<<8;
            xrenderCol.blue=theirTextCol.blue()<<8;
            xrenderCol.alpha=0xffff;
            XftColorAllocValue(QX11Info::display(), DefaultVisual(QX11Info::display(),
                               x11Info.screen()), 
                               DefaultColormap(QX11Info::display(), x11Info.screen()),
                               &xrenderCol, &xftCol);

            XftDraw *xftDraw=XftDrawCreate(QX11Info::display(), (Pixmap)(pix.handle()),
                                           (Visual*)(x11Info.visual()), x11Info.colormap());

            if(xftDraw)
            {
                XftFont *xftFont=NULL;

                if(thumb)
                {
                    QString text(i18n("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));

                    //
                    // Calculate size of text...
                    int fSize= h <= 32
                                ? h-(offset*2)          // 1 line of chars...
                                : /*h < 132
                                        ?*/ (h-(offset*3))/2   // 2 lines...
                                        /*: (h-(offset*4))/3*/;  // 3 lines or more

                    if(!itsScalable) // Then need to get nearest size...
                    {
                        int bSize=0;

                        for(int s=0; s<itsSizes.size(); ++s)
                            if (itsSizes[s]<=fSize || 0==bSize)
                                bSize=itsSizes[s];
                        fSize=bSize;
                    }

                    unsigned int ch;

                    xftFont=getFont(fSize);

                    y=fSize;
                    if(xftFont)
                    {
                        QString valid(usableStr(xftFont, text));

                        rv=true;

                        if(valid.length()<(text.length()/2))
                            drawAllGlyphs(xftDraw, xftFont, &xftCol, fSize, x, y, w, h, offset);
                        else
                            for(ch=0; ch<(unsigned int)valid.length(); ++ch) // Display char by char so wraps...
                                if(!drawChar(xftDraw, xftFont, &xftCol, text, ch, x, y, w, h, fSize,
                                             offset))
                                    break;
                    }
                }
                else if(STD_PREVIEW==unicodeStart)
                {
                    QString lowercase(getLowercaseLetters()),
                            uppercase(getUppercaseLetters()),
                            punctuation(getPunctuation());

                    drawName(painter, x, y, w, offset);

                    bool lc=true,
                         uc=true,
                         punc=true;

                    xftFont=getFont(itsAlphaSize);
                    if(xftFont)
                    {
                        QString validPunc(usableStr(xftFont, punctuation));

                        rv=true;
                        lc=hasStr(xftFont, lowercase);
                        uc=hasStr(xftFont, uppercase);
                        punc=validPunc.length()>=(punctuation.length()/2);

                        bool drawGlyphs=!lc && !uc;

                        if(drawGlyphs)
                            y-=8;
                        else
                        {
                            if(lc)
                                drawString(xftDraw, xftFont, &xftCol, lowercase, x, y, h, offset);
                            if(uc)
                                drawString(xftDraw, xftFont, &xftCol, uppercase, x, y, h, offset);
                            if(punc)
                                drawString(xftDraw, xftFont, &xftCol, validPunc, x, y, h, offset);
                            XftFontClose(QX11Info::display(), xftFont);
                            if(lc || uc || punc)
                                painter.drawLine(offset, y, w-(offset+1), y);
                            y+=8;
                        }

                        QString previewString(getPreviewString());

                        if(!drawGlyphs)
                        {
                            if(!lc && uc)
                                previewString=previewString.toUpper();
                            if(!uc && lc)
                                previewString=previewString.toLower();
                        }

                        for(int s=0; s<itsSizes.size(); ++s)
                            if((xftFont=getFont(itsSizes[s])))
                            {
                                rv=true;
                                if(drawGlyphs)
                                    drawAllGlyphs(xftDraw, xftFont, &xftCol, itsSizes[s], x, y, w, h, offset,
                                                  itsSizes.count()>1);
                                else
                                    drawString(xftDraw, xftFont, &xftCol, previewString, x, y, h,
                                               offset);
                                XftFontClose(QX11Info::display(), xftFont);
                            }
                    }

                    painter.setPen(theirBgndCol);
                    for(int l=0; l<offset; ++l)
                        painter.drawLine((w-1)-l, 0, (w-1)-l, h);
                }
                else if(ALL_CHARS==unicodeStart)
                {
                    drawName(painter, x, y, w, offset);

                    if((xftFont=getFont(itsAlphaSize)))
                    {
                        y-=8;
                        drawAllGlyphs(xftDraw, xftFont, &xftCol, itsAlphaSize, x, y, w, h, offset);
                        rv=true;
                        XftFontClose(QX11Info::display(), xftFont);
                    }
                }
                else  // Want to draw 256 chars from font...
                {
                    static const int constGap=4;
                    static const int constMinSize=8;

                    //
                    // Calculate size of text...
                    int fSize=(w-(2*offset))/16;

                    fSize-=2*constGap;

                    if(fSize<constMinSize)
                        fSize=constMinSize;

                    if(!itsScalable) // Then need to get nearest size...
                    {
                        int bSize=fSize;

                        for(int s=0; s<itsSizes.size(); ++s)
                            if (itsSizes[s]<=fSize)
                                bSize=itsSizes[s];
                        fSize=bSize;
                    }

                    xftFont=getFont(fSize);

                    if(xftFont)
                    {
                        QString str("A");

                        rv=true;
                        drawName(painter, x, y, w, offset);
                        painter.setPen(Qt::gray);
                        y+=constGap;
                        int ds=16*(fSize+(2*constGap));
                        for(int i=0; i<17; ++i)
                        {
                            painter.drawLine(x, y+(i*(fSize+(2*constGap))), x+ds-1,
                                             y+(i*(fSize+(2*constGap))));
                            painter.drawLine(x+(i*(fSize+(2*constGap))), y, x+(i*(fSize+(2*constGap))),
                                             y+ds-1);
                        }
                        y+=fSize;
                        x+=constGap;
                        for(int a=0; a<256; ++a)
                        {
                            str[0]=unicodeStart+a;
                            const FcChar16 *fcStr=(FcChar16 *)(&(str.utf16()[0]));
                            XftDrawString16(xftDraw, &xftCol, xftFont, x, y, fcStr, 1);

                            if(!((a+1)%16))
                            {
                                y+=fSize+(2*constGap);
                                x=offset+constGap;
                            }
                            else
                                x+=fSize+(2*constGap);

                            if(y>h)
                                break;
                        }
                    }
                }

                XftDrawDestroy(xftDraw);
            }
        }
    }

    return rv;
}

unsigned long CFcEngine::createStyleVal(const QString &name)
{
    int pos;

    if(-1==(pos=name.indexOf(", ")))   // No style information...
        return createStyleVal(FC_WEIGHT_REGULAR,
#ifdef KFI_FC_NO_WIDTHS
                              KFI_NULL_SETTING
#else
                              KFI_FC_WIDTH_NORMAL
#endif
                              , FC_SLANT_ROMAN);
    else
    {
        QString style(name.mid(pos+2));

        return createStyleVal(strToWeight(style, style),
#ifdef KFI_FC_NO_WIDTHS
                              KFI_NULL_SETTING
#else
                              strToWidth(style, style)
#endif
                              , strToSlant(style));
    }
}

QString CFcEngine::styleValToStr(unsigned long style)
{
    QString str;
    int     weight, width, slant;

    decomposeStyleVal(style, weight, width, slant);
    str.sprintf("0X%02X%02X%02X\n", weight, width, slant);
    return str;
}

void CFcEngine::decomposeStyleVal(int styleInfo, int &weight, int &width, int &slant)
{
    weight=(styleInfo&0xFF0000)>>16;
    width=(styleInfo&0x00FF00)>>8;
    slant=(styleInfo&0x0000FF);
}

unsigned long CFcEngine::styleValFromStr(const QString &style)
{
    if(style.isEmpty())
        return KFI_NO_STYLE_INFO;
    else
    {
        unsigned long val;

        QTextIStream(&style)>>val;
        return val;
    }
}

QString CFcEngine::getDefaultPreviewString()
{
    return i18nc("A sentence that uses all of the letters of the alphabet",
                 "The quick brown fox jumps over the lazy dog");
}

QString CFcEngine::getUppercaseLetters()
{
    return i18nc("All of the letters of the alphabet, uppercase", "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

QString CFcEngine::getLowercaseLetters()
{
    return i18nc("All of the letters of the alphabet, lowercase", "abcdefghijklmnopqrstuvwxyz");
}

QString CFcEngine::getPunctuation()
{
    return i18nc("Numbers and characters", "0123456789.:,;(*!?'/\\\")£$€%^&-+@~#<>{}[]");
}

QString CFcEngine::getFcString(FcPattern *pat, const char *val, int index)
{
    QString rv;
    FcChar8 *fcStr;

    if(FcResultMatch==FcPatternGetString(pat, val, index, &fcStr))
        rv=QString::fromUtf8((char *)fcStr);

    return rv;
}

#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
//
// Try to get the 'string' that matches the users KDE locale..
QString CFcEngine::getFcLangString(FcPattern *pat, const char *val, const char *valLang)
{
    QString                    rv;
    QStringList                kdeLangs=KGlobal::locale()->languagesTwoAlpha(),
                               fontLangs;
    QStringList::ConstIterator it(kdeLangs.begin()),
                               end(kdeLangs.end());

    // Create list of langs that this font's 'val' is encoded in...
    for(int i=0; true; ++i)
    {
        QString lang=getFcString(pat, valLang, i);

        if(lang.isEmpty())
            break;
        else
            fontLangs.append(lang);
    }

    // Now go through the user's KDE locale, and try to find a font match...
    for(; it!=end; ++it)
    {
        int index=fontLangs.findIndex(*it);

        if(-1!=index)
        {
            rv=getFcString(pat, val, index);

            if(!rv.isEmpty())
                break;
        }
    }

    if(rv.isEmpty())
        rv=getFcString(pat, val, 0);
    return rv;
}
#endif

int CFcEngine::getFcInt(FcPattern *pat, const char *val, int index, int def)
{
    int rv;

    if (FcResultMatch==FcPatternGetInteger(pat, val, index, &rv))
        return rv;
    return def;
}

void CFcEngine::getDetails(FcPattern *pat, QString &name, int &styleVal, int &index)
{
    int weight=getFcInt(pat,  FC_WEIGHT, 0, KFI_NULL_SETTING),
        width=
#ifdef KFI_FC_NO_WIDTHS
               KFI_NULL_SETTING,
#else
               getFcInt(pat,  FC_WIDTH, 0, KFI_NULL_SETTING),
#endif
        slant=getFcInt(pat,  FC_SLANT, 0, KFI_NULL_SETTING);

    index=getFcInt(pat,  FC_INDEX, 0, 0);
    name=CFcEngine::createName(pat, weight, width, slant);
    styleVal=CFcEngine::createStyleVal(weight, width, slant);
}

QString CFcEngine::createName(FcPattern *pat)
{
    return createName(pat, getFcInt(pat, FC_WEIGHT, 0),
#ifdef KFI_FC_NO_WIDTHS
                      KFI_NULL_SETTING,
#else
                      getFcInt(pat, FC_WIDTH, 0),
#endif
                      getFcInt(pat, FC_SLANT, 0));
}

QString CFcEngine::createName(FcPattern *pat, int weight, int width, int slant)
{
#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
    QString family(getFcLangString(pat, FC_FAMILY, FC_FAMILYLANG));
#else
    QString family(getFcString(pat, FC_FAMILY, 0));
#endif

    return createName(family, weight, width, slant);
}

QString CFcEngine::createName(const QString &family, int styleInfo)
{
    int weight, width, slant;

    decomposeStyleVal(styleInfo, weight, width, slant);
    return createName(family, weight, width, slant); 
}

QString CFcEngine::createName(const QString &family, int weight, int width, int slant)
{
//
//CPD: TODO: the names *need* to match up with kfontchooser's...
//         : Removing KFI_DISPLAY_OBLIQUE and KFI_DISPLAY_MEDIUM help this.
//           However, I have at least one bad font:
//               Rockwell Extra Bold. Both fontconfig, and kcmshell fonts list family
//               as "Rockwell Extra Bold" -- good (well at least they match). *But* fontconfig
//               is returning the weight "Extra Bold", and kcmshell fonts is using "Bold" :-(
//
    QString name(family),
            weightString,
            widthString,
            slantString;
    bool    comma=false;

#ifndef KFI_FC_NO_WIDTHS
    if(KFI_NULL_SETTING!=width)
        widthString=widthStr(width);
#endif

    if(KFI_NULL_SETTING!=slant)
        slantString=slantStr(slant);

    //
    // If weight is "Regular", we only want to display it if slant and width are empty.
    if(KFI_NULL_SETTING!=weight)
        weightString=weightStr(weight, !slantString.isEmpty() || !widthString.isEmpty());

    if(!weightString.isEmpty())
    {
        name+=QString(", ")+weightString;
        comma=true;
    }

#ifndef KFI_FC_NO_WIDTHS
    if(!widthString.isEmpty())
    {
        if(!comma)
        {
            name+=QChar(',');
            comma=true;
        }
        name+=QChar(' ')+widthString;
    }
#endif

    if(!slantString.isEmpty())
    {
        if(!comma)
        {
            name+=QChar(',');
            comma=true;
        }
        name+=QChar(' ')+slantString;
    }

    return name;
}

QString CFcEngine::weightStr(int weight, bool emptyNormal)
{
    switch(fcWeight(weight))
    {
        case FC_WEIGHT_THIN:
            return i18n(KFI_WEIGHT_THIN);
        case FC_WEIGHT_EXTRALIGHT:
            return i18n(KFI_WEIGHT_EXTRALIGHT);
        case FC_WEIGHT_LIGHT:
            return i18n(KFI_WEIGHT_LIGHT);
        case FC_WEIGHT_MEDIUM:
#ifdef KFI_DISPLAY_MEDIUM
            return i18n(KFI_WEIGHT_MEDIUM);
#endif
        case FC_WEIGHT_REGULAR:
            return emptyNormal ? QString::null : i18n(KFI_WEIGHT_REGULAR);
        case FC_WEIGHT_DEMIBOLD:
            return i18n(KFI_WEIGHT_DEMIBOLD);
        case FC_WEIGHT_BOLD:
            return i18n(KFI_WEIGHT_BOLD);
        case FC_WEIGHT_EXTRABOLD:
            return i18n(KFI_WEIGHT_EXTRABOLD);
        default:
            return i18n(KFI_WEIGHT_BLACK);
    }
}

QString CFcEngine::widthStr(int width, bool emptyNormal)
{
    switch(fcWidth(width))
    {
        case KFI_FC_WIDTH_ULTRACONDENSED:
            return i18n(KFI_WIDTH_ULTRACONDENSED);
        case KFI_FC_WIDTH_EXTRACONDENSED:
            return i18n(KFI_WIDTH_EXTRACONDENSED);
        case KFI_FC_WIDTH_CONDENSED:
            return i18n(KFI_WIDTH_CONDENSED);
        case KFI_FC_WIDTH_SEMICONDENSED:
            return i18n(KFI_WIDTH_SEMICONDENSED);
        case KFI_FC_WIDTH_NORMAL:
            return emptyNormal ? QString::null : i18n(KFI_WIDTH_NORMAL);
        case KFI_FC_WIDTH_SEMIEXPANDED:
            return i18n(KFI_WIDTH_SEMIEXPANDED);
        case KFI_FC_WIDTH_EXPANDED:
            return i18n(KFI_WIDTH_EXPANDED);
        case KFI_FC_WIDTH_EXTRAEXPANDED:
            return i18n(KFI_WIDTH_EXTRAEXPANDED);
        default:
            return i18n(KFI_WIDTH_ULTRAEXPANDED);
    }
}

QString CFcEngine::slantStr(int slant, bool emptyNormal)
{
    switch(fcSlant(slant))
    {
        case FC_SLANT_OBLIQUE:
#ifdef KFI_DISPLAY_OBLIQUE
            return i18n(KFI_SLANT_OBLIQUE);
#endif
        case FC_SLANT_ITALIC:
            return i18n(KFI_SLANT_ITALIC);
        default:
            return emptyNormal ? QString::null : i18n(KFI_SLANT_ROMAN);
    }
}

QString CFcEngine::spacingStr(int spacing)
{
    switch(fcSpacing(spacing))
    {
        case FC_MONO:
            return i18n(KFI_SPACING_MONO);
        case FC_CHARCELL:
            return i18n(KFI_SPACING_CHARCELL);
        default:
            return i18n(KFI_SPACING_PROPORTIONAL);
    }
}

bool CFcEngine::getInfo(const KUrl &url, int faceNo, QString &full, QString &family, QString &foundry, QString &weight,
                        QString &width, QString &spacing, QString &slant)
{
    if(parseUrl(url, faceNo, true))
    {
        full=itsDescriptiveName;
        if(url.isLocalFile() || Misc::isHidden(url))
        {
            int pos;

            if(-1==(pos=itsDescriptiveName.indexOf(", ")))   // No style information...
                family=itsDescriptiveName;
            else
                family=itsDescriptiveName.left(pos);
        }
        else
            family=itsName;
        weight=weightStr(itsWeight, false);
        width=widthStr(itsWidth, false);
        slant=slantStr(itsSlant, false);
        spacing=spacingStr(itsSpacing);
        foundry=itsFoundry;
        return true;
    }

    return false;
}

bool CFcEngine::getInfo(const KUrl &url, int faceNo, Misc::TFont &info)
{
    if(parseUrl(url, faceNo, true))
    {
        if(url.isLocalFile() || Misc::isHidden(url))
        {
            int pos;

            if(-1==(pos=itsDescriptiveName.indexOf(", ")))   // No style information...
                info.family=itsDescriptiveName;
            else
                info.family=itsDescriptiveName.left(pos);
        }
        else
            info.family=itsName;
        info.styleInfo=styleVal();
        return true;
    }

    return false;
}

QFont CFcEngine::getQFont(const QString &family, unsigned long style, int size)
{
    int weight,
        width,
        slant;

    decomposeStyleVal(style, weight, width, slant);

    QFont font(family, size, fcToQtWeight(weight), fcToQtSlant(slant));

#ifndef KFI_FC_NO_WIDTHS
    font.setStretch(fcToQtWidth(width));
#endif
    return font;
}

bool CFcEngine::parseUrl(const KUrl &url, int faceNo, bool all)
{
    if(faceNo<0)
        faceNo=0;

    itsFileName=QString::null;
    itsIndex=faceNo;

    reinit();

    //KFI_DBUG << "parseUrl:" << url.prettyUrl() << endl;
    // Possible urls:
    //
    //    fonts:/times.ttf
    //    fonts:/System/times.ttf
    //    file:/home/wibble/hmm.ttf
    //
    if(KFI_KIO_FONTS_PROTOCOL==url.protocol())
    {
        bool          hidden=Misc::isHidden(url);
        KIO::UDSEntry udsEntry;
        QString       name;
        unsigned long style=KFI_NO_STYLE_INFO;

        if(KIO::NetAccess::stat(url, udsEntry, NULL))  // Need to stat the url to get its font name...
        {
            name=udsEntry.stringValue((uint)KIO::UDS_NAME);
            itsFileName=udsEntry.stringValue((uint)UDS_EXTRA_FILE_NAME);
            style=styleValFromStr(udsEntry.stringValue((uint)UDS_EXTRA_FC_STYLE));
            itsIndex=Misc::getIntQueryVal(KUrl(udsEntry.stringValue((uint)KIO::UDS_URL)),
                                          KFI_KIO_FACE, 0);
        }

        if(hidden)
            name=itsFileName;

        if(!name.isEmpty())
        {
            if(hidden)
            {
                if(!parseUrl(KUrl(name), faceNo, all))
                    return false;
            }
            else
            {
                parseName(name, faceNo, style, all);
                itsInstalled=true;
            }
        }
        else
            return false;
    }
    else if(url.isLocalFile())
    {
        // Now lets see if its from the thumbnail job! if so, then file should contain either:
        //    a. fonts:/ Url
        //    b. FontName followed by style info
        QFile file(url.path());
        bool  isThumbnailUrl=false;

        if(file.size()<2048 && file.open(IO_ReadOnly)) // Data should be less than 2k, and fonts usually above!
        {
            QTextStream stream(&file);
            QString     line1(stream.readLine()),
                        line2(stream.readLine());

            if(line2.isEmpty())
                isThumbnailUrl=(0==line1.indexOf(KFI_KIO_FONTS_PROTOCOL) ||
                                0==line1.indexOf("file:/")) &&
                               parseUrl(KUrl(line1), faceNo, all);
            else if(0==line1.indexOf(KFI_PATH_KEY) && 0==line2.indexOf(KFI_FACE_KEY))
            {
                line1=line1.mid(strlen(KFI_PATH_KEY));
                line2=line2.mid(strlen(KFI_FACE_KEY));

                if(!line1.isEmpty() && !line2.isEmpty())
                {
                    bool ok=false;
                    int  face=line2.toInt(&ok);

                    isThumbnailUrl=ok && parseUrl(line1, face<0 ? 0 : face, true);
                }
            }
            else if(0==line1.indexOf(KFI_NAME_KEY) && 0==line2.indexOf(KFI_STYLE_KEY))
            {
                line1=line1.mid(strlen(KFI_NAME_KEY));
                line2=line2.mid(strlen(KFI_STYLE_KEY));

                if(!line1.isEmpty() && !line2.isEmpty())
                {
                    bool          ok=false;
                    unsigned long style=line2.toULong(&ok);

                    itsInstalled=isThumbnailUrl=ok && parseName(line1, faceNo, style, all);

                    if(itsInstalled)
                    {
                        QString line3(stream.readLine()),
                                line4(stream.readLine());

                        if(0==line3.indexOf(KFI_PATH_KEY) && 0==line4.indexOf(KFI_FACE_KEY))
                        {
                            line3=line3.mid(strlen(KFI_PATH_KEY));
                            line4=line4.mid(strlen(KFI_FACE_KEY));

                            if(!line1.isEmpty() && !line2.isEmpty())
                            {
                                ok=false;
                                int  face=line4.toInt(&ok);

                                if(ok)
                                {
                                    itsFileName=line3;
                                    itsIndex=face;
                                }
                            }
                        }
                    }
                }
            }
            file.close();
        }

        if(!isThumbnailUrl)  // Its not a thumbnail, so read the real font file...
        {
            itsName=itsFileName=url.path();

            int       count;
            FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(itsFileName).data()),
                                           faceNo, NULL, &count);

            itsWeight=FC_WEIGHT_REGULAR;
            itsWidth=KFI_FC_WIDTH_NORMAL;
            itsSlant=FC_SLANT_ROMAN;
            itsSpacing=FC_PROPORTIONAL;

            if(pat)
            {
                FcPatternGetInteger(pat, FC_WEIGHT, 0, &itsWeight);
                FcPatternGetInteger(pat, FC_SLANT, 0, &itsSlant);
#ifndef KFI_FC_NO_WIDTHS
                FcPatternGetInteger(pat, FC_WIDTH, 0, &itsWidth);
#endif
                if(all)
                {
                    FcPatternGetInteger(pat, FC_SPACING, 0, &itsSpacing);
                    itsFoundry=getFcString(pat, FC_FOUNDRY, 0);
                }
                itsDescriptiveName=createName(pat, itsWeight, itsWidth, itsSlant);
                FcPatternDestroy(pat);
            }
            else
            {
                itsDescriptiveName=QString::null;
                return false;
            }

            itsInstalled=false;
            itsIndex=faceNo;
        }
    }
    else
        return false;

    itsLastUrl=url;
    return true;
}

bool CFcEngine::parseName(const QString &name, int faceNo, unsigned long style, bool all,
                          const KUrl &url)
{
    int pos;

    reinit();

    itsDescriptiveName=name;
    itsSpacing=FC_PROPORTIONAL;

    if(-1==(pos=name.indexOf(", ")))   // No style information...
    {
        if(KFI_NO_STYLE_INFO!=style)
            decomposeStyleVal(style, itsWeight, itsWidth, itsSlant);
        else
        {
            itsWeight=FC_WEIGHT_REGULAR;
            itsWidth=KFI_FC_WIDTH_NORMAL;
            itsSlant=FC_SLANT_ROMAN;
        }
        itsName=name;
    }
    else
    {
        if(KFI_NO_STYLE_INFO!=style)
            decomposeStyleVal(style, itsWeight, itsWidth, itsSlant);
        else
        {
            QString style(name.mid(pos+2));

            itsWeight=strToWeight(style, style);
            itsWidth=strToWidth(style, style);
            itsSlant=strToSlant(style);
        }
        itsName=name.left(pos);
    }

    itsFileName=QString::null;
    if(all)
    {
        FcObjectSet *os  = FcObjectSetBuild(FC_SPACING, FC_FOUNDRY, (void *)0);
        FcPattern   *pat = FcPatternBuild(NULL,
                                            FC_FAMILY, FcTypeString,
                                                (const FcChar8 *)(itsName.toUtf8().data()),
                                            FC_WEIGHT, FcTypeInteger, itsWeight,
                                            FC_SLANT, FcTypeInteger, itsSlant,
#ifndef KFI_FC_NO_WIDTHS
                                            FC_WIDTH, FcTypeInteger, itsWidth,
#endif
                                            NULL);
        FcFontSet   *set = FcFontList(0, pat, os);

        FcPatternDestroy(pat);
        FcObjectSetDestroy(os);

        if(set && set->nfont)
        {
            FcPatternGetInteger(set->fonts[0], FC_SPACING, faceNo, &itsSpacing);
            itsFoundry=getFcString(set->fonts[0], FC_FOUNDRY, faceNo);
        }
    }

    itsIndex=0; // Doesn't matter, as we're gonna use font name!
    itsLastUrl=url;
    return true;
}

XftFont * CFcEngine::queryFont()
{
    static const int constQuerySize=8;

#ifdef KFI_FC_DEBUG
    KFI_DBUG << "queryFont" << endl;
#endif

    XftFont *f=getFont(constQuerySize);

    if(!isCorrect(f, true))
    {
        XftFontClose(QX11Info::display(), f);
        f=NULL;
    }

    if(itsInstalled && !f)
    {
        // Perhaps its a newly installed font? If so try re-initialising fontconfig...
        itsFcDirty=true;
        reinit();

        f=getFont(constQuerySize);

        // This time dont bother checking family - we've re-inited fc anyway, so things should be
        // up to date... And for "Symbol" Fc returns "Standard Symbols L", so wont match anyway!
        if(f && !isCorrect(f, false))
        {
            XftFontClose(QX11Info::display(), f);
            f=NULL;
        }
    }
#ifdef KFI_FC_DEBUG
    KFI_DBUG << "queryFont - ret" << (int)f << endl;
#endif
    return f;
}

XftFont * CFcEngine::getFont(int size)
{
    XftFont *f=NULL;

#ifdef KFI_FC_DEBUG
    KFI_DBUG << "getFont:" << QString(itsInstalled ? itsName : itsFileName) << ' ' << size << endl;
#endif

    if(itsInstalled)
    {
#ifndef KFI_FC_NO_WIDTHS
        if(KFI_NULL_SETTING!=itsWidth)
            f=XftFontOpen(QX11Info::display(), 0,
                          FC_FAMILY, FcTypeString, (const FcChar8 *)(itsName.toUtf8().data()),
                          FC_WEIGHT, FcTypeInteger, itsWeight,
                          FC_SLANT, FcTypeInteger, itsSlant,
                          FC_WIDTH, FcTypeInteger, itsWidth,
                          FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                          NULL);
        else
#endif
            f=XftFontOpen(QX11Info::display(), 0,
                          FC_FAMILY, FcTypeString, (const FcChar8 *)(itsName.toUtf8().data()),
                          FC_WEIGHT, FcTypeInteger, itsWeight,
                          FC_SLANT, FcTypeInteger, itsSlant,
                          FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                          NULL);
    }
    else
    {
        FcPattern *pattern = FcPatternBuild(NULL,
                                            FC_FILE, FcTypeString,
                                                     QFile::encodeName(itsFileName).data(),
                                            FC_INDEX, FcTypeInteger, itsIndex<0 ? 0 : itsIndex,
                                            FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                                            NULL);
        f=XftFontOpenPattern(QX11Info::display(), pattern);
    }

#ifdef KFI_FC_DEBUG
    KFI_DBUG << "getFont, ret: " << (int)f << endl;
#endif

    return f;
}

bool CFcEngine::isCorrect(XftFont *f, bool checkFamily)
{
    int     iv;
    FcChar8 *str;

#ifdef KFI_FC_DEBUG
    QString     xxx;
    QTextStream s(&xxx);
    if(f)
    {
        if(itsInstalled)
        {
            s << "weight:";
            if(FcResultMatch==FcPatternGetInteger(f->pattern, FC_WEIGHT, 0, &iv))
               s << iv << '/' << itsWeight;
            else
                s << "no";

            s << " slant:";
            if(FcResultMatch==FcPatternGetInteger(f->pattern, FC_SLANT, 0, &iv))
                s << iv << '/' << itsSlant;
            else
                s << "no";

            s << " width:";
            if(FcResultMatch==FcPatternGetInteger(f->pattern, FC_WIDTH, 0, &iv))
                s << iv << '/' << itsWidth;
            else
                s << "no";

            s << " fam:";
            if(checkFamily)
                if(FcResultMatch==FcPatternGetString(f->pattern, FC_FAMILY, 0, &str) && str)
                    s << QString::fromUtf8((char *)str) << '/' << itsName;
                else
                    s << "no";
            else
                s << "ok";
            fprintf(stderr, "  ");
        }
        else
            s << "NOT Installed...   ";
    }
    else
        s << "No font!!!  ";
    KFI_DBUG << "isCorrect? " << xxx << endl;
#endif

    return
        f
            ? itsInstalled
                ? FcResultMatch==FcPatternGetInteger(f->pattern, FC_WEIGHT, 0, &iv) &&
                   equalWeight(iv, itsWeight) &&
                  FcResultMatch==FcPatternGetInteger(f->pattern, FC_SLANT, 0, &iv) &&
                   equalSlant(iv, itsSlant) &&
    #ifndef KFI_FC_NO_WIDTHS
                  (KFI_NULL_SETTING==itsWidth ||
                    (FcResultMatch==FcPatternGetInteger(f->pattern, FC_WIDTH, 0, &iv) &&
                     equalWidth(iv, itsWidth))) &&
    #endif
                  (!checkFamily ||
                    (FcResultMatch==FcPatternGetString(f->pattern, FC_FAMILY, 0, &str) && str &&
                     QString::fromUtf8((char *)str)==itsName))
                : FcResultMatch==FcPatternGetInteger(f->pattern, FC_INDEX, 0, &iv) &&
                  itsIndex==iv &&
                  FcResultMatch==FcPatternGetString(f->pattern, FC_FILE, 0, &str) && str &&
                  QString::fromUtf8((char *)str)==itsFileName
            : false;
}

void CFcEngine::getSizes()
{
#ifdef KFI_FC_DEBUG
    KFI_DBUG << "getSizes" << endl;
#endif

    XftFont *f=queryFont();

    itsScalable=FcTrue;

    itsSizes.clear();
    itsAlphaSize=0;

    if(f)
    {
        bool   gotSizes=false;
        double px(0.0);

        if(itsInstalled)
        {
            if(FcResultMatch!=FcPatternGetBool(f->pattern, FC_SCALABLE, 0, &itsScalable))
                itsScalable=FcFalse;

            if(!itsScalable)
            {
                FcPattern   *pat=NULL;
                FcObjectSet *os  = FcObjectSetBuild(FC_PIXEL_SIZE, (void*)0);
#ifndef KFI_FC_NO_WIDTHS
                if(KFI_NULL_SETTING!=itsWidth)
                    pat=FcPatternBuild(NULL,
                                   FC_FAMILY, FcTypeString,
                                        (const FcChar8 *)(itsName.toUtf8().data()),
                                   FC_WEIGHT, FcTypeInteger, itsWeight,
                                   FC_SLANT, FcTypeInteger, itsSlant,
                                   FC_WIDTH, FcTypeInteger, itsWidth,
                                   NULL);
                else
#endif
                    pat=FcPatternBuild(NULL,
                                   FC_FAMILY, FcTypeString,
                                        (const FcChar8 *)(itsName.toUtf8().data()),
                                   FC_WEIGHT, FcTypeInteger, itsWeight,
                                   FC_SLANT, FcTypeInteger, itsSlant,
                                   NULL);

                FcFontSet *set=FcFontList(0, pat, os);

                FcPatternDestroy(pat);
                FcObjectSetDestroy(os);

                if (set)
                {
#ifdef KFI_FC_DEBUG
                    KFI_DBUG << "got fixed sizes: " << set->nfont << endl;
#endif
                    itsSizes.reserve(set->nfont);
                    for (int i = 0; i < set->nfont; i++)
                        if(FcResultMatch==FcPatternGetDouble(set->fonts[i], FC_PIXEL_SIZE, 0, &px))
                        {
                            gotSizes=true;
                            itsSizes.push_back((int)px);

#ifdef KFI_FC_DEBUG
                            KFI_DBUG << "got fixed: " << px << endl;
#endif
                            if (px<=constDefaultAlphaSize)
                                itsAlphaSize=(int)px;
                        }
                    FcFontSetDestroy(set);
                }
            }
        }
        else
        {
            FT_Face face=XftLockFace(f);

            if(face)
            {
                itsIndexCount=face->num_faces;
                if(!(itsScalable=FT_IS_SCALABLE(face)))
                {
                    int numSizes=face->num_fixed_sizes,
                        size;

                    gotSizes=true;

                    itsSizes.reserve(numSizes);

#ifdef KFI_FC_DEBUG
                    KFI_DBUG << "numSizes fixed: " << numSizes << endl;
#endif
                    for (size=0; size<numSizes; size++)
                    {
#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
                        double px=face->available_sizes[size].y_ppem>>6;
#else
                        double px=face->available_sizes[size].width;
#endif
#ifdef KFI_FC_DEBUG
                        KFI_DBUG << "px: " << px << endl;
#endif
                        itsSizes.push_back((int)px);

                        if (px<=constDefaultAlphaSize)
                            itsAlphaSize=(int)px;
                    }
                }
                XftUnlockFace(f);
            }
        }

        XftFontClose(QX11Info::display(), f);
    }

    if(itsScalable)
    {
        itsSizes.reserve(sizeof(constScalableSizes)/sizeof(int));

        for (int i=0; constScalableSizes[i]; ++i)
            itsSizes.push_back(point2Pixel(constScalableSizes[i]));
        itsAlphaSize=constDefaultAlphaSize;
    }

    if(0==itsAlphaSize && itsSizes.count())
        itsAlphaSize=itsSizes[0];
#ifdef KFI_FC_DEBUG
    KFI_DBUG << "getSizes, end" << endl;
#endif
}

void CFcEngine::drawName(QPainter &painter, int x, int &y, int w, int offset)
{
    QString title(itsDescriptiveName.isEmpty()
                    ? i18n("ERROR: Could not determine font's name.")
                    : itsDescriptiveName);

    if(1==itsSizes.size())
        title=i18np("%1 [1 pixel]", "%1 [%n pixels]", itsSizes[0], title);

    painter.setFont(KGlobalSettings::generalFont());
    painter.setPen(theirTextCol);
    y=painter.fontMetrics().height();
    drawText(painter, x, y, w-offset, title);
    y+=4;
    painter.drawLine(offset, y, w-(offset+1), y);
    y+=8;
}

void CFcEngine::addFontFile(const QString &file)
{
    if(!itsAddedFiles.contains(file))
    {
        FcInitReinitialize();
        FcConfigAppFontAddFile(FcConfigGetCurrent(), (const FcChar8 *)(QFile::encodeName(file).data()));
        itsAddedFiles.append(file);
    }
}

void CFcEngine::reinit()
{
    if(itsFcDirty)
    {
        FcInitLoadConfigAndFonts();
        FcInitReinitialize();
        itsFcDirty=false;
    }
}

}
