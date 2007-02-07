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
#include <kstaticdeleter.h>
#include <kglobal.h>
#include <klocale.h>
#include <math.h>
#include "FcEngine.h"
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <fixx11h.h>

//#define KFI_FC_DEBUG

#ifdef KFI_FC_DEBUG
#define KFI_DBUG kDebug() << "[" << (int)(getpid()) << "] CFcEngine - "
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

static KStaticDeleter<CFcEngine> staticDel;
static CFcEngine                 *theInstance=NULL;

const int CFcEngine::constScalableSizes[]={8, 10, 12, 24, 36, 48, 64, 72, 96, 0 };
const int CFcEngine::constDefaultAlphaSize=24;

QColor CFcEngine::theirBgndCol(Qt::white);
QColor CFcEngine::theirTextCol(Qt::black);

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

static bool fcToQtSlant(int slant)
{
    return FC_SLANT_ROMAN==slant ? false : true;
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
    return a==b || FC::weight(a)==FC::weight(b);
}

#ifndef KFI_FC_NO_WIDTHS
inline bool equalWidth(int a, int b)
{
    return a==b || FC::width(a)==FC::width(b);
}
#endif

inline bool equalSlant(int a, int b)
{
    return a==b || FC::slant(a)==FC::slant(b);
}

static bool drawChar32(XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, quint32 ch,
                       int &x, int &y, int w, int h, int fSize, int offset)
{
    static const int constBorder=2;

    if(XftCharExists(QX11Info::display(), xftFont, ch))
    {
        XGlyphInfo extents;

        XftTextExtents32(QX11Info::display(), xftFont, &ch, 1, &extents);

        if(x+extents.width+constBorder>w)
        {
            x=offset;
            y+=fSize+constBorder;
        }

        if(y+offset<h)
        {
            XftDrawString32(xftDraw, xftCol, xftFont, x, y, &ch, 1);
            x+=extents.width+constBorder;
            return true;
        }
        return false;
    }

    return true;
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
        staticDel.setObject(theInstance, new CFcEngine);

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
                     const QList<TRange> &range, const QString &name, unsigned long style)
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
                    QString text(i18nc("All letters of the alphabet (in upper/lower case pairs), followed by numbers",
                                       "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));

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

                    xftFont=getFont(fSize);

                    y=fSize;
                    if(xftFont)
                    {
                        QString valid(usableStr(xftFont, text));

                        rv=true;

                        if(valid.length()<(text.length()/2))
                            drawAllGlyphs(xftDraw, xftFont, &xftCol, fSize, x, y, w, h, offset);
                        else
                        {
                            QVector<uint> ucs4(valid.toUcs4());

                            for(int ch=0; ch<ucs4.size(); ++ch) // Display char by char so wraps...
                                if(!drawChar32(xftDraw, xftFont, &xftCol, ucs4[ch], x, y, w, h, fSize,
                                               offset))
                                    break;
                        }
                    }
                }
                else if(0==range.count())
                {
                    QString lowercase(getLowercaseLetters()),
                            uppercase(getUppercaseLetters()),
                            punctuation(getPunctuation());

                    drawName(painter, x, y, w, offset);

                    xftFont=getFont(itsAlphaSize);
                    if(xftFont)
                    {
                        bool lc(hasStr(xftFont, lowercase)),
                             uc(hasStr(xftFont, uppercase)),
                             drawGlyphs=!lc && !uc;

                        if(drawGlyphs)
                            y-=8;
                        else
                        {
                            QString validPunc(usableStr(xftFont, punctuation));
                            bool    punc(validPunc.length()>=(punctuation.length()/2));

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
                else if(1==range.count() && range.first().null())
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
                else
                {
                    QList<TRange>::ConstIterator it(range.begin()),
                                                 end(range.end());

                    if((xftFont=getFont(itsAlphaSize)))
                    {
                        rv=true;
                        drawName(painter, x, y, w, offset);
                        y+=itsAlphaSize;

                        bool stop=false;
                        int  xOrig(x), yOrig(y);

                        for(it=range.begin(); it!=end && !stop; ++it)
                            for(quint32 c=(*it).from; c<=(*it).to && !stop; ++c)
                                if(!drawChar32(xftDraw, xftFont, &xftCol, c, x, y, w, h, itsAlphaSize,
                                               offset))
                                    stop=true;

                        if(x==xOrig && y==yOrig)
                        {
                            // No characters found within the selected range...
                            painter.setFont(KGlobalSettings::generalFont());
                            painter.setPen(theirTextCol);
                            drawText(painter, x, y, w-offset, i18n("No characters found."));
                        }
                    }
                }

                XftDrawDestroy(xftDraw);
            }
        }
    }

    return rv;
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
        weight=FC::weightStr(itsWeight, false);
        width=FC::widthStr(itsWidth, false);
        slant=FC::slantStr(itsSlant, false);
        spacing=FC::spacingStr(itsSpacing);
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

    FC::decomposeStyleVal(style, weight, width, slant);

    QFont font(family, size, fcToQtWeight(weight), fcToQtSlant(slant));

#ifndef KFI_FC_NO_WIDTHS
    font.setStretch(fcToQtWidth(width));
#endif
    return font;
}

bool CFcEngine::parseUrl(const KUrl &url, int faceNo, bool all)
{
#ifdef KFI_FC_DEBUG
    KFI_DBUG << "parseUrl(" << url.prettyUrl() << ", " << faceNo << ", " << all << ")" << endl;
#endif
    if(faceNo<0)
        faceNo=0;

    itsFileName.clear();
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
            style=FC::styleValFromStr(udsEntry.stringValue((uint)UDS_EXTRA_FC_STYLE));
            itsIndex=Misc::getIntQueryVal(KUrl(udsEntry.stringValue((uint)KIO::UDS_URL)),
                                          KFI_KIO_FACE, 0);
#ifdef KFI_FC_DEBUG
            KFI_DBUG << "Stated fonts:/ url, name:" << name << " itsFileName:" << itsFileName
                         << " style:" << style << " itsIndex:" << itsIndex << endl;
#endif
        }
#ifdef KFI_FC_DEBUG
KFI_DBUG << "isHidden:" << hidden << endl;
#endif
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
                    itsFoundry=FC::getFcString(pat, FC_FOUNDRY, 0);
                }
                itsDescriptiveName=FC::createName(pat, itsWeight, itsWidth, itsSlant);
                FcPatternDestroy(pat);
            }
            else
            {
                itsDescriptiveName.clear();
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
            FC::decomposeStyleVal(style, itsWeight, itsWidth, itsSlant);
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
            FC::decomposeStyleVal(style, itsWeight, itsWidth, itsSlant);
        else
        {
            QString style(name.mid(pos+2));

            itsWeight=FC::strToWeight(style, style);
            itsWidth=FC::strToWidth(style, style);
            itsSlant=FC::strToSlant(style);
        }
        itsName=name.left(pos);
    }

    itsFileName.clear();
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
            itsFoundry=FC::getFcString(set->fonts[0], FC_FOUNDRY, faceNo);
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
