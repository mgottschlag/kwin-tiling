#ifndef __FONT_ENGINE_H__
#define __FONT_ENGINE_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontEngine
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
// Creation Date : 29/04/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include "Encodings.h"
#include <freetype/freetype.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qwindowdefs.h>

class CFontEngine
{
    public:

    enum EType
    {
        ANY,
        TRUE_TYPE,
        TYPE_1,
        SPEEDO,
        BITMAP,
        NONE
    };

    enum EWeight
    {
        WEIGHT_UNKNOWN=0,
        WEIGHT_THIN,
        WEIGHT_ULTRA_LIGHT,
        WEIGHT_EXTRA_LIGHT,
        WEIGHT_DEMI,
        WEIGHT_LIGHT,
        WEIGHT_BOOK,
        WEIGHT_MEDIUM,
        WEIGHT_REGULAR,
        WEIGHT_SEMI_BOLD,
        WEIGHT_DEMI_BOLD,
        WEIGHT_BOLD,
        WEIGHT_EXTRA_BOLD,
        WEIGHT_ULTRA_BOLD,
        WEIGHT_HEAVY,
        WEIGHT_BLACK
    };

    enum EWidth
    {
        WIDTH_UNKNOWN=0,
        WIDTH_ULTRA_CONDENSED,
        WIDTH_EXTRA_CONDENSED,
        WIDTH_CONDENSED,
        WIDTH_SEMI_CONDENSED,
        WIDTH_NORMAL,
        WIDTH_SEMI_EXPANDED,
        WIDTH_EXPANDED,
        WIDTH_EXTRA_EXPANDED,
        WIDTH_ULTRA_EXPANDED
    };

    enum ESpacing
    {
        SPACING_MONOSPACED,
        SPACING_PROPORTIONAL
    };

    enum EItalic
    {
        ITALIC_NONE,
        ITALIC_ITALIC,
        ITALIC_OBLIQUE
    };

    enum EOpen
    {
        TEST       = 0x0,    // Justr try to open font - used for checking if font is valid...
        NAME       = 0x1,    // Read name
        PROPERTIES = 0x2,    // Read weight, familiy, and postscript names
        XLFD       = 0x4,    // Read extra details only needed for xlfd (i.e. foundry, width, and spacing)
        PREVIEW    = 0x8,    // Only really affects fonts opened with FreeType
        AFM        = NAME|PROPERTIES|XLFD|PREVIEW   // For AFM, just read all properties...
    };

    struct TGlyphInfo
    {
        enum EConstants
        {
            MAX_NAME_LEN = 255
        };

        char name[MAX_NAME_LEN+1];
        int  scaledWidth,
             xMin,
             xMax,
             yMin,
             yMax;
    };

    private:

    struct TFtData
    {
        struct TBitmap
        {
            TBitmap() : w(0), h(0), data(NULL) {}
            ~TBitmap() { if(data) delete data; }

            int           w,
                          h;
            unsigned char *data;
        };

        TFtData() : open(false) {}

        FT_Library library;
        FT_Face    face;
        bool       open;
        TBitmap    bmp;
    };

    public:

    CFontEngine();
    ~CFontEngine();

    static bool    isA(const char *fname, const char *ext, bool z=false);
    static bool    isATtf(const char *fname)    { return isA(fname, "ttf"); }
    static bool    isAType1(const char *fname)  { return isA(fname, "pfa") || isA(fname, "pfb"); }
    static bool    isASpeedo(const char *fname) { return isA(fname, "spd"); }
    static bool    isABdf(const char *fname)    { return isA(fname, "bdf", true); }
    static bool    isASnf(const char *fname)    { return isA(fname, "snf", true); }
    static bool    isAPcf(const char *fname)    { return isA(fname, "pcf", true); }
    static bool    isABitmap(const char *fname) { return isAPcf(fname) || isABdf(fname) || isASnf(fname); }
    static bool    isAFont(const char *fname)   { return isATtf(fname) || isAType1(fname) || isASpeedo(fname) || isABitmap(fname); }
    static bool    correctType(const char *fname, CFontEngine::EType type);
    static EType   getType(const char *fname);
    static QString weightStr(EWeight w);
    static QString widthStr(EWidth w);
    static QString italicStr(EItalic i)         { return ITALIC_NONE==i ? "r" : ITALIC_ITALIC==i ? "i" : "o"; }
    static QString spacingStr(ESpacing s);

    //
    // General functions - these should be used instead of specfic ones below...
    //
    bool            openFont(const QString &file, unsigned short mask=NAME);
    void            closeFont();

    QPixmap         createPixmap(const QString &str, int width, int height, int pointSize, int resolution,  QRgb backgroundColour);
    const QString & getFullName()     { return itsFullName; }
    const QString & getFamilyName()   { return itsFamily; }
    const QString & getPsName()       { return itsPsName; }

    EWeight         getWeight()       { return itsWeight; }
    EWidth          getWidth()        { return itsWidth; }
    EItalic         getItalic()       { return itsItalic; }
    ESpacing        getSpacing()      { return itsSpacing; }

    const char *    getFoundry()      { return itsFoundry; }

    EType           getType()         { return itsType; }

    QStringList     getEncodings();
    QStringList     get8BitEncodings();

    EWeight         strToWeight(const char *str);
    EWidth          strToWidth(const QString &str);

    //
    // Metric accsessing functions...  (only work for TrueType & Type1)
    //
    int                scaleMetric(int metric);
    float              getItalicAngle();
    int                getAscender();
    int                getDescender();
    int                getUnderlineThickness();
    int                getUnderlinePosition(); 
    int                getBBoxXMin();
    int                getBBoxXMax();
    int                getBBoxYMin();
    int                getBBoxYMax();
    const TGlyphInfo * getGlyphInfo(unsigned long glyph);

    //
    // Type1 functions...
    //
    const char *    getTokenT1(const char *str, const char *key);
    const char *    getReadOnlyTokenT1(const char *str, const char *key);
    bool            openFontT1(const QString &file, unsigned short mask=NAME);
    QStringList     getEncodingsT1();
    QStringList     get8BitEncodingsT1();
    bool            getIsArrayEncodingT1();

    //
    // TrueType functions...
    //
    bool            openFontTT(const QString &file, unsigned short mask=NAME);
    void            closeFontTT();
    EWeight         mapWeightTT(FT_UShort os2Weight);
    EWidth          mapWidthTT(FT_UShort os2Width);
    QCString        lookupNameTT(int index);
        //
        //  TrueType & Type1 shared functionality (FreeType2)
        //
    bool            setCharmapFt(FT_CharMap &charMap)    { return FT_Set_Charmap(itsFt.face, charMap) ? false : true; }
    unsigned int    getGlyphIndexFt(unsigned short code) { return FT_Get_Char_Index(itsFt.face, code); }
    bool            setCharmapUnicodeFt()                { return FT_Select_Charmap(itsFt.face, ft_encoding_unicode) ? false : true; }
    bool            setCharmapSymbolFt()                 { return FT_Select_Charmap(itsFt.face, ft_encoding_symbol) ? false : true; } 
    QPixmap         createPixmapFt(const QString &str, int width, int height, int pointSize, int resolution, QRgb backgroundColour);
    bool            hasEncodingFt(FT_UShort enc);
    bool            has16BitEncodingFt(const QString &enc);
    bool            has8BitEncodingFt(CEncodings::T8Bit *data);
    QStringList     get8BitEncodingsFt();
    QStringList     getEncodingsFt();
    int             getNumGlyphsFt()                     { return itsFt.open ? itsFt.face->num_glyphs : 0; }

    //
    // Speedo functions...
    //
    bool            openFontSpd(const QString &file, unsigned short mask=NAME);
    QStringList     getEncodingsSpd();

    //
    // Bitmap functions...
    //
    bool            openFontBmp(const QString &file, unsigned short mask=NAME);
    void            createNameBmp(int pointSize, int res, const QString &enc);
    void            createNameFromXlfdBmp();
    bool            getFileEncodingBmp(const char *str);
    QString &       getXlfdBmp()                          { return itsXlfd; }

    // Bdf...
    bool            openFontBdf(const QString &file, unsigned short mask=NAME);

    // Snf...
    bool            openFontSnf(const QString &file, unsigned short mask=NAME);

    // Pcf...
    bool            openFontPcf(const QString &file, unsigned short mask=NAME);

    private:

    EWeight        itsWeight;
    EWidth         itsWidth;
    EType          itsType;
    EItalic        itsItalic;
    ESpacing       itsSpacing;
    QString        itsFullName,
                   itsFamily,
                   itsPsName,
                   itsEncoding, // Used for Bitmap & Type1 fonts
                   itsXlfd;     // Used for Bitmap fonts
    float          itsItalicAngle;
    TFtData        itsFt;
    const char     *itsFoundry;
};

#endif
