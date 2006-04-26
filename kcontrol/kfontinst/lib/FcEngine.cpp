#include <qpainter.h>
#include <qpixmap.h>
#include <qfontmetrics.h>
#include <qfile.h>
#include <qtextstream.h>
#include <kurl.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kio/netaccess.h>
#include <math.h>
#include "FcEngine.h"
#include "KfiConstants.h"
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif

#define KFI_HAVE_OBLIQUE         // Do we differentiate between Italic and Oblique?
#define KFI_HAVE_MEDIUM_WEIGHT   // Do we differentiate between Medium and Normal weights?

#define KFI_PREVIEW_GROUP      "Preview Settings"
#define KFI_PREVIEW_STRING_KEY "String"

#ifdef HAVE_XFT
#define KFI_DISPLAY(pix) (pix ? pix->x11Display() : QPaintDevice::x11AppDisplay())
#endif

// CPD: TODO: should I use latin1, toLocal8Bit, or toUtf8?

namespace KFI
{

const int CFcEngine::constScalableSizes[]={8, 10, 12, 24, 36, 48, 64, 72, 96, 0 };
const int CFcEngine::constDefaultAlphaSize=24;

static int fcWeight(int weight)
{
    if(weight<FC_WEIGHT_ULTRALIGHT)
        return FC_WEIGHT_THIN;

    if(weight<(FC_WEIGHT_ULTRALIGHT+FC_WEIGHT_LIGHT)/2)
        return FC_WEIGHT_ULTRALIGHT;

    if(weight<(FC_WEIGHT_LIGHT+FC_WEIGHT_NORMAL)/2)
        return FC_WEIGHT_LIGHT;

#ifdef KFI_HAVE_MEDIUM_WEIGHT
    if(weight<(FC_WEIGHT_NORMAL+FC_WEIGHT_MEDIUM)/2)
        return FC_WEIGHT_NORMAL;

    if(weight<(FC_WEIGHT_MEDIUM+FC_WEIGHT_SEMIBOLD)/2)
        return FC_WEIGHT_MEDIUM;
#else
    if(weight<(FC_WEIGHT_NORMAL+FC_WEIGHT_SEMIBOLD)/2)
        return FC_WEIGHT_NORMAL;
#endif

    if(weight<(FC_WEIGHT_SEMIBOLD+FC_WEIGHT_BOLD)/2)
        return FC_WEIGHT_SEMIBOLD;

    if(weight<(FC_WEIGHT_BOLD+FC_WEIGHT_ULTRABOLD)/2)
        return FC_WEIGHT_BOLD;

    if(weight<(FC_WEIGHT_ULTRABOLD+FC_WEIGHT_HEAVY)/2)
        return FC_WEIGHT_ULTRABOLD;

    return FC_WEIGHT_HEAVY;
}

static int fcToQtWeight(int weight)
{
    switch(weight)
    {
        case FC_WEIGHT_THIN:
            return 0;
        case FC_WEIGHT_ULTRALIGHT:
            return QFont::Light>>1;
        case FC_WEIGHT_LIGHT:
            return QFont::Light;
        default:
        case FC_WEIGHT_NORMAL:
            return QFont::Normal;
        case FC_WEIGHT_MEDIUM:
#ifdef KFI_HAVE_MEDIUM_WEIGHT
            return (QFont::Normal+QFont::DemiBold)>>1;
#endif
            return QFont::Normal;
        case FC_WEIGHT_SEMIBOLD:
            return QFont::DemiBold;
        case FC_WEIGHT_BOLD:
            return QFont::Bold;
        case FC_WEIGHT_ULTRABOLD:
            return (QFont::Bold+QFont::Black)>>1;
        case FC_WEIGHT_HEAVY:
            return QFont::Black;
    }
}

#ifndef KFI_FC_NO_WIDTHS
static int fcWidth(int width)
{
    if(width<FC_WIDTH_EXTRACONDENSED)
        return FC_WIDTH_ULTRACONDENSED;

    if(width<(FC_WIDTH_EXTRACONDENSED+FC_WIDTH_CONDENSED)/2)
        return FC_WIDTH_EXTRACONDENSED;

    if(width<(FC_WIDTH_CONDENSED+FC_WIDTH_SEMICONDENSED)/2)
        return FC_WIDTH_CONDENSED;

    if(width<(FC_WIDTH_SEMICONDENSED+FC_WIDTH_NORMAL)/2)
        return FC_WIDTH_SEMICONDENSED;

    if(width<(FC_WIDTH_NORMAL+FC_WIDTH_SEMIEXPANDED)/2)
        return FC_WIDTH_NORMAL;

    if(width<(FC_WIDTH_SEMIEXPANDED+FC_WIDTH_EXPANDED)/2)
        return FC_WIDTH_SEMIEXPANDED;

    if(width<(FC_WIDTH_EXPANDED+FC_WIDTH_EXTRAEXPANDED)/2)
        return FC_WIDTH_EXPANDED;

    if(width<(FC_WIDTH_EXTRAEXPANDED+FC_WIDTH_ULTRAEXPANDED)/2)
        return FC_WIDTH_EXTRAEXPANDED;

    return FC_WIDTH_ULTRAEXPANDED;
}

static int fcToQtWidth(int weight)
{
    switch(weight)
    {
        case FC_WIDTH_ULTRACONDENSED:
            return QFont::UltraCondensed;
        case FC_WIDTH_EXTRACONDENSED:
            return QFont::ExtraCondensed;
        case FC_WIDTH_CONDENSED:
            return QFont::Condensed;
        case FC_WIDTH_SEMICONDENSED:
            return QFont::SemiCondensed;
        default:
        case FC_WIDTH_NORMAL:
            return QFont::Unstretched;
        case FC_WIDTH_SEMIEXPANDED:
            return QFont::SemiExpanded;
        case FC_WIDTH_EXPANDED:
            return QFont::Expanded;
        case FC_WIDTH_EXTRAEXPANDED:
            return QFont::ExtraExpanded;
        case FC_WIDTH_ULTRAEXPANDED:
            return QFont::UltraExpanded;
    }
}
#endif

static int fcSlant(int slant)
{
    if(slant<FC_SLANT_ITALIC)
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
    if(0==str.indexOf(i18n(KFI_WEIGHT_ULTRALIGHT), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_ULTRALIGHT).length());
        return FC_WEIGHT_ULTRALIGHT;
    }
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
    if(0==str.indexOf(i18n(KFI_WEIGHT_NORMAL), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_NORMAL).length());
        return FC_WEIGHT_NORMAL;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_MEDIUM), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_MEDIUM).length());
        return FC_WEIGHT_MEDIUM;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_DEMIBOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_DEMIBOLD).length());
        return FC_WEIGHT_SEMIBOLD;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_SEMIBOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_SEMIBOLD).length());
        return FC_WEIGHT_SEMIBOLD;
    }
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
    if(0==str.indexOf(i18n(KFI_WEIGHT_ULTRABOLD), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_ULTRABOLD).length());
        return FC_WEIGHT_ULTRABOLD;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_BLACK), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_BLACK).length());
        return FC_WEIGHT_BLACK;
    }
    if(0==str.indexOf(i18n(KFI_WEIGHT_HEAVY), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WEIGHT_HEAVY).length());
        return FC_WEIGHT_HEAVY;
    }

    newStr=str;
    return FC_WEIGHT_REGULAR;
}

#ifndef KFI_FC_NO_WIDTHS
static int strToWidth(const QString &str, QString &newStr)
{
    if(0==str.indexOf(i18n(KFI_WIDTH_ULTRACONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_ULTRACONDENSED).length());
        return FC_WIDTH_ULTRACONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXTRACONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXTRACONDENSED).length());
        return FC_WIDTH_EXTRACONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_CONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_CONDENSED).length());
        return FC_WIDTH_CONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_SEMICONDENSED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_SEMICONDENSED).length());
        return FC_WIDTH_SEMICONDENSED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_NORMAL), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_NORMAL).length());
        return FC_WIDTH_NORMAL;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_SEMIEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_SEMIEXPANDED).length());
        return FC_WIDTH_SEMIEXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXPANDED).length());
        return FC_WIDTH_EXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_EXTRAEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_EXTRAEXPANDED).length());
        return FC_WIDTH_EXTRAEXPANDED;
    }
    if(0==str.indexOf(i18n(KFI_WIDTH_ULTRAEXPANDED), 0, Qt::CaseInsensitive))
    {
        newStr=str.mid(i18n(KFI_WIDTH_ULTRAEXPANDED).length());
        return FC_WIDTH_ULTRAEXPANDED;
    }

    newStr=str;
    return FC_WIDTH_NORMAL;
}
#endif

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

#ifdef HAVE_XFT
static bool drawChar(QPixmap &pix, XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, const QString &text, int pos,
                     int &x, int &y, int w, int h, int fSize, int offset)
{
    XGlyphInfo     extents;
    const FcChar16 *str=(FcChar16 *)(&(text.utf16()[pos]));

    XftTextExtents16(pix.x11Display(), xftFont, str, 1, &extents);

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

static bool drawString(QPixmap &pix, XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, const QString &text,
                       int x, int &y, int h, int offset)
{
    XGlyphInfo     extents;
    const FcChar16 *str=(FcChar16 *)(text.utf16());

    XftTextExtents16(pix.x11Display(), xftFont, str, text.length(), &extents);
    if(y+extents.height<h)
        XftDrawString16(xftDraw, xftCol, xftFont, x, y+extents.y, str, text.length());
    if(extents.height>0)
    {
        y+=extents.height+offset;
        return true;
    }
    return false;
}

static bool drawGlyph(QPixmap &pix, XftDraw *xftDraw, XftFont *xftFont, XftColor *xftCol, FT_UInt i,
                      int &x, int &y, int &w, int &h, int fSize, int offset)
{
    XGlyphInfo extents;

    XftGlyphExtents(pix.x11Display(), xftFont, &i, 1, &extents);

    if(x+extents.width+2>w)
    {
        x=offset;
        y+=fSize;
    }

    if(y+offset<h)
    {
        XftDrawGlyphs(xftDraw, xftCol, xftFont, x, y, &i, 1);
        x+=extents.width+2;
        return true;
    }
    return false;
}

inline int point2Pixel(int point)
{
    return (point*QPaintDevice::x11AppDpiX()+36)/72;
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
#endif

CFcEngine::CFcEngine()
         : itsIndex(-1),
           itsIndexCount(1)
{
}

CFcEngine::~CFcEngine()
{
    // Clear any fonts that may have been added...
    FcConfigAppFontClear(FcConfigGetCurrent());
}

QString CFcEngine::getName(const KUrl &url, int faceNo)
{
    if(url!=itsLastUrl || faceNo!=itsIndex)
        parseUrl(url, faceNo);

    return itsDescriptiveName;
}

#ifdef HAVE_XFT
bool CFcEngine::draw(const KUrl &url, int w, int h, QPixmap &pix, int faceNo, bool thumb)
{
    bool rv=false;

    if((url==itsLastUrl && faceNo==itsIndex) || parseUrl(url, faceNo))
    {
        rv=true;

        if(!itsInstalled)  // Then add to fontconfig's list, so that Xft can display it...
        {
            FcInitReinitialize();
            FcConfigAppFontAddFile(FcConfigGetCurrent(), (const FcChar8 *)(itsName.toLatin1().data()));
        }

        if(thumb && (w!=h || h>128))
            thumb=false;

        int offset=thumb
                       ? h<=32
                             ? 2
                             : 3
                       : 4,
            x=offset, y=offset;

        pix = QPixmap(w, h);
        pix.fill(Qt::white);

        QPainter painter(&pix);

        getSizes(&pix);

        if(itsSizes.size())
        {
            XRenderColor xrenderCol;
            XftColor     xftCol;

            xrenderCol.red=xrenderCol.green=xrenderCol.blue=0;
            xrenderCol.alpha=0xffff;
            XftColorAllocValue(pix.x11Display(), DefaultVisual(pix.x11Display(),
                               pix.x11Screen()),
                               DefaultColormap(pix.x11Display(), pix.x11Screen()),
                               &xrenderCol, &xftCol);

            XftDraw *xftDraw=XftDrawCreate(pix.x11Display(), (Pixmap)(pix.handle()),
                                           (Visual*)(pix.x11Visual()), pix.x11Colormap());

            if(xftDraw)
            {
                XftFont *xftFont=NULL;
                bool    drawGlyphs=false;

                if(thumb)
                {
                    QString text(i18n("AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789"));

                    //
                    // Calculate size of text...
                    int fSize= h <= 32
                                ? h-(offset*2)          // 1 line of chars...
                                : h <= 64
                                        ? (h-(offset*3))/2   // 2 lines...
                                        : (h-(offset*4))/3;  // 3 lines or more

                    if(!itsScalable) // Then need to get nearest size...
                    {
                        int bSize=fSize;

                        for(int s=0; s<itsSizes.size(); ++s)
                            if (itsSizes[s]<=fSize)
                                bSize=itsSizes[s];
                        fSize=bSize;
                    }

                    int ch;

                    xftFont=getFont(fSize, &pix);

                    y=fSize;
                    if(xftFont)
                    {
                        drawGlyphs=!hasStr(xftFont, text);

                        if(!drawGlyphs)
                            for(ch=0; ch<text.length(); ++ch)   // Display char by char so that it wraps...
                                if(!drawChar(pix, xftDraw, xftFont, &xftCol, text, ch, x, y, w, h, fSize, offset))
                                    break;
                        if(drawGlyphs)
                        {
                            FT_Face face=XftLockFace(xftFont);

                            if(face)
                            {
                                for(int i=1; i<face->num_glyphs && y<w; ++i)  // Glyph 0 is the NULL glyph
                                    if(!drawGlyph(pix, xftDraw, xftFont, &xftCol, i, x, y, w, h, fSize, offset))
                                        break;

                                XftUnlockFace(xftFont);
                            }
                        }
                    }
                }
                else
                {
                    QString lowercase(getLowercaseLetters()),
                            uppercase(getUppercaseLetters()),
                            punctuation(getPunctuation()),
                            title(itsDescriptiveName.isEmpty()
                                    ? i18n("ERROR: Could not determine font's name.")
                                    : itsDescriptiveName);

                    if(1==itsSizes.size())
                        title=i18np("%1 [1 pixel]", "%1 [%n pixels]", itsSizes[0], title);

                    painter.setFont(KGlobalSettings::generalFont());
                    painter.setPen(Qt::black);
                    y=painter.fontMetrics().height();
                    drawText(painter, x, y, w-offset, title);
                    y+=4;
                    painter.drawLine(offset, y, w-(offset+1), y);
                    y+=8;

                    bool lc=true,
                         uc=true,
                         punc=true;

                    xftFont=getFont(itsAlphaSize, &pix);
                    if(xftFont)
                    {
                        lc=hasStr(xftFont, lowercase);
                        uc=hasStr(xftFont, uppercase);
                        punc=hasStr(xftFont, punctuation);

                        drawGlyphs=!lc && !uc;

                        if(!drawGlyphs)
                        {
                            if(lc)
                                drawString(pix, xftDraw, xftFont, &xftCol, lowercase, x, y, h, offset);
                            if(uc)
                                drawString(pix, xftDraw, xftFont, &xftCol, uppercase, x, y, h, offset);
                            if(punc)
                                drawString(pix, xftDraw, xftFont, &xftCol, punctuation, x, y, h, offset);
                            XftFontClose(pix.x11Display(), xftFont);
                            if(lc || uc || punc)
                                painter.drawLine(offset, y, w-(offset+1), y);
                            y+=8;
                        }

                        QString previewString(getPreviewString());
                        bool    stop=false;

                        if(!drawGlyphs)
                        {
                            if(!lc && uc)
                                previewString=previewString.toUpper();
                            if(!uc && lc)
                                previewString=previewString.toLower();
                        }

                        for(int s=0; s<itsSizes.size(); ++s)
                        {
                            xftFont=getFont(itsSizes[s], &pix);

                            if(xftFont)
                            {
                                if(drawGlyphs)
                                {
                                    FT_Face face=XftLockFace(xftFont);

                                    if(face)
                                    {
                                        int        space=itsSizes[s]/10;
                                        XGlyphInfo extents;

                                        if(!space)
                                            space=1;

                                        for(int i=1; i<face->num_glyphs && y<w && !stop; ++i)
                                        {
                                            XftGlyphExtents(pix.x11Display(), xftFont, (const FT_UInt *)&i, 1, &extents);

                                            if(y+extents.height>h)
                                                stop=true;
                                            else
                                            {
                                                if(x+extents.width<w)
                                                    XftDrawGlyphs(xftDraw, &xftCol, xftFont, x, y+extents.y,
                                                                  (const FT_UInt *)&i, 1);
                                                if(extents.width>0)
                                                    x+=extents.width+space;
                                            }
                                            if(x>=w || i==face->num_glyphs-1)
                                            {
                                                y+=itsSizes[s]+offset;
                                                x=offset;
                                                break;
                                            }
                                        }

                                        XftUnlockFace(xftFont);
                                    }
                                }
                                else
                                    drawString(pix, xftDraw, xftFont, &xftCol, previewString, x, y, h, offset);
                                XftFontClose(pix.x11Display(), xftFont);
                            }
                        }
                    }
                }

                XftDrawDestroy(xftDraw);
            }
        }
    }

    return rv;
}
#endif

QString CFcEngine::getPreviewString()
{
    KConfig cfg(KFI_UI_CFG_FILE);

    cfg.setGroup(KFI_PREVIEW_GROUP);

    QString str(cfg.readEntry(KFI_PREVIEW_STRING_KEY));

    return str.isEmpty() ? i18nc("A sentence that uses all of the letters of the alphabet",
                                "The quick brown fox jumps over the lazy dog")
                         : str;
}

void CFcEngine::setPreviewString(const QString &str)
{
    KConfig cfg(KFI_UI_CFG_FILE);

    cfg.setGroup(KFI_PREVIEW_GROUP);
    cfg.writeEntry(KFI_PREVIEW_STRING_KEY, str);
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

QString CFcEngine::getFcString(FcPattern *pat, const char *val, int faceNo)
{
    QString rv;
    FcChar8 *fcStr;

    if(FcResultMatch==FcPatternGetString(pat, val, faceNo, &fcStr))
        rv=QString::fromUtf8((char *)fcStr);

    return rv;
}

QString CFcEngine::createName(FcPattern *pat, int faceNo)
{
//CPD: TODO: the names *need* to match up with kfontchooser's...
    QString name(getFcString(pat, FC_FAMILY, faceNo)),
            str;
    int     intVal;
    bool    comma=false;

    if (FcResultMatch==FcPatternGetInteger(pat, FC_WEIGHT, faceNo, &intVal))
    {
        str=weightStr(intVal);
        if(!str.isEmpty())
        {
            name+=QString(", ")+str;
            comma=true;
        }
    }

    if (FcResultMatch==FcPatternGetInteger(pat, FC_SLANT, faceNo, &intVal))
    {
        str=slantStr(intVal);
        if(!str.isEmpty())
        {
            if(!comma)
            {
                name+=QChar(',');
                comma=true;
            }
            name+=QChar(' ')+str;
        }
    }

#ifndef KFI_FC_NO_WIDTHS
    if (FcResultMatch==FcPatternGetInteger(pat, FC_WIDTH, faceNo, &intVal))
    {
        str=widthStr(intVal);
        if(!str.isEmpty())
            name+=QChar(' ')+str;
    }
#endif

    return name;
}

QString CFcEngine::weightStr(int weight, bool emptyNormal)
{
    switch(fcWeight(weight))
    {
        case FC_WEIGHT_THIN:
            return i18n(KFI_WEIGHT_THIN);
        case FC_WEIGHT_ULTRALIGHT:
            return i18n(KFI_WEIGHT_ULTRALIGHT);
        case FC_WEIGHT_LIGHT:
            return i18n(KFI_WEIGHT_LIGHT);
        case FC_WEIGHT_NORMAL:
            return emptyNormal ? QString() : i18n(KFI_WEIGHT_NORMAL);
        case FC_WEIGHT_MEDIUM:
            return i18n(KFI_WEIGHT_MEDIUM);
        case FC_WEIGHT_DEMIBOLD:
            return i18n(KFI_WEIGHT_SEMIBOLD);
        case FC_WEIGHT_BOLD:
            return i18n(KFI_WEIGHT_BOLD);
        case FC_WEIGHT_ULTRABOLD:
            return i18n(KFI_WEIGHT_ULTRABOLD);
        default:
            return i18n(KFI_WEIGHT_HEAVY);
    }
}

#ifndef KFI_FC_NO_WIDTHS
QString CFcEngine::widthStr(int width, bool emptyNormal)
{
    switch(fcWidth(width))
    {
        case FC_WIDTH_ULTRACONDENSED:
            return i18n(KFI_WIDTH_ULTRACONDENSED);
        case FC_WIDTH_EXTRACONDENSED:
            return i18n(KFI_WIDTH_EXTRACONDENSED);
        case FC_WIDTH_CONDENSED:
            return i18n(KFI_WIDTH_CONDENSED);
        case FC_WIDTH_SEMICONDENSED:
            return i18n(KFI_WIDTH_SEMICONDENSED);
        case FC_WIDTH_NORMAL:
            return emptyNormal ? QString() : i18n(KFI_WIDTH_NORMAL);
        case FC_WIDTH_SEMIEXPANDED:
            return i18n(KFI_WIDTH_SEMIEXPANDED);
        case FC_WIDTH_EXPANDED:
            return i18n(KFI_WIDTH_EXPANDED);
        case FC_WIDTH_EXTRAEXPANDED:
            return i18n(KFI_WIDTH_EXTRAEXPANDED);
        default:
            return i18n(KFI_WIDTH_ULTRAEXPANDED);
    }
}
#endif

QString CFcEngine::slantStr(int slant, bool emptyNormal)
{
    switch(fcSlant(slant))
    {
        case FC_SLANT_OBLIQUE:
            return i18n(KFI_SLANT_OBLIQUE);
        case FC_SLANT_ITALIC:
            return i18n(KFI_SLANT_ITALIC);
        default:
            return emptyNormal ? QString() : i18n(KFI_SLANT_ROMAN);
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
#ifndef KFI_FC_NO_WIDTHS
                        QString &width,
#endif
                        QString &spacing, QString &slant)
{
    if(parseUrl(url, faceNo, true))
    {
        full=itsDescriptiveName;
        if(url.isLocalFile())
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
#ifndef KFI_FC_NO_WIDTHS
        width=widthStr(itsWidth, false);
#endif
        slant=slantStr(itsSlant, false);
        spacing=spacingStr(itsSpacing);
        foundry=itsFoundry;
        return true;
    }

    return false;
}

QFont CFcEngine::getQFont(const QString &name, int size)
{
    parseName(name, 0, false);

    QFont font(itsName, size, fcToQtWeight(itsWeight), fcToQtSlant(itsSlant));

#ifndef KFI_FC_NO_WIDTHS
    font.setStretch(fcToQtWidth(itsWidth));
#endif
    return font;
}

bool CFcEngine::parseUrl(const KUrl &url, int faceNo, bool all)
{
    FcInitLoadConfigAndFonts();

    // Possible urls:
    //
    //    fonts:/times.ttf
    //    fonts:/System/times.ttf
    //    file:/home/wibble/hmm.ttf
    //
    if(KFI_KIO_FONTS_PROTOCOL==url.protocol())
    {
        KIO::UDSEntry udsEntry;
        QString       name;

        FcInitReinitialize();
        if(KIO::NetAccess::stat(url, udsEntry, NULL))  // Need to stat the url to get its font name...
	    name = udsEntry.stringValue(KIO::UDS_NAME);

        if(!name.isEmpty())
        {
            parseName(name, faceNo, all);
            itsInstalled=true;
        }
        else
            return false;
    }
    else if(url.isLocalFile())
    {
        // Now lets see if its from the thumbnail job! if so, then file will just contain the URL!
        QFile file(url.path());
        bool  isThumbnailUrl=false;

        if(file.size()<2048 && file.open(QIODevice::ReadOnly)) // Urls should be less than 2k, and fonts usually above!
        {
            QString     thumbUrl;
            QTextStream stream(&file);

            thumbUrl=stream.readLine();
            isThumbnailUrl=0==thumbUrl.indexOf(KFI_KIO_FONTS_PROTOCOL) && parseUrl(KUrl(thumbUrl), faceNo, all);
            file.close();
        }

        if(!isThumbnailUrl)  // Its not a thumbnail, so read the real font file...
        {
            itsName=url.path();

            int       count;
            FcPattern *pat=FcFreeTypeQuery((const FcChar8 *)(QFile::encodeName(itsName).data()), 0, NULL, &count);

            itsWeight=FC_WEIGHT_NORMAL;
#ifndef KFI_FC_NO_WIDTHS
            itsWidth=FC_WIDTH_NORMAL;
#endif
            itsSlant=FC_SLANT_ROMAN;
            itsSpacing=FC_PROPORTIONAL;

            if(pat)
            {
                itsDescriptiveName=createName(pat, faceNo);

                if(all)
                {
                    FcPatternGetInteger(pat, FC_WEIGHT, faceNo, &itsWeight);
                    FcPatternGetInteger(pat, FC_SLANT, faceNo, &itsSlant);
#ifndef KFI_FC_NO_WIDTHS
                    FcPatternGetInteger(pat, FC_WIDTH, faceNo, &itsWidth);
#endif
                    FcPatternGetInteger(pat, FC_SPACING, faceNo, &itsSpacing);
                    itsFoundry=getFcString(pat, FC_FOUNDRY, faceNo);
                }

                FcPatternDestroy(pat);
            }
            else
                itsDescriptiveName=QString();

            itsInstalled=false;
            itsIndex=faceNo;
        }
    }
    else
        return false;

    itsLastUrl=url;
    return true;
}

void CFcEngine::parseName(const QString &name, int faceNo, bool all)
{
    int pos;

    itsDescriptiveName=name;
    itsSpacing=FC_PROPORTIONAL;
    if(-1==(pos=name.find(", ")))   // No style information...
    {
        itsWeight=FC_WEIGHT_NORMAL;
#ifndef KFI_FC_NO_WIDTHS
        itsWidth=FC_WIDTH_NORMAL;
#endif
        itsSlant=FC_SLANT_ROMAN;
        itsName=name;
    }
    else
    {
        QString style(name.mid(pos+2));

        itsWeight=strToWeight(style, style);
#ifndef KFI_FC_NO_WIDTHS
        itsWidth=strToWidth(style, style);
#endif
        itsSlant=strToSlant(style);
        itsName=name.left(pos);
    }

    if(all)
    {
        FcObjectSet *os  = FcObjectSetBuild(FC_SPACING, FC_FOUNDRY, (void *)0);
        FcPattern   *pat = FcPatternBuild(NULL,
                                            FC_FAMILY, FcTypeString, (const FcChar8 *)(itsName.toLatin1().data()),
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
    itsLastUrl=KUrl();
}

#ifdef HAVE_XFT
XftFont * CFcEngine::getFont(int size, QPixmap *pix)
{
    if(itsInstalled)
        return XftFontOpen(KFI_DISPLAY(pix), 0,
                           FC_FAMILY, FcTypeString, (const FcChar8 *)(itsName.toLatin1().data()),
                           FC_WEIGHT, FcTypeInteger, itsWeight,
                           FC_SLANT, FcTypeInteger, itsSlant,
#ifndef KFI_FC_NO_WIDTHS
                           FC_WIDTH, FcTypeInteger, itsWidth,
#endif
                           FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                           NULL);
    else
    {
        FcPattern *pattern = FcPatternBuild(NULL,
                                            FC_FILE, FcTypeString, QFile::encodeName(itsName).data(),
                                            FC_INDEX, FcTypeInteger, itsIndex,
                                            FC_PIXEL_SIZE, FcTypeDouble, (double)size,
                                            NULL);
        return XftFontOpenPattern(KFI_DISPLAY(pix), pattern);
    }
}

void CFcEngine::getSizes(QPixmap *pix)
{
    static const int constNumSizes=11;
    static const int constNumSizeRanges=2;
    static const int constSizes[constNumSizeRanges][constNumSizes]= { {8, 10, 12, 14, 16, 18, 24, 36, 48, 72, 96},
                                                                      {7, 9, 11, 13, 15, 17, 23, 35, 47, 71, 95} };
    XftFont *f=getFont(8, pix);

    itsScalable=FcTrue;

    itsSizes.clear();
    itsAlphaSize=0;

    if(f)
    {
        bool gotSizes=false;

        if(itsInstalled)
        {
            if(FcResultMatch!=FcPatternGetBool(f->pattern, FC_SCALABLE, 0, &itsScalable))
                itsScalable=FcFalse;
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

                    for (size=0; size<numSizes; size++)
                    {
                        itsSizes.push_back(face->available_sizes[size].height);
                        if (face->available_sizes[size].height<=constDefaultAlphaSize)
                            itsAlphaSize=face->available_sizes[size].height;
                    }
                }
                XftUnlockFace(f);
            }
        }

        XftFontClose(KFI_DISPLAY(pix), f);

        //
        // Hmm... its not a scalable font, and its installed. So to get list of sizes, iterate through a list of standard
        // sizes, and ask fontconfig for a font of that sizes. Then check the retured size, family, etc is what was asked
        // for!
        if(!itsScalable && !gotSizes)
        {
            itsSizes.reserve(constNumSizes);

            for(int l=0; l<constNumSizeRanges && !gotSizes; ++l)
                for(int i=0; i<constNumSizes; ++i)
                {
                    double  px;
                    int     iv;
                    FcChar8 *str;

                    f=getFont(constSizes[l][i], pix);

                    if(f)
                    {
                        if(FcResultMatch==FcPatternGetDouble(f->pattern, FC_PIXEL_SIZE, 0, &px) && equal(constSizes[l][i], px) &&
                           FcResultMatch==FcPatternGetInteger(f->pattern, FC_WEIGHT, 0, &iv) && equalWeight(iv,itsWeight) &&
                           FcResultMatch==FcPatternGetInteger(f->pattern, FC_SLANT, 0, &iv) && equalSlant(iv, itsSlant) &&
#ifndef KFI_FC_NO_WIDTHS
                           FcResultMatch==FcPatternGetInteger(f->pattern, FC_WIDTH, 0, &iv) && equalWidth(iv, itsWidth) &&
#endif
                           FcResultMatch==FcPatternGetString(f->pattern, FC_FAMILY, 0, &str) && str &&
                           0==strcmp((const char *)str, itsName.toLatin1()))
                        {
                            itsSizes.push_back(constSizes[l][i]);
                            gotSizes=true;
                            if(constSizes[l][i]<=constDefaultAlphaSize)
                                itsAlphaSize=constSizes[l][i];
                        }
                        XftFontClose(KFI_DISPLAY(pix), f);
                    }
                }
        }
    }

    if(itsScalable)
    {
        itsSizes.reserve(constNumSizes);

        for (int i=0; constScalableSizes[i]; ++i)
            itsSizes.push_back(point2Pixel(constScalableSizes[i]));
        itsAlphaSize=constDefaultAlphaSize;
    }
}
#endif

}
