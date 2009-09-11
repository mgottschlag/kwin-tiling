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

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QX11Info>
#include <KDE/KUrl>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobalSettings>
#include <KDE/KIO/NetAccess>
#include <KDE/KGlobal>
#include <KDE/KLocale>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <fixx11h.h>

//#define KFI_FC_DEBUG

#define KFI_PREVIEW_GROUP      "KFontInst Preview Settings"
#define KFI_PREVIEW_STRING_KEY "String"

namespace KFI
{

bool      CFcEngine::theirFcDirty(true);
const int CFcEngine::constScalableSizes[]={8, 10, 12, 24, 36, 48, 64, 72, 96, 0 };
const int CFcEngine::constDefaultAlphaSize=24;

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

static void closeFont(XftFont *&font)
{
    if(font)
        XftFontClose(QX11Info::display(), font);
    font=0L;
}

class CFcEngine::Xft
{
    public:

    struct Pix
    {
        Pix() : currentW(0), currentH(0), allocatedW(0), allocatedH(0) { }

        static int getSize(int s)
        {
            static const int constBlockSize=64;

            return ((s/constBlockSize)+(s%constBlockSize ? 1 : 0))*constBlockSize;
        }

        bool allocate(int w, int h)
        {
            int requiredW=getSize(w),
                requiredH=getSize(h);

            currentW=w;
            currentH=h;
            if(requiredW!=allocatedW || requiredH!=allocatedH)
            {
                free();

                if(w && h)
                {
                    allocatedW=requiredW;
                    allocatedH=requiredH;
                    x11=XCreatePixmap(QX11Info::display(), RootWindow(QX11Info::display(), 0), allocatedW, allocatedH,
                                      DefaultDepth(QX11Info::display(), 0));
                    return true;
                }
            }

            return false;
        }

        void free()
        {
            if(allocatedW && allocatedH)
            {
                XFreePixmap(QX11Info::display(), x11);
                allocatedW=allocatedH=0;
            }
        }
        
        int    currentW,
               currentH,
               allocatedW,
               allocatedH;
        Pixmap x11;
    };
    
    Xft();
    ~Xft();

    bool init(const QColor &txt, const QColor &bgnd, int w, int h);
    void freeColors();
    bool drawChar32Centre(XftFont *xftFont, quint32 ch, int w, int h) const;
    bool drawChar32(XftFont *xftFont, quint32 ch,int &x, int &y, int w, int h,
                    int fontHeight, QRect &r) const;
    bool drawString(XftFont *xftFont, const QString &text, int x, int &y, int h) const;
    void drawString(const QString &text, int x, int &y, int h) const;
    bool drawGlyph(XftFont *xftFont, FT_UInt i, int &x, int &y, int w, int h,
                   int fontHeight,bool oneLine, QRect &r) const;
    bool drawAllGlyphs(XftFont *xftFont, int fontHeight, int &x, int &y, int w, int h,
                       bool oneLine=false, int max=-1, QRect *used=0L) const;
    QImage toImage(int w=0, int h=0) const;

    private:

    XftDraw  *itsDraw;
    XftColor itsTxtColor,
             itsBgndColor;
    Pix      itsPix;
};

CFcEngine::Xft::Xft()
{
    itsDraw=0L;
    itsTxtColor.color.alpha=0x0000;
}

CFcEngine::Xft::~Xft()
{
    freeColors();
    if(itsDraw)
    {
        XftDrawDestroy(itsDraw);
        itsDraw=0L;
    }
}

bool CFcEngine::Xft::init(const QColor &txt, const QColor &bnd, int w, int h)
{
    if(itsDraw &&
       (txt.red()<<8 != itsTxtColor.color.red ||
        txt.green()<<8 != itsTxtColor.color.green ||
        txt.blue()<<8 != itsTxtColor.color.blue ||
        bnd.red()<<8 != itsBgndColor.color.red ||
        bnd.green()<<8 != itsBgndColor.color.green ||
        bnd.blue()<<8 != itsBgndColor.color.blue))
        freeColors();

    if(0x0000==itsTxtColor.color.alpha)
    {
        XRenderColor xrenderCol;
        Visual       *visual=DefaultVisual(QX11Info::display(), 0);
        Colormap     colorMap=DefaultColormap(QX11Info::display(), 0);

        xrenderCol.red=bnd.red()<<8;
        xrenderCol.green=bnd.green()<<8;
        xrenderCol.blue=bnd.green()<<8;
        xrenderCol.alpha=0xFFFF;

        XftColorAllocValue(QX11Info::display(), visual, colorMap, &xrenderCol, &itsBgndColor);
        xrenderCol.red=txt.red()<<8;
        xrenderCol.green=txt.green()<<8;
        xrenderCol.blue=txt.green()<<8;
        xrenderCol.alpha=0xFFFF;
        XftColorAllocValue(QX11Info::display(), visual, colorMap, &xrenderCol, &itsTxtColor);
    }

    if(itsPix.allocate(w, h) && itsDraw)
        XftDrawChange(itsDraw, itsPix.x11);

    if(!itsDraw)
        itsDraw=XftDrawCreate(QX11Info::display(), itsPix.x11, DefaultVisual(QX11Info::display(), 0),
                              DefaultColormap(QX11Info::display(), 0));

    if(itsDraw)
        XftDrawRect(itsDraw, &itsBgndColor, 0, 0, w, h);

    return itsDraw;
}

void CFcEngine::Xft::freeColors()
{
    XftColorFree(QX11Info::display(), DefaultVisual(QX11Info::display(), 0),
                 DefaultColormap(QX11Info::display(), 0), &itsTxtColor);
    XftColorFree(QX11Info::display(), DefaultVisual(QX11Info::display(), 0),
                 DefaultColormap(QX11Info::display(), 0), &itsBgndColor);
    itsTxtColor.color.alpha=0x0000;
}

bool CFcEngine::Xft::drawChar32Centre(XftFont *xftFont, quint32 ch, int w, int h) const
{
    if(XftCharExists(QX11Info::display(), xftFont, ch))
    {
        XGlyphInfo extents;

        XftTextExtents32(QX11Info::display(), xftFont, &ch, 1, &extents);

        int rx(((w-extents.width)/2)+extents.x),
            ry(((h-extents.height)/2)+(extents.y));

        XftDrawString32(itsDraw, &itsTxtColor, xftFont, rx, ry, &ch, 1);
        return true;
    }

    return false;
}

static const int constBorder=2;

bool CFcEngine::Xft::drawChar32(XftFont *xftFont, quint32 ch, int &x, int &y, int w, int h,
                                int fontHeight, QRect &r) const
{
    r=QRect();
    if(XftCharExists(QX11Info::display(), xftFont, ch))
    {
        XGlyphInfo extents;

        XftTextExtents32(QX11Info::display(), xftFont, &ch, 1, &extents);

        if(extents.x>0)
            x+=extents.x;

        if(x+extents.width+constBorder>w)
        {
            x=0;
            if(extents.x>0)
                x+=extents.x;
            y+=fontHeight+constBorder;
        }

        if(y<h)
        {
            r=QRect(x-extents.x, y-extents.y, extents.width+constBorder, extents.height);

            XftDrawString32(itsDraw, &itsTxtColor, xftFont, x, y, &ch, 1);
            x+=extents.xOff+constBorder;
            return true;
        }
        return false;
    }

    return true;
}

bool CFcEngine::Xft::drawString(XftFont *xftFont, const QString &text, int x, int &y, int h) const
{
    XGlyphInfo     extents;
    const FcChar16 *str=(FcChar16 *)(text.utf16());

    XftTextExtents16(QX11Info::display(), xftFont, str, text.length(), &extents);
    if(y+extents.height<h)
        XftDrawString16(itsDraw, &itsTxtColor, xftFont, x, y+extents.y, str, text.length());
    if(extents.height>0)
    {
        y+=extents.height;
        return true;
    }
    return false;
}

void CFcEngine::Xft::drawString(const QString &text, int x, int &y, int h) const
{
    QFont   qt(KGlobalSettings::generalFont());
    XftFont *xftFont=XftFontOpen(QX11Info::display(), 0,
                                 FC_FAMILY, FcTypeString, (const FcChar8 *)(qt.family().toUtf8().data()),
                                 FC_WEIGHT, FcTypeInteger, qt.bold() ? FC_WEIGHT_BOLD : FC_WEIGHT_REGULAR,
                                 FC_SLANT, FcTypeInteger, qt.italic() ? FC_SLANT_ITALIC : FC_SLANT_ROMAN,
                                 FC_SIZE, FcTypeDouble, (double)qt.pointSize(),
                                 NULL);

    if(xftFont)
    {
        drawString(xftFont, text, x, y, h);
        closeFont(xftFont);
    }
}

bool CFcEngine::Xft::drawGlyph(XftFont *xftFont, FT_UInt i, int &x, int &y, int w, int h, int fontHeight,
                               bool oneLine, QRect &r) const
{
    XGlyphInfo extents;

    XftGlyphExtents(QX11Info::display(), xftFont, &i, 1, &extents);

    if(x+extents.width+2>w)
    {
        if(oneLine)
            return false;

        x=0;
        y+=fontHeight+2;
    }

    if(y<h)
    {
        XftDrawGlyphs(itsDraw, &itsTxtColor, xftFont, x, y, &i, 1);
        x+=extents.width+2;
        r=QRect(x-extents.x, y-extents.y, extents.width+constBorder, extents.height);

        return true;
    }
    return false;
}

bool CFcEngine::Xft::drawAllGlyphs(XftFont *xftFont, int fontHeight, int &x, int &y, int w, int h,
                                   bool oneLine, int max, QRect *used) const
{
    bool rv(false);

    if(xftFont)
    {
        FT_Face face=XftLockFace(xftFont);

        if(face)
        {
            int   space(fontHeight/10),
                  drawn(1);
            QRect r;

            if(!space)
                space=1;

            rv=true;
            y+=fontHeight;
            for(int i=1; i<face->num_glyphs && y<h; ++i)
                if(drawGlyph(xftFont, i, x, y, w, h, fontHeight, oneLine, r))
                {
                    if(r.height()>0)
                    {
                        if(used)
                        {
                            if(used->isEmpty())
                                *used=r;
                            else
                                *used=used->united(r);
                        }
                        if(max>0 && ++drawn>=max)
                            break;
                    }
                }
                else
                    break;

            if(oneLine)
                x=0;
            XftUnlockFace(xftFont);
        }
    }

    return rv;
}

QImage CFcEngine::Xft::toImage(int w, int h) const
{
    int imgW=w ? w : itsPix.currentW,
        imgH=h ? h : itsPix.currentH;

    if(!XftDrawPicture(itsDraw))
        return QImage();

    XImage *xi = XGetImage(QX11Info::display(), itsPix.x11, 0, 0, imgW, imgH, AllPlanes, ZPixmap);

    if (!xi)
        return QImage();

    QImage image(imgW, imgH, QImage::Format_RGB32);
    bool   xOk=32==xi->bits_per_pixel;

    if(xOk)
        memcpy(image.bits(), xi->data, xi->bytes_per_line*xi->height);

    if (xi->data)
    {
        free(xi->data);
        xi->data = 0;
    }
    XDestroyImage(xi);

    return xOk ? image : QImage();
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

static void setTransparentBackground(QImage &img, const QColor &col)
{
    // Convert background to transparent, and text to correct colour...
    img=img.convertToFormat(QImage::Format_ARGB32);
    for(int x=0; x<img.width(); ++x)
        for(int y=0; y<img.height(); ++y)
        {
            int v(qRed(img.pixel(x, y)));
            img.setPixel(x, y, qRgba(qMin(col.red()+v, 255),
                                     qMin(col.green()+v, 255),
                                     qMin(col.blue()+v, 255),
                                     255-v));
        }
}

CFcEngine::CFcEngine(bool init)
         : itsIndex(-1),
           itsIndexCount(1),
           itsAlphaSizeIndex(-1),
           itsPreviewString(getDefaultPreviewString()),
           itsXft(0L)
{
    if(init)
        reinit();
}

CFcEngine::~CFcEngine()
{
    // Clear any fonts that may have been added...
    FcConfigAppFontClear(FcConfigGetCurrent());
    delete itsXft;
}

void CFcEngine::readConfig(KConfig &cfg)
{
    cfg.group(KFI_PREVIEW_GROUP).readEntry(KFI_PREVIEW_STRING_KEY, getDefaultPreviewString());
}

void CFcEngine::writeConfig(KConfig &cfg)
{
    cfg.group(KFI_PREVIEW_GROUP).writeEntry(KFI_PREVIEW_STRING_KEY, itsPreviewString);
}

const QString & CFcEngine::getName(const KUrl &url, int faceNo)
{
    if(url!=itsLastUrl || faceNo!=itsIndex)
        parseUrl(url, faceNo);

    return itsDescriptiveName;
}

QImage CFcEngine::drawPreview(const QString &item, const QColor &txt, const QColor &bgnd, int h, quint32 style, int face)
{
    QImage img;

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
            parseName(item, style);
            itsInstalled=true;
        }

        if(ok)
        {
            itsLastUrl=KUrl();
            getSizes();
        }

        if(itsSizes.size())
        {
            //
            // Calculate size of text...
            int  fSize=((int)(h*0.75))-2,
                 origHeight(0);
            bool needAlpha(bgnd.alpha()<255);

            if(!itsScalable) // Then need to get nearest size...
            {
                int bSize=0;

                for(int s=0; s<itsSizes.size(); ++s)
                    if (itsSizes[s]<=fSize || 0==bSize)
                        bSize=itsSizes[s];
                fSize=bSize;
                if(bSize>h)
                {
                    origHeight=h;
                    h=bSize+8;
                }
            }

            if(xft()->init(needAlpha ? Qt::black : txt, needAlpha ? Qt::white : bgnd, constInitialWidth, h))
            {
                XftFont *xftFont=getFont(fSize);
                QString text(itsPreviewString);

                if(xftFont)
                {
                    bool rv=false;
                    int  usedWidth=0;

                    if(hasStr(xftFont, text) || hasStr(xftFont, text=text.toUpper()) ||
                       hasStr(xftFont, text=text.toLower()))
                    {
                        XGlyphInfo     extents;
                        const FcChar16 *str=(FcChar16 *)(text.utf16());

                        XftTextExtents16(QX11Info::display(), xftFont, str, text.length(),
                                         &extents);

                        int y=(h-extents.height)/2;

                        rv=xft()->drawString(xftFont, text, constOffset, y, h);
                        usedWidth=extents.width;
                    }
                    else
                    {
                        int   x=constOffset,
                              y=constOffset;
                        QRect used;

                        rv=xft()->drawAllGlyphs(xftFont, fSize, x, y, constInitialWidth, h, true, text.length()+1, &used);
                        if(rv)
                            usedWidth=used.width();
                    }

                    if(rv)
                    {
                        img=xft()->toImage();
                        if(!img.isNull())
                        {
                            if(origHeight)
                            {
                                int width=(int)((usedWidth*(double)(((double)h)/((double)origHeight)))+0.5);
                                img=img.scaledToHeight(origHeight, Qt::SmoothTransformation)
                                       .copy(0, 0, width+(2*constOffset)<constInitialWidth
                                                    ? width+(2*constOffset)
                                                    : constInitialWidth,
                                             origHeight);
                            }
                            else
                                img=img.copy(0, 0, usedWidth+(2*constOffset)<constInitialWidth
                                                    ? usedWidth+(2*constOffset)
                                                    : constInitialWidth,
                                             h);

                            if(needAlpha)
                                setTransparentBackground(img, txt);
                        }
                    }
                    closeFont(xftFont);
                }
            }
        }
    }

    return img;
}

QImage CFcEngine::draw(const KUrl &url, int w, int h, const QColor &txt, const QColor &bgnd, int faceNo,
                       bool thumb, const QList<TRange> &range, QList<TChar> *chars, const QString &name, quint32 style)
{
    QImage img;
    bool   rv=false;

    if(chars)
        chars->clear();

    if((url==itsLastUrl && faceNo==itsIndex) ||
        (!name.isEmpty() && parseName(name, style, url)) ||
        parseUrl(url, faceNo))
    {
        if(!name.isEmpty())
            itsInstalled=true;

        if(!itsInstalled)  // Then add to fontconfig's list, so that Xft can display it...
            addFontFile(itsFileName);

        //
        // We allow kio_thumbnail to cache our thumbs. Normal is 128x128, and large is 256x256
        // ...if kio_thumbnail asks us for a bigger size, then it is probably the file info dialog, in
        // which case treat it as a normal preview...
        if(thumb && (h>256 || w!=h))
            thumb=false;

        int x=0, y=0;

        getSizes();

        if(itsSizes.size())
        {
            int  imgWidth(thumb && itsScalable ? w*4 : w),
                 imgHeight(thumb && itsScalable ? h*4 : h);
            bool needAlpha(bgnd.alpha()<255);

            if(xft()->init(needAlpha ? Qt::black : txt, needAlpha ? Qt::white : bgnd, imgWidth, imgHeight))
            {
                XftFont *xftFont=NULL;
                int     line1Pos(0),
                        line2Pos(0);
                QRect   used(0, 0, 0, 0);

                if(thumb)
                {
                    QString text(itsScalable
                                    ? i18nc("First letter of the alphabet (in upper then lower case)", "Aa")
                                    : i18nc("All letters of the alphabet (in upper/lower case pairs), followed by numbers",
                                            "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));

                    //
                    // Calculate size of text...
                    int fSize=h;

                    if(!itsScalable) // Then need to get nearest size...
                    {
                        int bSize=0;

                        for(int s=0; s<itsSizes.size(); ++s)
                            if (itsSizes[s]<=fSize || 0==bSize)
                                bSize=itsSizes[s];
                        fSize=bSize;
                    }

                    xftFont=getFont(fSize);

                    if(xftFont)
                    {
                        QString valid(usableStr(xftFont, text));

                        y=fSize;
                        rv=true;

                        if(itsScalable)
                        {
                            if(valid.length()!=text.length())
                            {
                                text=getPunctuation().mid(1, 2);  // '1' '2'
                                valid=usableStr(xftFont, text);
                            }
                        }
                        else
                            if(valid.length()<(text.length()/2))
                                for(int i=0; i<3; ++i)
                                {
                                    text=0==i ? getUppercaseLetters() : 1==i ? getLowercaseLetters() : getPunctuation();
                                    valid=usableStr(xftFont, text);

                                    if(valid.length()>=(text.length()/2))
                                        break;
                                }

                        if(itsScalable
                            ? valid.length()!=text.length()
                            : valid.length()<(text.length()/2))
                            xft()->drawAllGlyphs(xftFont, fSize, x, y, imgWidth, imgHeight, true,
                                                 itsScalable ? 4 : -1, itsScalable ? &used : NULL);
                        else
                        {
                            QVector<uint> ucs4(valid.toUcs4());
                            QRect         r;

                            for(int ch=0; ch<ucs4.size(); ++ch) // Display char by char so wraps...
                                if(xft()->drawChar32(xftFont, ucs4[ch], x, y, imgWidth, imgHeight, fSize, r))
                                {
                                    if(used.isEmpty())
                                        used=r;
                                    else
                                        used=used.united(r);
                                }
                                else
                                    break;
                        }

                        closeFont(xftFont);
                    }
                }
                else if(0==range.count())
                {
                    QString lowercase(getLowercaseLetters()),
                            uppercase(getUppercaseLetters()),
                            punctuation(getPunctuation());

                    drawName(x, y, h);
                    y+=4;
                    line1Pos=y;
                    y+=8;

                    xftFont=getFont(alphaSize());
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
                                xft()->drawString(xftFont, lowercase, x, y, h);
                            if(uc)
                                xft()->drawString(xftFont, uppercase, x, y, h);
                            if(punc)
                                xft()->drawString(xftFont, validPunc, x, y, h);
                            if(lc || uc || punc)
                                line2Pos=y+2;
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

                        closeFont(xftFont);
                        for(int s=0; s<itsSizes.size(); ++s)
                            if((xftFont=getFont(itsSizes[s])))
                            {
                                int fontHeight=xftFont->ascent+xftFont->descent;

                                rv=true;
                                if(drawGlyphs)
                                    xft()->drawAllGlyphs(xftFont, fontHeight, x, y, w, h,
                                                         itsSizes.count()>1);
                                else
                                    xft()->drawString(xftFont, previewString, x, y, h);
                                closeFont(xftFont);
                            }
                    }
                }
                else if(1==range.count() && (range.first().null() || 0==range.first().to))
                {
                    if(range.first().null())
                    {
                        drawName(x, y, h);

                        if((xftFont=getFont(alphaSize())))
                        {
                            int fontHeight=xftFont->ascent+xftFont->descent;

                            y-=8;
                            xft()->drawAllGlyphs(xftFont, fontHeight, x, y, w, h, 0);
                            rv=true;
                            closeFont(xftFont);
                        }
                    }
                    else if((xftFont=getFont(alphaSize()*2)))
                    {
                        QRect r;
                        rv=xft()->drawChar32Centre(xftFont, (*(range.begin())).from,
                                                   imgWidth, imgHeight);
                        closeFont(xftFont);
                    }
                }
                else
                {
                    QList<TRange>::ConstIterator it(range.begin()),
                                                 end(range.end());

                    if((xftFont=getFont(alphaSize())))
                    {
                        rv=true;
                        drawName(x, y, h);
                        y+=alphaSize();

                        bool  stop=false;
                        int   fontHeight=xftFont->ascent+xftFont->descent, xOrig(x), yOrig(y);
                        QRect r;

                        for(it=range.begin(); it!=end && !stop; ++it)
                            for(quint32 c=(*it).from; c<=(*it).to && !stop; ++c)
                            {
                                if(xft()->drawChar32(xftFont, c, x, y, w, h, fontHeight, r))
                                {
                                    if(chars && !r.isEmpty())
                                        chars->append(TChar(r, c));
                                }
                                else
                                    stop=true;
                            }

                        if(x==xOrig && y==yOrig)
                        {
                            // No characters found within the selected range...
                            xft()->drawString(i18n("No characters found."), x, y, h);
                            rv=true;
                        }
                        closeFont(xftFont);
                    }
                }

                if(rv)
                {
                    img=xft()->toImage();
                    if(!img.isNull() && line1Pos)
                    {
                        QPainter p(&img);

                        p.setPen(txt);
                        p.drawLine(0, line1Pos, w-1, line1Pos);
                        if(line2Pos)
                            p.drawLine(0, line2Pos, w-1, line2Pos);
                    }
                    if(!img.isNull())
                    {
                        if(itsScalable && !used.isEmpty() && used.width()<imgWidth && used.height()<imgHeight)
                            img=img.copy(used);
                        if(needAlpha)
                            setTransparentBackground(img, txt);
                    }
                }
            }
        }
    }

    return img;
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
    return i18nc("Numbers and characters", "0123456789.:,;(*!?'/\\\")£$€%^&-+@~#<>{}[]"); //krazy:exclude=i18ncheckarg
}

#ifdef KFI_USE_TRANSLATED_FAMILY_NAME
//
// Try to get the 'string' that matches the users KDE locale..
QString CFcEngine::getFcLangString(FcPattern *pat, const char *val, const char *valLang)
{
    QString                    rv;
    QStringList                kdeLangs=KGlobal::locale()->languageList(),
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

bool CFcEngine::getInfo(const KUrl &url, int faceNo, Misc::TFont &info)
{
    if((url==itsLastUrl && faceNo==itsIndex) || parseUrl(url, faceNo))
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

QFont CFcEngine::getQFont(const QString &family, quint32 style, int size)
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

bool CFcEngine::parseUrl(const KUrl &url, int faceNo)
{
#ifdef KFI_FC_DEBUG
    kDebug() << url.prettyUrl() << ' ' << faceNo;
#endif
    if(faceNo<0)
        faceNo=0;

    itsFileName.clear();
    itsIndex=faceNo;

    reinit();

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
        quint32       style=KFI_NO_STYLE_INFO;

        if(KIO::NetAccess::stat(url, udsEntry, NULL))  // Need to stat the url to get its font name...
        {
            name=udsEntry.stringValue((uint)KIO::UDSEntry::UDS_NAME);
            itsFileName=udsEntry.stringValue((uint)UDS_EXTRA_FILE_NAME);
            style=udsEntry.numberValue((uint)UDS_EXTRA_FC_STYLE);
            itsIndex=Misc::getIntQueryVal(KUrl(udsEntry.stringValue((uint)KIO::UDSEntry::UDS_URL)),
                                          KFI_KIO_FACE, 0);
#ifdef KFI_FC_DEBUG
            kDebug() << "Stated fonts:/ url, name:" << name << " itsFileName:" << itsFileName
                     << " style:" << style << " itsIndex:" << itsIndex;
#endif
        }
#ifdef KFI_FC_DEBUG
        kDebug() << "isHidden:" << hidden;
#endif
        if(hidden)
            name=itsFileName;

        if(!name.isEmpty())
        {
            if(hidden)
            {
                if(!parseUrl(KUrl(name), faceNo))
                    return false;
            }
            else
            {
                parseName(name, style);
                itsInstalled=true;
            }
        }
        else
            return false;
    }
    else if(url.isLocalFile() || url.protocol().isEmpty())
    {
        // Now lets see if it is from the thumbnail job! if so, then file should contain either:
        //    a. fonts:/ Url
        //    b. FontName followed by style info
        QFile file(url.toLocalFile());
        bool  isThumbnailUrl=false;

        if(file.size()<2048 && file.open(QIODevice::ReadOnly)) // Data should be less than 2k, and fonts usually above!
        {
            QTextStream stream(&file);
            QString     line1(stream.readLine()),
                        line2(stream.readLine());

            if(line2.isEmpty())
                isThumbnailUrl=(0==line1.indexOf(KFI_KIO_FONTS_PROTOCOL) ||
                                0==line1.indexOf("file:/")) &&
                                parseUrl(KUrl(line1), faceNo);
            else if(0==line1.indexOf(KFI_PATH_KEY) && 0==line2.indexOf(KFI_FACE_KEY))
            {
                line1=line1.mid(strlen(KFI_PATH_KEY));
                line2=line2.mid(strlen(KFI_FACE_KEY));

                if(!line1.isEmpty() && !line2.isEmpty())
                {
                    bool ok=false;
                    int  face=line2.toInt(&ok);

                    isThumbnailUrl=ok && parseUrl(line1, face<0 ? 0 : face);
                }
            }
            else if(0==line1.indexOf(KFI_NAME_KEY) && 0==line2.indexOf(KFI_STYLE_KEY))
            {
                line1=line1.mid(strlen(KFI_NAME_KEY));
                line2=line2.mid(strlen(KFI_STYLE_KEY));

                if(!line1.isEmpty() && !line2.isEmpty())
                {
                    bool    ok=false;
                    quint32 style=line2.toULong(&ok);

                    itsInstalled=isThumbnailUrl=ok && parseName(line1, style);

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

            if(pat)
            {
                FcPatternGetInteger(pat, FC_WEIGHT, 0, &itsWeight);
                FcPatternGetInteger(pat, FC_SLANT, 0, &itsSlant);
#ifndef KFI_FC_NO_WIDTHS
                FcPatternGetInteger(pat, FC_WIDTH, 0, &itsWidth);
#endif
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

bool CFcEngine::parseName(const QString &name, quint32 style, const KUrl &url)
{
    int pos;

    reinit();

    itsDescriptiveName=name;

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

    itsIndex=0; // Doesn't matter, as we're gonna use font name!
    itsLastUrl=url;
    return true;
}

XftFont * CFcEngine::queryFont()
{
    static const int constQuerySize=8;

#ifdef KFI_FC_DEBUG
    kDebug();
#endif

    XftFont *f=getFont(constQuerySize);

    if(f && !isCorrect(f, true))
        closeFont(f);

    if(itsInstalled && !f)
    {
        // Perhaps it is a newly installed font? If so try re-initialising fontconfig...
        theirFcDirty=true;
        reinit();

        f=getFont(constQuerySize);

        // This time don't bother checking family - we've re-inited fc anyway, so things should be
        // up to date... And for "Symbol" Fc returns "Standard Symbols L", so wont match anyway!
        if(f && !isCorrect(f, false))
            closeFont(f);
    }
#ifdef KFI_FC_DEBUG
    kDebug() << "ret" << (int)f;
#endif
    return f;
}

XftFont * CFcEngine::getFont(int size)
{
    XftFont *f=NULL;

#ifdef KFI_FC_DEBUG
    kDebug() << QString(itsInstalled ? itsName : itsFileName) << ' ' << size;
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
    kDebug() << "ret: " << (int)f;
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
    kDebug() << "isCorrect? " << xxx;
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
                : (itsIndex<0 || (FcResultMatch==FcPatternGetInteger(f->pattern, FC_INDEX, 0, &iv) && itsIndex==iv)) &&
                  FcResultMatch==FcPatternGetString(f->pattern, FC_FILE, 0, &str) && str &&
                  QString::fromUtf8((char *)str)==itsFileName
            : false;
}

void CFcEngine::getSizes()
{
#ifdef KFI_FC_DEBUG
    kDebug();
#endif
    XftFont *f=queryFont();
    int     alphaSize(itsSizes.size()>itsAlphaSizeIndex && itsAlphaSizeIndex>=0 ? itsSizes[itsAlphaSizeIndex] : constDefaultAlphaSize);

    itsScalable=FcTrue;

    itsSizes.clear();
    itsAlphaSizeIndex=0;

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
                    int size(0);
#ifdef KFI_FC_DEBUG
                    kDebug() << "got fixed sizes: " << set->nfont;
#endif
                    itsSizes.reserve(set->nfont);
                    for (int i = 0; i < set->nfont; i++)
                        if(FcResultMatch==FcPatternGetDouble(set->fonts[i], FC_PIXEL_SIZE, 0, &px))
                        {
                            gotSizes=true;
                            itsSizes.push_back((int)px);

#ifdef KFI_FC_DEBUG
                            kDebug() << "got fixed: " << px;
#endif
                            if (px<=alphaSize)
                                itsAlphaSizeIndex=size;
                            size++;
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
                    kDebug() << "numSizes fixed: " << numSizes;
#endif
                    for (size=0; size<numSizes; size++)
                    {
#if (FREETYPE_MAJOR*10000+FREETYPE_MINOR*100+FREETYPE_PATCH) >= 20105
                        double px=face->available_sizes[size].y_ppem>>6;
#else
                        double px=face->available_sizes[size].width;
#endif
#ifdef KFI_FC_DEBUG
                        kDebug() << "px: " << px;
#endif
                        itsSizes.push_back((int)px);

                        if (px<=alphaSize)
                            itsAlphaSizeIndex=size;
                    }
                }
                XftUnlockFace(f);
            }
        }

        closeFont(f);
    }

    if(itsScalable)
    {
        itsSizes.reserve(sizeof(constScalableSizes)/sizeof(int));

        for (int i=0; constScalableSizes[i]; ++i)
        {
            int px=point2Pixel(constScalableSizes[i]);

            if (px<=alphaSize)
                itsAlphaSizeIndex=i;
            itsSizes.push_back(px);
        }
    }

#ifdef KFI_FC_DEBUG
    kDebug() << "end";
#endif
}

void CFcEngine::drawName(int x, int &y, int h)
{
    QString title(itsDescriptiveName.isEmpty()
                    ? i18n("ERROR: Could not determine font's name.")
                    : itsDescriptiveName);

    if(1==itsSizes.size())
        title=i18np("%2 [1 pixel]", "%2 [%1 pixels]", itsSizes[0], title);

    xft()->drawString(title, x, y, h);
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
    if(theirFcDirty)
    {
        FcInitReinitialize();
        theirFcDirty=false;
    }
}

CFcEngine::Xft * CFcEngine::xft()
{
    if(!itsXft)
        itsXft=new Xft;
    return itsXft;
}

}
