#ifndef __FONT_ENGINE_H__
#define __FONT_ENGINE_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CFontEngine
// Author        : Craig Drummond
// Project       : K Font Installer
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Encodings.h"
#include <kdeversion.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qwindowdefs.h>
#include <qfile.h>

#ifdef HAVE_FT_CACHE
#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include FT_CACHE_H
#include <qptrlist.h>
#include <qpaintdevice.h>
#endif

// Config file...
#define KFI_PREVIEW_GROUP         "Preview Settings"
#define KFI_PREVIEW_STRING_KEY    "String"
#define KFI_PREVIEW_SIZE_KEY      "Size"
#define KFI_PREVIEW_WATERFALL_KEY "Waterfall"

// OK - some macros to make determining the FreeType version easier...
#define KFI_FREETYPE_VERSION      KDE_MAKE_VERSION(FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH)
#define KFI_FT_IS_GE(a,b,c)       (KFI_FREETYPE_VERSION >= KDE_MAKE_VERSION(a,b,c))

class KURL;
class KConfig;

class CFontEngine
{
    public:

    struct TId
    {
        TId(const QString &p, int f=0) : path(p), faceNo(f) {}

        QString path;
        int     faceNo;
    };

    enum EType
    {
        // These have PS Info / support AFM stuff...
        TRUE_TYPE,
        TT_COLLECTION,
        OPEN_TYPE,
        TYPE_1,

        TYPE_1_AFM,  // Not really a font type, but can be returned by getType()

        // These do not...
        SPEEDO,
        BDF,
        PCF,
        SNF,

        ANY,
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
        SPACING_PROPORTIONAL,
        SPACING_CHARCELL
    };

    enum EItalic
    {
        ITALIC_NONE,
        ITALIC_ITALIC,
        ITALIC_OBLIQUE
    };

    enum EOpen
    {
        TEST       = 0x0,            // Justr try to open font - used for checking if font is valid...
        NAME       = 0x1,            // Read name
        PROPERTIES = 0x2|NAME,       // Read weight, familiy, and postscript names
        XLFD       = 0x4|PROPERTIES  // Read extra details only needed for xlfd (i.e. foundry, width, and spacing)
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

        TFtData();
        ~TFtData();

        FT_Library      library;
        FT_Face         face;
        bool            open;

#ifdef HAVE_FT_CACHE
        FTC_Manager     cacheManager;

#if KFI_FT_IS_GE(2, 1, 8)
        FTC_ImageCache  imageCache;
        FTC_SBitCache   sBitCache;
#else
        FTC_Image_Cache imageCache;
        FTC_SBit_Cache  sBitCache;
#endif

        QPtrList<TId>   ids;
        unsigned char   *buffer;
        int             bufferSize;
#endif
    };

    struct Bitmap
    {
        int           width,
                      height,
                      greys,
                      pitch;
        bool          mono;
        unsigned char *buffer;
    };

    public:

    CFontEngine() : itsType(NONE)               { }
    ~CFontEngine()                              { closeFont(); }

    static bool    isA(const char *fname, const char *ext, bool z=false);
    static bool    isATtf(const char *fname)    { return isA(fname, "ttf"); }
    static bool    isATtc(const char *fname)    { return isA(fname, "ttc"); }
    static bool    isAOtf(const char *fname)    { return isA(fname, "otf"); }
    static bool    isAType1(const char *fname)  { return isA(fname, "pfa") || isA(fname, "pfb"); }
    static bool    isASpeedo(const char *fname) { return isA(fname, "spd"); }
    static bool    isABdf(const char *fname)    { return isA(fname, "bdf", true); }
    static bool    isASnf(const char *fname)    { return isA(fname, "snf", true); }
    static bool    isAPcf(const char *fname)    { return isA(fname, "pcf", true); }
    static bool    isABitmap(const char *fname) { return isAPcf(fname) || isABdf(fname) || isASnf(fname); }
    static bool    isABitmap(EType type)        { return PCF==type || BDF==type || SNF==type; }
    static bool    isAFont(const char *fname);
    static bool    isAFont(EType type)          { return type<=SNF && TYPE_1_AFM!=type; }
    static bool    isAAfm(const char *fname)    { return isA(fname, "afm"); }
    static bool    isAFontOrAfm(const char *fn) { return getType(fn)<=SNF; }
    static EType   getType(const char *fname);
    static QString weightStr(EWeight w);
    static QString widthStr(EWidth w);
    static QString italicStr(EItalic i)         { return ITALIC_NONE==i ? "r" : ITALIC_ITALIC==i ? "i" : "o"; }
    static QString spacingStr(ESpacing s);

    //
    // General functions - these should be used instead of specfic ones below...
    //
    bool            openFont(const QString &file, unsigned short mask=NAME, bool force=false, int face=0);
    bool            openFont(const KURL &url, unsigned short mask=NAME, bool force=false, int face=0);
    void            closeFont();

    const QString & getFullName()     { return itsFullName; }
    const QString & getFamilyName()   { return itsFamily; }
    const QString & getPsName()       { return itsPsName; }

    EWeight         getWeight()       { return itsWeight; }
    EWidth          getWidth()        { return itsWidth; }
    EItalic         getItalic()       { return itsItalic; }
    ESpacing        getSpacing()      { return itsSpacing; }

    const QString & getFoundry()      { return itsFoundry; }
    const QString & getAddStyle()     { return itsAddStyle; }

    EType           getType()         { return itsType; }
    int             getNumFaces()     { return itsFt.open ? itsFt.face->num_faces : 1; }

    bool            hasPsInfo()       { return itsType<=TYPE_1_AFM; }
    static bool     hasPsInfo(const char *fname) { return getType(fname)<=TYPE_1_AFM; }

    bool            hasAfmInfo()      { return itsType<TYPE_1_AFM; }
    static bool     hasAfmInfo(const char *fname) { return getType(fname)<TYPE_1_AFM; }

    bool            isScaleable();
    QStringList     getEncodings();

    static EWeight  strToWeight(const char *str);
    static EWidth   strToWidth(const QString &str);

    QString         createName(const QString &file, bool force=false);

#ifdef HAVE_FT_CACHE
    QString         getPreviewString();
    void            setPreviewString(const QString &str);

    void            createPreview(int width, int height, QPixmap &pix, int faceNo=0, int fSize=-1, bool thumb=true,
                                  bool waterfall=false);
    static int      point2Pixel(int point)
    {
        return (point* /*QPaintDevice::x11AppDpiX()*/ 75 +36)/72;
    }
#endif

    QString &       getXlfdBmp()                          { return itsXlfd; }

    private:

    //
    // Type1 functions...
    //
    bool            openFontT1(const QString &file, unsigned short mask=NAME);

    //
    // AFM...
    //

    bool            openFontAfm(const QString &file);

    //
    // TrueType functions...
    //
    bool            openFontTT(const QString &file, unsigned short mask=NAME);
    static EWeight  mapWeightTT(FT_UShort os2Weight);
    static EWidth   mapWidthTT(FT_UShort os2Width);
        //
        //  TrueType & Type1 shared functionality (FreeType2)
        //
    void            closeFaceFt();
    void            setPsNameFt();
    bool            setCharmapFt(FT_CharMap &charMap)    { return FT_Set_Charmap(itsFt.face, charMap) ? false : true; }
    unsigned int    getGlyphIndexFt(unsigned short code) { return FT_Get_Char_Index(itsFt.face, code); }
    bool            setCharmapUnicodeFt()                { return FT_Select_Charmap(itsFt.face, ft_encoding_unicode) ? false : true; }
    bool            setCharmapSymbolFt()                 { return FT_Select_Charmap(itsFt.face, ft_encoding_symbol) ? false : true; } 

#ifdef HAVE_FONT_ENC
    bool            findCharMapFt(int type, int pid, int eid);
    bool            checkEncodingFt(const QString &enc);
    bool            checkExtraEncodingFt(const QString &enc, bool found);
#else
    bool            has16BitEncodingFt(const QString &enc);
    bool            has8BitEncodingFt(CEncodings::T8Bit *data);
    QStringList     get8BitEncodingsFt();
#endif
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
    void            createNameBmp(int pointSize, int res, const QString &enc);
    void            parseXlfdBmp();

    // Bdf...
    bool            openFontBdf(const QString &file);

    // Snf...
    bool            openFontSnf(const QString &file);

    // Pcf...
    bool            openFontPcf(const QString &file);

#ifdef HAVE_FT_CACHE
    FTC_FaceID getId(const QString &f, int faceNo); 

#if KFI_FT_IS_GE(2, 1, 8)
    bool       getGlyphBitmap(FTC_ImageTypeRec &font, FT_ULong index, Bitmap &target, int &left, int &top,
                             int &xAdvance, FT_Pointer *ptr);
#else
    bool       getGlyphBitmap(FTC_Image_Desc &font, FT_ULong index, Bitmap &target, int &left, int &top,
                             int &xAdvance, FT_Pointer *ptr);
#endif

    void       align32(Bitmap &bmp);

#if KFI_FT_IS_GE(2, 1, 8)
    bool       drawGlyph(QPixmap &pix, FTC_ImageTypeRec &font, int glyphNum, FT_F26Dot6 &x, FT_F26Dot6 &y,
                         FT_F26Dot6 width, FT_F26Dot6 height, FT_F26Dot6 startX, FT_F26Dot6 stepY, bool multiLine=true);
#else
    bool       drawGlyph(QPixmap &pix, FTC_Image_Desc &font, int glyphNum, FT_F26Dot6 &x, FT_F26Dot6 &y,
                         FT_F26Dot6 width, FT_F26Dot6 height, FT_F26Dot6 startX, FT_F26Dot6 stepY, bool multiLine=true);
#endif

#endif

    void       createAddStyle();

    private:

    EWeight  itsWeight;
    EWidth   itsWidth;
    EType    itsType;
    EItalic  itsItalic;
    ESpacing itsSpacing;
    QString  itsFullName,
             itsFamily,
             itsPsName,
             itsXlfd,        // Used for Bitmap fonts
             itsFoundry,
             itsAddStyle,
             itsPath;
    int      itsNumFaces,
             itsPixelSize,   // Used for Bitmap fonts
             itsFaceIndex;   // Only for TTC fonts - at the moment...
    TFtData  itsFt;
};

#endif
