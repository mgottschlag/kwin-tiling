///////////////////////////////////////////////////////////////////////////////
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FontEngine.h"
#include "KfiGlobal.h"
#include "Config.h"
#include "Misc.h"
#include "CompressedFile.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fstream.h>
#include <iostream.h>
#include <stdio.h>
#include <zlib.h>
#include <netinet/in.h>
#include <qimage.h>
#include <qregexp.h>

//
// NOTE: The FreeType headers are in quotes and not brackets, as the path is supplied in the compile line
//       and some systems have both FreeType1 and FreeType2 installed - if we used <freetype/freetype.h> then it
//       is likely that the FreeType1 header would be included!
//
#ifdef HAVE_FT2_FTSNAMES
#include "freetype/ftsnames.h"
#else
#include "freetype/ftnames.h"
#endif
#include "freetype/ttnameid.h"
#include "freetype/tttables.h"
#include "freetype/ftimage.h"

static const char *constNoPsName="NO_PS_NAME";
static const char *constBmpRoman="";
static const char *constBmpItalic=" Italic";
static const char *constBmpOblique=" Oblique";

static const char *constDefaultFoundry="misc";

struct TTtfFoundryMap
{
    const char *vendorId,
               *foundry;
};

static const TTtfFoundryMap constTtfFoundries[]=    // These are taken from ttmkfdir
{                                                   // Removed any commas - StarOffice doesn't like these...
    { "2REB", "2Rebels"},                           // Shortened quite a few entires to help with StarOffice
    { "3IP" , "3ip"},
    { "3IP ", "3ip"},
    { "ABC" , "AltekInst"},
    { "ABC ", "AltekInst"},
    { "ACUT", "AcuteType"},
    { "ADBE", "adobe"},
    { "AGFA", "AgfaTypoSys"},
    { "ALTS", "altsys"},
    { "AOP" , "AnArtofP"},
    { "AOP ", "AnArtofP"},
    { "APPL", "Apple"},
    { "ARPH", "ArphicTech"},
    { "ATEC", "alltype"},
    { "AZLS", "AzaleaInc"},
    { "B&H",  "B&H"},
    { "B&H ", "B&H"},
    { "BARS", "CIA UK"},
    { "BERT", "Berthold"},
    { "BITM", "Bitmap Soft"},
    { "BITS", "Bitstream"},
    { "BLAH", "MisterBlas"},
    { "BOYB", "I.Frances"},
    { "BRTC", "BearRockTech"},
    { "BWFW", "B/WFontworks"},
    { "C&C",  "Carter&Cone"},
    { "C&C ", "Carter&Cone"},
    { "CAK" , "pluginfonts"},
    { "CAK ", "pluginfonts"},
    { "CANO", "cannon"},
    { "CASL", "H.W.Caslon"},
    { "COOL", "CoolFonts"},
    { "CTDL", "ChinaTyDesLtd"},
    { "DAMA", "D.M.Ltd"},
    { "DS"  , "DSMCoInc"},
    { "DS  ", "DScCoInc"},
    { "DSCI", "DesScInc"},
    { "DTC",  "DTCorp"},
    { "DTC ", "DTC"},
    { "DTPS", "DTPS"},
    { "DUXB", "DSysInc"},
    { "DYNA", "DynaLab"},
    { "ECF" , "e.c.f."},
    { "ECF ", "e.c.f."},
    { "EDGE", "R.E.Corp."},
    { "EFF" , "EFF"},
    { "EFF ", "EFF"},
    { "EFNT", "EFLLC"},
    { "ELSE", "elseware"},
    { "EPSN", "epson"},
    { "ERAM", "eraman"},
    { "ESIG", "E-Signature"},
    { "FBI",  "TFBI"},
    { "FBI ", "TFBI"},
    { "FCAB", "TFC"},
    { "FJ",   "Fujitsu"},
    { "FJ  ", "Fujitsu"},
    { "FONT", "FontSource"},
    { "FS"  , "FormulaSls"},
    { "FS  ", "FormulaSls"},
    { "FSE" , "FontSrc"},
    { "FSE ", "FontSrc"},
    { "FSI" , "FSIGmbH"},
    { "FSI ", "FSIGmbH"},
    { "FTFT", "FontFont"},
    { "FWRE", "FontwareLtd"},
    { "GALA", "Galapagos"},
    { "GD"  , "GDFonts"},
    { "GD  ", "GDFonts"},
    { "GLYF", "GlyphSys"},
    { "GPI",  "GammaProd"},
    { "GPI ", "GammaProd"},
    { "HILL", "Hill Sys"},
    { "HOUS", "HouseInd"},
    { "HP",   "hp"},
    { "HP  ", "hp"},
    { "HY",   "HanYangSys"},
    { "HY  ", "HanYangSys"},
    { "IBM",  "IBM"},
    { "IBM ", "IBM"},
    { "IDEE", "IDEE"},
    { "IDF",  "IntDigital"},
    { "IDF ", "IntDigital"},
    { "ILP",  "IndLang"},
    { "ILP ", "IndLang"},
    { "ITC",  "itc"},
    { "ITC ", "itc"},
    { "IMPR", "impress"},
    { "ITF" , "IntTypeInc"},
    { "ITF ", "IntTypeInc"},
    { "KATF", "Kingsley"},
    { "LANS", "Lanston"},
    { "LARA", "Larabie"},
    { "LEAF", "Interleaf"},
    { "LETR", "letraset"},
    { "LGX" , "LogixRII"},
    { "LGX ", "LogixRII"},
    { "LING", "Linguists"},
    { "LINO", "linotype"},
    { "LP",   "LetterPer"},
    { "LP  ", "LetterPer"},
    { "LTRX", "lighttracks"},
    { "MACR", "macromedia"},
    { "MC"  , "Cerajewski"},
    { "MC  ", "Cerajewski"},
    { "MILL", "Millan"},
    { "MJ"  , "MajusCorp"},
    { "MJ  ", "MajusCorp"},
    { "MLGC", "Micrologic"},
    { "MONO", "monotype"},
    { "MS",   "microsoft"},
    { "MS  ", "microsoft"},
    { "MSCR", "MajusCorp"},
    { "MT",   "monotype"},
    { "MT  ", "monotype"},
    { "MTY" , "MotoyaCoLTD"},
    { "MTY ", "MotoyaCoLTD"},
    { "MUTF", "CACHE"},
    { "NB"  , "NoBodoniTyp"},
    { "NB  ", "NoBodoniTyp"},
    { "NDTC", "NeufvilleDig"},
    { "NEC",  "nec"},
    { "NEC ", "nec"},
    { "NIS" , "NIS Corp"},
    { "NIS ", "NIS Corp"},
    { "ORBI", "OrbitEntInc"},
    { "P22" , "P22 Inc"},
    { "P22 ", "P22 Inc"},
    { "PARA", "ParaType"},
    { "PDWX", "ParsonsDes"},
    { "PF"  , "PhilsFonts"},
    { "PF  ", "PhilsFonts"},
    { "PRFS", "Production"},
    { "QMSI", "QMS/Imagen"},
    { "RICO", "Ricoh"},
    { "RKFN", "R K Fonts"},
    { "robo", "Buro Petr"},
    { "RUDY", "RudynFluffy"},
    { "SAX" , "s.a.x.gmbh"},
    { "SAX ", "s.a.x.gmbh"},
    { "Sean", "The FontSite"},
    { "SFI" , "SoftwareF"},
    { "SFI ", "SoftwareF"},
    { "SFUN", "Soft Union"},
    { "SG"  , "ScooterGfx"},
    { "SG  ", "ScooterGfx"},
    { "SIG" , "Signature"},
    { "SIG ", "Signature"},
    { "SKZ" , "CelticLadys"},
    { "SKZ ", "CelticLadys"},
    { "SN"  , "SourceNet"},
    { "SN  ", "SourceNet"},
    { "SOHO", "SoftHorizons"},
    { "SOS" , "StandingOvs"},
    { "SOS ", "StandingOvs"},
    { "STF" , "BrianSooy"},
    { "STF ", "BrianSooy"},
    { "STON", "ZHUHAI"},
    { "SUNW", "sunwalk"},
    { "SWFT", "SwfteInt"},
    { "SYN" , "SynFonts"},
    { "SYN ", "SynFonts"},
    { "TDR" , "TansinADarcs"},
    { "TDR ", "TansinADarcs."},
    { "TF"  , "Treacyfaces"},
    { "TF  ", "Treacyfaces"},
    { "TILD", "SIATilde"},
    { "TPTC", "TESTPILOT"},
    { "TR"  , "TypeRevivals"},
    { "TR  ", "TypeRevivals"},
    { "TS"  , "TamilSoft"},
    { "TS  ", "TamilSoft"},
    { "UA"  , "UnAuthorized"},
    { "UA  ", "UnAuthorized"},
    { "UNKN", constDefaultFoundry},
    { "URW",  "urw"},
    { "URW ", "urw"},
    { "VKP" , "VijayKPatel"},
    { "VKP ", "VijayKPatel"},
    { "VLKF", "Visualogik"},
    { "VOG" , "MartinVogel"},
    { "VOG ", "MartinVogel"},
    { "Y&Y" , "y&y"},
    { "Y&Y ", "y&y"},
    { "ZEGR", "ZebraFontFacit"},
    { "ZETA", "TangramStudio"},
    { "ZSFT", "ZSoft"},
    { NULL  ,  NULL}
};

struct TT1AndSpdFoundryMap
{
    const char *noticeStr,
               *foundry;
};

static const TT1AndSpdFoundryMap constT1AndSpdFoundries[]=    // These are taken from type1inst ( (C) 1996-1998 James Macnicol )
{
    { "Richard Mitchell",                   "mitchell" },
    { "Doug Miles",                         "miles" },
    { "Hank Gillette",                      "gillette" },
    { "Three Islands Press",                "3ip" },
    { "MacroMind",                          "macromind" },
    { "MWSoft",                             "mwsoft" },
    { "Digiteyes Multimedia",               "digiteyes" },
    { "ZSoft",                              "zsoft" },
    { "Title Wave",                         "titlewave" },
    { "Southern Software",                  "southern" },
    { "Reasonable Solutions",               "reasonable" },
    { "David Rakowski",                     "rakowski" },
    { "D. Rakowski",                        "rakowski" },
    { "S. G. Moye",                         "moye" },
    { "S.G. Moye",                          "moye" },
    { "Andrew s. Meit",                     "meit" },
    { "A.S.Meit",                           "meit" },
    { "Hershey",                            "hershey" },
    { "FontBank",                           "fontbank" },
    { "A. Carr",                            "carr" },
    { "Brendel Informatik",                 "brendel" },
    { "Jonathan Brecher",                   "brecher" },
    { "URW",                                "urw" },
    { "SoftMaker",                          "softmaker" },
    { "Monotype Corporation",               "monotype" },
    { "LETRASET",                           "letraset" },
    { "IBM",                                "ibm" },
    { "International Typeface Corporation", "itc" },
    { "Corel Corporation",                  "corel"},
    { "Bitstream",                          "bitstream" },
    { "Bigelow & Holmes",                   "b&h" },
    { "PUBLISHERS PARADISE",                "paradise" },
    { "Publishers Paradise",                "paradise" },
    { "Allied Corporation",                 "allied" },
    { "Adobe",                              "adobe" },
    { "XFree86",                            "xfree86"},
    { NULL,                                 NULL }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    GENERIC
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFontEngine::CFontEngine()
{
    itsType=NONE;
    itsFoundry=NULL;
    if(FT_Init_FreeType(&itsFt.library))
    {
        cerr << "ERROR: FreeType2 failed to initialise\n";
        exit(0);
    }
}

CFontEngine::~CFontEngine()
{
    closeFont();
    FT_Done_FreeType(itsFt.library);
}

bool CFontEngine::openFont(const QString &file, unsigned short mask)
{
    closeFont();

    itsType=getType(file.local8Bit());
    itsWeight=WEIGHT_MEDIUM;
    itsWidth=WIDTH_NORMAL;
    itsSpacing=SPACING_PROPORTIONAL;
    itsItalicAngle=0;
    itsItalic=ITALIC_NONE;
    itsEncoding=QString::null;
    itsFt.open=false;

    switch(itsType)
    {
        case TRUE_TYPE:
            return openFontTT(file, mask);
        case TYPE_1:
            return openFontT1(file, mask);
        case SPEEDO:
            return openFontSpd(file, mask);
        case BITMAP:
            return openFontBmp(file, mask);
        default:
            return false;
    }
}
        
void CFontEngine::closeFont()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
    {
        FT_Done_Face(itsFt.face);
        itsFt.open=false;
    }

    itsType=NONE;
}

QPixmap CFontEngine::createPixmap(const QString &str, int width, int height, int pointSize, int resolution,
                                  QRgb backgroundColour)
{
    QPixmap nullPix;

    if(TRUE_TYPE==itsType || TYPE_1==itsType)
        return createPixmapFt(str, width, height, pointSize, resolution, backgroundColour);
    else
        return nullPix;
}

bool CFontEngine::isA(const char *fname, const char *ext, bool z)
{
    int  len=strlen(fname);
    bool fnt=false;
 
    if(z)
    {
        if(len>7)                 // Check for .ext.gz
            fnt=(fname[len-7]=='.' && tolower(fname[len-6])==ext[0] && tolower(fname[len-5])==ext[1] && tolower(fname[len-4])==ext[2] &&
                 fname[len-3]=='.' && tolower(fname[len-2])=='g' && tolower(fname[len-1])=='z');

        if(!fnt && len>6)         // Check for .ext.Z
            fnt=(fname[len-6]=='.' && tolower(fname[len-5])==ext[0] && tolower(fname[len-4])==ext[1] && tolower(fname[len-3])==ext[2] &&
                 fname[len-2]=='.' && toupper(fname[len-1])=='Z');
    }
 
    if(!fnt && len>4)  // Check for .ext
        fnt=(fname[len-4]=='.' && tolower(fname[len-3])==ext[0] && tolower(fname[len-2])==ext[1] && tolower(fname[len-1])==ext[2]);
 
    return fnt;
}
 
bool CFontEngine::correctType(const char *fname, CFontEngine::EType type)
{
    return ((type==TRUE_TYPE && isATtf(fname)) || (type==TYPE_1 && isAType1(fname)) ||
            (type==SPEEDO && isASpeedo(fname)) || (type==BITMAP && isABitmap(fname)) || (type==ANY && isAFont(fname)))
           ? true
           : false;
}
 
CFontEngine::EType CFontEngine::getType(const char *fname)
{
    if(isATtf(fname))
        return TRUE_TYPE;
    if(isAType1(fname))
        return TYPE_1;
    if(isASpeedo(fname))
        return SPEEDO;
    if(isABitmap(fname))
        return BITMAP;
    else
        return NONE;
}

QString CFontEngine::weightStr(enum EWeight w)
{
    switch(w)
    {
        case WEIGHT_THIN:
            return "Thin";
        case WEIGHT_ULTRA_LIGHT:
            return "UltraLight";
        case WEIGHT_EXTRA_LIGHT:
            return "ExtraLight";
        case WEIGHT_DEMI:
            return "Demi";
        case WEIGHT_LIGHT:
            return "Light";
        case WEIGHT_BOOK:
            return "Book";
        case WEIGHT_MEDIUM:
            return "Medium";
        case WEIGHT_REGULAR:
            return "Regular";
        case WEIGHT_SEMI_BOLD:
            return "SemiBold";
        case WEIGHT_DEMI_BOLD:
            return "DemiBold";
        case WEIGHT_BOLD:
            return "Bold";
        case WEIGHT_EXTRA_BOLD:
            return "ExtraBold";
        case WEIGHT_ULTRA_BOLD:
            return "UltraBold";
        case WEIGHT_HEAVY:
            return "Heavy";
        case WEIGHT_BLACK:
            return "Black";
        case WEIGHT_UNKNOWN:
        default:
            return "Medium";
    }
}

QString CFontEngine::widthStr(enum EWidth w)
{
    switch(w)
    {
        case WIDTH_ULTRA_CONDENSED:
            return "UltraCondensed";
        case WIDTH_EXTRA_CONDENSED:
            return "ExtraCondensed";
        case WIDTH_CONDENSED:
            return "Condensed";
        case WIDTH_SEMI_CONDENSED:
            return "SemiCondensed";
        case WIDTH_SEMI_EXPANDED:
            return "SemiExpanded";
        case WIDTH_EXPANDED:
            return "Expanded";
        case WIDTH_EXTRA_EXPANDED:
            return "ExtraExpanded";
        case WIDTH_ULTRA_EXPANDED:
            return "UltraExpanded";
        case WIDTH_NORMAL:
        case WIDTH_UNKNOWN:
        default:
            return "Normal";
    }
}
 
QString CFontEngine::spacingStr(enum ESpacing s)
{
    switch(s)
    {
        case SPACING_MONOSPACED:
            return "m";
        default:
        case SPACING_PROPORTIONAL:
            return "p";
    }
}

QStringList CFontEngine::getEncodings()
{
    switch(itsType)
    {
        case TRUE_TYPE:
            return getEncodingsFt();
        case TYPE_1:
            return getEncodingsT1();
        case SPEEDO:
            return getEncodingsSpd();
        case BITMAP:
        default:
        {
            QStringList empty;

            return empty;
        }
    }
}

QStringList CFontEngine::get8BitEncodings()
{
    switch(itsType)
    {
        case TRUE_TYPE:
            return get8BitEncodingsFt();
        case TYPE_1:
            return get8BitEncodingsT1();
        case SPEEDO:
        case BITMAP:
        default:
        {
            QStringList empty;
 
            return empty;
        }
    }
}

CFontEngine::EWeight CFontEngine::strToWeight(const char *str)
{
    if(NULL==str)
        return WEIGHT_UNKNOWN;
    else if(CMisc::stricmp(str, "Bold")==0)
        return WEIGHT_BOLD;
    else if(CMisc::stricmp(str, "Black")==0)
        return WEIGHT_BLACK;
    else if(CMisc::stricmp(str, "ExtraBold")==0)
        return WEIGHT_EXTRA_BOLD;
    else if(CMisc::stricmp(str, "UltraBold")==0)
        return WEIGHT_ULTRA_BOLD;
    else if(CMisc::stricmp(str, "ExtraLight")==0)
        return WEIGHT_EXTRA_LIGHT;
    else if(CMisc::stricmp(str, "UltraLight")==0)
        return WEIGHT_ULTRA_LIGHT;
    else if(CMisc::stricmp(str, "Light")==0)
        return WEIGHT_LIGHT;
    else if(CMisc::stricmp(str, "Medium")==0 || CMisc::stricmp(str, "Normal")==0 || CMisc::stricmp(str, "Roman")==0)
        return WEIGHT_MEDIUM; 
    else if(CMisc::stricmp(str, "Regular")==0)
        return WEIGHT_REGULAR;
    else if(CMisc::stricmp(str, "Demi")==0)
        return WEIGHT_DEMI;
    else if(CMisc::stricmp(str, "SemiBold")==0)
        return WEIGHT_SEMI_BOLD;
    else if(CMisc::stricmp(str, "DemiBold")==0)
        return WEIGHT_DEMI_BOLD;
    else if(CMisc::stricmp(str, "Thin")==0)
        return WEIGHT_THIN;
    else if(CMisc::stricmp(str, "Book")==0)
        return WEIGHT_BOOK;
    else
        return WEIGHT_UNKNOWN;
}

CFontEngine::EWidth CFontEngine::strToWidth(const QString &str)
{
    if(QString::null==str)
        return WIDTH_UNKNOWN;
    else if(str.contains("UltraCondensed", false))
        return WIDTH_ULTRA_CONDENSED;
    else if(str.contains("ExtraCondensed", false))
        return WIDTH_EXTRA_CONDENSED;
    else if(str.contains("SemiCondensed", false))
        return WIDTH_SEMI_CONDENSED;
    else if(str.contains("Condensed", false))
        return WIDTH_CONDENSED;
    else if(str.contains("SemiExpanded", false))
        return WIDTH_SEMI_EXPANDED;
    else if(str.contains("UltraExpanded", false))
        return WIDTH_ULTRA_EXPANDED;
    else if(str.contains("ExtraExpanded", false))
        return WIDTH_EXTRA_EXPANDED;
    else if(str.contains("Expanded", false))
        return WIDTH_EXPANDED;
    else
        return WIDTH_NORMAL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Metric accsessing functions...  (only work for TrueType & Type1)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CFontEngine::scaleMetric(int metric)
{
    switch(itsType)
    {
        case TRUE_TYPE:
            return (int)(((metric*1000.0)/((double)itsFt.face->units_per_EM))+0.5);
        case TYPE_1:
            return metric;
        default:
            return 0;
    }
}

float CFontEngine::getItalicAngle()
{
    if(itsFt.open)
        switch(itsType)
        {
            case TRUE_TYPE:
            case TYPE_1:
                return itsItalicAngle;
            default:
                return 0;
        }
    else
        return 0;
}

int CFontEngine::getAscender()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->ascender);
    else
        return 0;
}

int CFontEngine::getDescender()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->descender);
    else
        return 0;
}

int CFontEngine::getUnderlineThickness()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->underline_thickness);
    else
        return 0;
}

int CFontEngine::getUnderlinePosition()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->underline_position);
    else
        return 0;
}

int CFontEngine::getBBoxXMin()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->bbox.xMin);
    else
        return 0;
}

int CFontEngine::getBBoxXMax()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->bbox.xMax);
    else
        return 0;
}

int CFontEngine::getBBoxYMin()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->bbox.yMin);
    else
        return 0;
}

int CFontEngine::getBBoxYMax()
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open)
        return scaleMetric(itsFt.face->bbox.yMax);
    else
        return 0;
}

const CFontEngine::TGlyphInfo * CFontEngine::getGlyphInfo(unsigned long glyph)
{
    if((TRUE_TYPE==itsType || TYPE_1==itsType)&&itsFt.open && !FT_Load_Glyph(itsFt.face, glyph, FT_LOAD_NO_SCALE))
    {
        static TGlyphInfo info;

        FT_Get_Glyph_Name(itsFt.face, glyph, info.name, TGlyphInfo::MAX_NAME_LEN+1);
        info.scaledWidth=scaleMetric(itsFt.face->glyph->metrics.horiAdvance);
        info.xMin=scaleMetric(itsFt.face->glyph->metrics.horiBearingX);
        info.xMax=scaleMetric(itsFt.face->glyph->metrics.width+itsFt.face->glyph->metrics.horiBearingX);
        info.yMin=scaleMetric(itsFt.face->glyph->metrics.horiBearingY-itsFt.face->glyph->metrics.height);
        info.yMax=scaleMetric(itsFt.face->glyph->metrics.horiBearingY);

        return &info;
    }
    else
        return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TrueType, Type1, and Speedo
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void removeString(QString &str, const QString &remove, QCString &removed, bool store=true)
{
    static const QChar space(' ');
    int                pos;
    unsigned int       slen=remove.length();

    if(0<(pos=str.find(remove, 0, false)) && space==str[pos-1] && (str.length()<=pos+slen || space==str[pos+slen]))
    {
        str.remove(pos-1, slen+1);

        if(store)
        {
            removed+=remove.latin1();
            removed+=" ";
        }
    }
}

static QString createNames(const QString &familyName, QString &fullName)
{
    //
    // Some fonts have a FamilyName like "Wibble Wobble"
    // and a FullName like               "Wibble Wobble Bold Italic Oldstyle"
    // ...Therefore can't just use the family name read in from the font - as this would ignore the "Oldstyle" part.
    // So, use the "FullName", and remove the any style information (weight, width, italic/roman/oblique) from this...
    //
    // NOTE: Can't simply use FullName and remove style info - as this would convert "Times New Roman" to "Times New"!!
    //
    QString  family(fullName);
    QCString removed;
    bool     removedFamily=true;

    //
    // Remove family name...
    if(QString::null!=familyName)
        if(0==family.find(familyName))    // This removes "Times New Roman" from "Times New Roman Bold" -- this is the gneral case...
            family.remove(0, familyName.length());
        else
        {
            //
            // Some fonts have the family name listed with no spaces, but have spaces in the full name, e.g.
            //
            //     Family   : LuciduxMono
            //     FullName : Lucidux Mono Italic Oldstyle
            //
            // ...therefore, need to remove each string from FullName that occurs in FamilyName
            QString full(fullName),
                    fam(familyName);
 
            full.replace(QRegExp(" "), "");   // Remove whitespace - so we would now have "LuciduxMonoItalicOldstyle"
            fam.replace(QRegExp(" "), "");
 
            if(0==full.find(fam))  // Found "LuciduxMono" in "LuciduxMonoItalic" - so set family to "ItalicOldstyle"
            {
                //
                // Now we need to extract the family name, and the rest...
                // i.e. Family: "LuciduxMono"
                //      rest  : "Italic Oldstyle" -- this is what get's assigned to the 'family' varaible...
 
                if(full.length()==fam.length())  // No style information...
                    family="";
                else  // Need to remove style info...
                {
                    unsigned int i;
 
                    //
                    // Remove familyName from family
                    for(i=0; i<familyName.length() && family.length(); ++i)
                    {
                        if(family[0]==QChar(' '))
                            family.remove(0, 1);
                        if(family.length())
                            family.remove(0, 1);
                    }
                }
            }
            else
            {
                //
                // FamilyName and family name within FullName are different...
                removedFamily=false;
            }
        }
 
    //
    // Remove widthm, weight, and italic stuff from fullName...
    int prop;
 
    for(prop=CFontEngine::WEIGHT_THIN; prop<=CFontEngine::WEIGHT_BLACK; prop++)
        removeString(family, CFontEngine::weightStr((CFontEngine::EWeight)prop), removed);

    removeString(family, "Italic", removed);
    removeString(family, "Oblique", removed);

    //
    // Most fonts don't list the roman part, if some do then we don't really
    // want to show this - to make everything as similar as possible...
    removeString(family, "Roman", removed, false);
 
    for(prop=CFontEngine::WIDTH_ULTRA_CONDENSED; prop<=CFontEngine::WIDTH_ULTRA_EXPANDED; prop++)
        removeString(family, CFontEngine::widthStr((CFontEngine::EWidth)prop), removed);
 
    removeString(family, "Cond", removed);  // Some fonts just have Cond and not Condensed!
 
    //
    // Add the family name back on...
    if(removedFamily && QString::null!=familyName)
        family=familyName+family;
 
    //
    // Replace any non-alphanumeric or space characters...
    family.replace(QRegExp("&"), "And");
    family=CMisc::removeSymbols(family);
 
    family.simplifyWhiteSpace();

    if(removed.length())
    {
        QCString fn(removedFamily ? family.latin1() : familyName.latin1());

        fn+=" ";
        fn+=removed;
        fullName=fn;
    }
    else
        fullName=removedFamily ? family : familyName;

    return removedFamily ? family : familyName;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TYPE 1
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char * CFontEngine::getReadOnlyTokenT1(const char *str, const char *key)
{
    //
    // Read only tokens have the format:
    //
    //   <key> ( <token> ) readonly def
    // e.g. /FullName (Utopia Bold Italic) readonly def
    //
    static const int constMaxTokenLen=1024;

    static char token[constMaxTokenLen];

    char *start,
         *end;

    token[0]='\0';
    if(NULL!=(start=strstr(str, key)) && NULL!=(start=strchr(start, '(')) && NULL!=(end=strstr(start, "readonly")))
    {
        start+=1;  // Move past open bracket
        // go backwards from 'readonly', until we get to ')'

        for(; end>start; end--)
            if(*end==')')  // Then we've found it...
            {
                unsigned int numChars=end-start;

                if(numChars>constMaxTokenLen-1)
                    numChars=constMaxTokenLen-1;
                strncpy(token, start, numChars);
                token[numChars]='\0';
                break;
            }
    }

    return token[0]=='\0' ? NULL : token;
}

const char * CFontEngine::getTokenT1(const char *str, const char *key)
{
    //
    // Tokens have the format:
    //
    //   <key> <token> def
    // e.g. /isFixedPitch false def
    //
    static const int constMaxTokenLen=1024;
 
    static char token[constMaxTokenLen];
 
    char *start,
         *end;
 
    token[0]='\0';
    if(NULL!=(start=strstr(str, key)) && NULL!=(end=strstr(start, "def")) && end>start)
    {
        for(start+=strlen(key); *start==' ' || *start=='\t'; ++start)  // Skip past any spaces...
            ;

        for(end--; *end==' ' || *end=='\t'; --end)   //  Ditto
            ;
        unsigned int numChars=(end-start)+1;
 
        if(numChars>constMaxTokenLen-1)
            numChars=constMaxTokenLen-1;
        strncpy(token, start, numChars);
        token[numChars]='\0';
    }

    return token[0]=='\0' ? NULL : token;
}

bool CFontEngine::openFontT1(const QString &file, unsigned short mask)
{
    const int constHeaderMaxLen=4096;  // Read in the first X bytes, this should be enough for just the header data.
 
    bool status=false;

    if(TEST==mask || mask&PREVIEW || mask&XLFD)
    {
        if(FT_New_Face(itsFt.library, file.local8Bit(), 0, &itsFt.face))
            return false;
        else
            itsFt.open=true;
    }

    if(TEST==mask)
        status=true; // If we've reached this point, then the FT_New_Face worked!
    else
    {
        CCompressedFile f(file.local8Bit());
 
        if(f)
        {
            char          data[constHeaderMaxLen];
            unsigned char *hdr=(unsigned char *)data;

            int bytesRead=f.read(data, constHeaderMaxLen);
            f.close();
 
            data[bytesRead-1]='\0';

            bool binary=(hdr[0]==0x80 && hdr[1]==0x01) || (hdr[0]==0x01 && hdr[1]==0x80);

            if(bytesRead>2 && ( binary || (strstr(data, "%!")==data) ) )
            {
                const char *header,
                           *str,
                           *dict;
                char       *end;
                bool       foundName=false,
                           foundFamily=false,
                           foundPs=false,
                           foundNotice=false,
                           foundEncoding=false,
                           familyIsFull=false;

                header=binary ? &data[6] : data;

                // Look for start of 'dict' section...
                if((dict=strstr(header, "dict begin"))!=NULL)
                {
                    // Now look for the end of the 'dict' section
                    if((end=strstr(dict, "currentdict end"))!=NULL)
                        *end='\0';  // If found, then set to NULL - this should speed up the following strstr's

                    // Having found the 'dict' section, now try to read the data...

                    if(NULL!=(str=getTokenT1(dict, "/Encoding")))
                    {
                        itsEncoding=str;
                        foundEncoding=true;
                    }

                    if((mask&NAME || mask&XLFD || mask&PROPERTIES) && !foundName && NULL!=(str=getReadOnlyTokenT1(dict, "/FullName")))
                    {
                        itsFullName=str;
                        foundName=true;
                    }

                    if((mask&NAME || mask&XLFD || mask&PROPERTIES) && NULL!=(str=getTokenT1(dict, "/FontName")))
                    {
                        itsPsName=str[0]=='/' ? &str[1] : str;
                        foundPs=true;
                    }

                    if(mask&NAME || mask&PROPERTIES || mask&XLFD)
                    {
                        if(NULL!=(str=getReadOnlyTokenT1(dict, "/FamilyName")))
                        {
                            itsFamily=str;
                            foundFamily=true;
                        }
                        if(NULL!=(str=getReadOnlyTokenT1(dict, "/Weight")))
                            itsWeight=strToWeight(str);
                        if(NULL!=(str=getTokenT1(dict, "/ItalicAngle")))
                        {
                            itsItalicAngle=atof(str);
                            itsItalic=itsItalicAngle== 0.0f ? ITALIC_NONE : ITALIC_ITALIC;
                        }
                    }

                    if(mask&XLFD)
                    {
                        if(NULL!=(str=getTokenT1(dict, "/isFixedPitch")))
                            itsSpacing=strstr(str, "false")==str ? SPACING_PROPORTIONAL : SPACING_MONOSPACED;
                        if(NULL!=(str=getReadOnlyTokenT1(dict, "/Notice")))
                        {
                            const TT1AndSpdFoundryMap *map;

                            itsFoundry=constDefaultFoundry;
                            for(map=constT1AndSpdFoundries; NULL != map->foundry; map++)
                                if(strstr(str, map->noticeStr)!=NULL)
                                {
                                    itsFoundry=map->foundry;
                                    break;
                                }

                            foundNotice=true;
                        }
                    }

                    if(mask&XLFD && !foundNotice)
                    {
                        foundNotice=true;
                        itsFoundry=constDefaultFoundry;
                    }
                }

                if(mask&NAME || mask&PROPERTIES || mask&XLFD)
                    if(!foundName && foundPs)  // The Hershey fonts don't have the FullName or FamilyName parameters!...
                    {
                        itsFullName=itsPsName;
                        itsFullName.replace(QRegExp("\\-"), " ");
                        foundName=true;
                    }

                if(mask&PROPERTIES || mask&XLFD)
                    if(!foundFamily && foundName)
                    {
                        itsFamily=itsFullName;
                        familyIsFull=true;
                        foundFamily=true;
                    }

                if((mask&XLFD || mask&NAME) && foundName)
                    itsWidth=strToWidth(itsFullName);

                if(mask&XLFD && !foundNotice)
                {
                    foundNotice=true;
                    itsFoundry=constDefaultFoundry;
                }

                if(foundName && (mask&PROPERTIES || mask&XLFD || mask&NAME))
                    if(ITALIC_ITALIC==itsItalic)
                    {
                        const char *constOblique="Oblique";

                        int pos=itsFullName.find(constOblique);

                        if(-1!=pos && pos==(int)(itsFullName.length()-strlen(constOblique)))
                            itsItalic=ITALIC_OBLIQUE;
                    }

                if(foundName && foundFamily)
                    itsFamily=createNames(familyIsFull ? QString::null : itsFamily, itsFullName);

                status= ( (mask&NAME && !foundName) || (mask&PROPERTIES && (!foundPs || !foundFamily)) ||
                        (mask&XLFD && (!foundNotice || !foundName || !foundEncoding)) ) ? false : true;
            }
        }
    }

    return status;
}

QStringList CFontEngine::getEncodingsT1()
{
    QStringList enc;

    if(getIsArrayEncodingT1())
        enc.append(CEncodings::constT1Symbol);
    else
        enc=getEncodingsFt();

    return enc;
}

QStringList CFontEngine::get8BitEncodingsT1()
{
    QStringList enc;

    if(getIsArrayEncodingT1())
        enc.append(CEncodings::constT1Symbol);
    else
        enc=get8BitEncodingsFt();

    return enc;
}

bool CFontEngine::getIsArrayEncodingT1()
{
    return itsType==TYPE_1 && itsEncoding.find("array")!=-1 ? true : false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    TRUE TYPE
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontTT(const QString &file, unsigned short mask)
{
    bool status=FT_New_Face(itsFt.library, file.local8Bit(), 0, &itsFt.face) ? false : true;

    if(status)
        itsFt.open=true;

    if(TEST!=mask && status)
    {
        if(mask&NAME || mask&PROPERTIES)
        {
            itsFullName=lookupNameTT(TT_NAME_ID_FULL_NAME);
            itsFamily=lookupNameTT(TT_NAME_ID_FONT_FAMILY);

            if(mask&PROPERTIES)
            {
                void *table=NULL;
                //
                // Algorithm taken from ttf2pt1.c ...
                //
                QString psName=lookupNameTT(TT_NAME_ID_PS_NAME);

                if(QString::null==psName)
                    psName=itsFullName;

                itsPsName=psName;

                // Must not start with a digit
                if(QString::null!=itsPsName)
                {
                    unsigned int ch,
                                 ch2;

                    if(itsPsName[0].isDigit())
                        itsPsName[0]=itsPsName.local8Bit()[0]+('A'-'0');

                    for(ch=1; ch<itsPsName.length(); ++ch)
                        if('_'==itsPsName.local8Bit()[ch] || ' '==itsPsName.local8Bit()[ch])
                            for(ch2=ch; ch2<itsPsName.length()-1; ++ch2)
                                itsPsName[ch2]=itsPsName[ch2+1];
                }

                bool gotItalic=false;

                if(NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post)))
                {
                    itsItalic=ITALIC_NONE;
                    itsItalicAngle=0;
                }
                else
                {
                    struct TFixed
                    {
                        TFixed(unsigned long v) : upper(v>>16), lower(v&0xFFFF) {}

                         short upper,
                               lower;

                         float value() { return upper+(lower/65536.0); }
                    };

		    gotItalic=true;
		    itsItalicAngle=((TFixed)((TT_Postscript*)table)->italicAngle).value();
		    itsItalic=itsItalicAngle== 0.0f ? ITALIC_NONE : ITALIC_ITALIC;
		}

		if((NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2))) || (0xFFFF==((TT_OS2*)table)->version) )
		{
			itsWeight=WEIGHT_UNKNOWN;
			if(!gotItalic)
                    {
                        itsItalicAngle=0;
                        itsItalic=ITALIC_NONE;
                    }
                }
                else
                {
                    itsWeight=mapWeightTT(((TT_OS2*)table)->usWeightClass);
                    if(!gotItalic)
                    {
                        itsItalic=((TT_OS2*)table)->fsSelection&(1 << 0) ? ITALIC_ITALIC : ITALIC_NONE;
                        itsItalicAngle=ITALIC_ITALIC==itsItalic ? -12 : 0 ; // Hmm...
                    }
                }

                if(itsItalicAngle>45.0 || itsItalicAngle<-45.0)
                    itsItalicAngle=0.0;
 
                if((NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2))) || (0xFFFF==((TT_OS2*)table)->version) )
                    itsWidth=WIDTH_UNKNOWN;
                else
                    itsWidth=mapWidthTT(((TT_OS2*)table)->usWidthClass);
            }

            if(mask&XLFD)
            {
                const TTtfFoundryMap *map;
                char                 code[5];
                void                 *table=NULL;

                if((NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2))) || (0xFFFF==((TT_OS2*)table)->version) )
                    code[0]=code[1]=code[2]=code[3]=code[4]='-';
                else
                {
                    code[0]=toupper(((TT_OS2*)table)->achVendID[0]);
                    code[1]=toupper(((TT_OS2*)table)->achVendID[1]);
                    code[2]=toupper(((TT_OS2*)table)->achVendID[2]);
                    code[3]=toupper(((TT_OS2*)table)->achVendID[3]);
                    code[4]='\0';
                }

                if(NULL==(table=FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post)))
                    itsSpacing=SPACING_PROPORTIONAL;
                else
                    itsSpacing=((TT_Postscript*)table)->isFixedPitch ? SPACING_MONOSPACED : SPACING_PROPORTIONAL;
            
                unsigned int slen=strlen(code);

                itsFoundry=constDefaultFoundry;

                for(map=constTtfFoundries; NULL!=map->vendorId; map++)
                    if(memcmp(map->vendorId, code, slen)==0)
                    {
                        itsFoundry=map->foundry;
                        break;
                    }
            }

            if(mask&NAME || mask&PROPERTIES)
                itsFamily=createNames(itsFamily, itsFullName);
        }
    }

    return status;
}

CFontEngine::EWeight CFontEngine::mapWeightTT(FT_UShort os2Weight)
{
    enum ETtfWeight
    {
        TTF_WEIGHT_UNKNOWN    = 0,
        TTF_WEIGHT_THIN       = 100,
        TTF_WEIGHT_EXTRALIGHT = 200,
        TTF_WEIGHT_LIGHT      = 300,
        TTF_WEIGHT_NORMAL     = 400,
        TTF_WEIGHT_MEDIUM     = 500,
        TTF_WEIGHT_SEMIBOLD   = 600,
        TTF_WEIGHT_BOLD       = 700,
        TTF_WEIGHT_EXTRABOLD  = 800,
        TTF_WEIGHT_BLACK      = 900
    };

    switch(os2Weight)
    {
        case TTF_WEIGHT_THIN:
            return WEIGHT_THIN;
        case TTF_WEIGHT_EXTRALIGHT:
            return WEIGHT_EXTRA_LIGHT;
        case TTF_WEIGHT_LIGHT:
            return WEIGHT_LIGHT;
        case TTF_WEIGHT_NORMAL:
        case TTF_WEIGHT_MEDIUM:
            return WEIGHT_MEDIUM;
        case TTF_WEIGHT_SEMIBOLD:
            return WEIGHT_SEMI_BOLD;
        case TTF_WEIGHT_BOLD:
            return WEIGHT_BOLD;
        case TTF_WEIGHT_EXTRABOLD:
            return WEIGHT_EXTRA_BOLD;
        case TTF_WEIGHT_BLACK:
            return WEIGHT_BLACK;
        case TTF_WEIGHT_UNKNOWN:
        default:
            return WEIGHT_UNKNOWN;
    }
}
 
CFontEngine::EWidth CFontEngine::mapWidthTT(FT_UShort os2Width)
{
    enum ETtfWidth
    {
        TTF_WIDTH_ULTRA_CONDENSED = 1,
        TTF_WIDTH_EXTRA_CONDENSED = 2,
        TTF_WIDTH_CONDENSED       = 3,
        TTF_WIDTH_SEMI_CONDENSED  = 4,
        TTF_WIDTH_NORMAL          = 5,
        TTF_WIDTH_SEMI_EXPANDED   = 6,
        TTF_WIDTH_EXPANDED        = 7,
        TTF_WIDTH_EXTRA_EXPANDED  = 8,
        TTF_WIDTH_ULTRA_EXPANDED  = 9
    };

    switch(os2Width)
    {
        case TTF_WIDTH_ULTRA_CONDENSED:
            return WIDTH_ULTRA_CONDENSED;
        case TTF_WIDTH_EXTRA_CONDENSED:
            return WIDTH_EXTRA_CONDENSED;
        case TTF_WIDTH_CONDENSED:
            return WIDTH_CONDENSED;
        case TTF_WIDTH_SEMI_CONDENSED:
            return WIDTH_SEMI_CONDENSED;
        case TTF_WIDTH_NORMAL:
            return WIDTH_NORMAL;
        case TTF_WIDTH_SEMI_EXPANDED:
            return WIDTH_SEMI_EXPANDED;
        case TTF_WIDTH_EXPANDED:
            return WIDTH_EXPANDED;
        case TTF_WIDTH_EXTRA_EXPANDED:
            return WIDTH_EXTRA_EXPANDED;
        case TTF_WIDTH_ULTRA_EXPANDED:
            return WIDTH_ULTRA_EXPANDED;
        default:
            return WIDTH_UNKNOWN;
    }
}

QCString CFontEngine::lookupNameTT(int index)  // Code copied from freetype/ftdump.c
{
    unsigned int i;
    int          j;
    bool         found=false;
    QCString     buffer;
 
    FT_UInt     numNames=FT_Get_Sfnt_Name_Count(itsFt.face);
    FT_SfntName fName;
 
    for(i=0; !found && i<numNames && !FT_Get_Sfnt_Name(itsFt.face, i, &fName); ++i)
        if(fName.name_id==index )
        {
            // The following code was inspired from Mark Leisher's ttf2bdf package
 
            // Try to find a Microsoft English name
            if (fName.platform_id==TT_PLATFORM_MICROSOFT)
                for(j=TT_MS_ID_UNICODE_CS; j>=TT_MS_ID_SYMBOL_CS; j-- )
                    if((fName.encoding_id==j) && ((fName.language_id&0x3FF)==0x009))
                    {
                        found=true;
                        break;
                    }
 
            if (!found && fName.platform_id==TT_PLATFORM_APPLE_UNICODE && fName.language_id==TT_MAC_LANGID_ENGLISH)
                found=true;
 
            // Found a Unicode Name.
            if(found)
                for(i=1; i<fName.string_len; i+=2)
                    buffer+=((char *)(fName.string))[i];
        }

    return buffer;
}

bool CFontEngine::has16BitEncodingFt(const QString &enc)
{
    if(enc=="jisx0208.1983-0" || enc=="jisx0201.1976-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_sjis) ? false : true;
    else if(enc=="gb2312.1980-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_gb2312) ? false : true;
    else if(enc=="big5.et-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_big5) ? false : true;
    else if(enc=="ksc5601.1987-0")
        return FT_Select_Charmap(itsFt.face, ft_encoding_wansung) ?  FT_Select_Charmap(itsFt.face, ft_encoding_johab) ?  false : true : true;
    else
        return false;
}

bool CFontEngine::has8BitEncodingFt(CEncodings::T8Bit *data)
{
    if(data)
    {
        if(data->load())
        {
            int cm;

            for(cm=0; cm<itsFt.face->num_charmaps; cm++)
            {
                static const int constMaxMissing=5;

                int ch,
                    missing=0;
                setCharmapFt(itsFt.face->charmaps[cm]);
                for(ch=0; ch<CEncodings::T8Bit::NUM_MAP_ENTRIES; ch++)
                    if(data->map[ch]>-1 && FT_Get_Char_Index(itsFt.face, data->map[ch])==0)
                        missing++;
                if(missing<=constMaxMissing)
                    return true;
            }
        }
    }

    return false;
}

QStringList CFontEngine::get8BitEncodingsFt()
{
    QStringList enc;
 
    // Do 8-bit encodings...
    CEncodings::T8Bit *enc8;
 
    for(enc8=CKfiGlobal::enc().first8Bit(); enc8; enc8=CKfiGlobal::enc().next8Bit())
        if(has8BitEncodingFt(enc8))
            enc.append(enc8->name);

    return enc;
}

QStringList CFontEngine::getEncodingsFt()
{
    QStringList enc;
    bool        exclusive=false;

    // Check if font has the 'exclusive' encoding, if so just use this...
    if(CKfiGlobal::cfg().getExclusiveEncoding())
        if(CKfiGlobal::cfg().getEncoding()==CEncodings::constUnicodeStr ||
           has8BitEncodingFt(CKfiGlobal::enc().get8Bit(CKfiGlobal::cfg().getEncoding())) ||
           has16BitEncodingFt(CKfiGlobal::cfg().getEncoding()))
        {
            enc.append(CKfiGlobal::cfg().getEncoding());
            exclusive=true;
        }

    if(!exclusive) // Either 'use exclusive' is false, or the font doesn't contain it...
    {
        // Check for symbol encoding...
        if(setCharmapSymbolFt())
            enc.append(itsType==TYPE_1 ? CEncodings::constT1Symbol : CEncodings::constTTSymbol);
        else
        {
            // Add Unicode encoding...
            if(setCharmapUnicodeFt())
                enc.append(CEncodings::constUnicodeStr);

            // Do 8-bit encodings...
            enc+=get8BitEncodingsFt();

            // Do 16-bit encodings...
            CEncodings::T16Bit *enc16;

            for(enc16=CKfiGlobal::enc().first16Bit(); enc16; enc16=CKfiGlobal::enc().next16Bit())
                if(has16BitEncodingFt(enc16->name))
                    enc.append(enc16->name);
        }
    }
 
    return enc;
}

QPixmap CFontEngine::createPixmapFt(const QString &str, int width, int height, int pointSize, int resolution, QRgb backgroundColour)
{
    QPixmap pix;
    bool    symbol=false,
            t1array=false;

    if(QString::null!=str && str.length() && !FT_Set_Char_Size(itsFt.face, 0, pointSize*64, resolution, resolution) &&
      ((t1array=(TYPE_1==itsType && getIsArrayEncodingT1())) || setCharmapUnicodeFt() || (symbol=setCharmapSymbolFt()) ))
    {
        int oldW=itsFt.bmp.w,
            oldH=itsFt.bmp.h;

        itsFt.bmp.w=width;
        itsFt.bmp.h=height;

        // Delete any previous bitmap if size is differnt to previous
        if(NULL!=itsFt.bmp.data && oldW!=itsFt.bmp.w && oldH!=itsFt.bmp.h)
        {
            delete itsFt.bmp.data;
            itsFt.bmp.data=NULL;
        }
 
        if(NULL==itsFt.bmp.data)
            itsFt.bmp.data=new unsigned char[itsFt.bmp.w*itsFt.bmp.h];
 
        if(NULL!=itsFt.bmp.data)
        {
            FT_GlyphSlot   slot=itsFt.face->glyph;
            unsigned int   ch=0;
            int            maxAscender=0,
                           maxHeight=0;
            unsigned short code;

            for(ch=0; ch<str.length(); ++ch)
            {
                code=symbol | t1array ?
                                ( CKfiGlobal::fe().getType()==CFontEngine::TRUE_TYPE ?
                                      CKfiGlobal::fe().getGlyphIndexFt(str[ch].unicode()+CEncodings::MS_SYMBOL_MODIFIER) :
                                      ch
                                ) :
                                CKfiGlobal::fe().getGlyphIndexFt(str[ch].unicode());

                if(!FT_Load_Glyph(itsFt.face, code, FT_LOAD_NO_SCALE))
                {
                    if(abs(slot->metrics.horiBearingY)>maxAscender)
                        maxAscender=abs(slot->metrics.horiBearingY);
                    if(abs(slot->metrics.height)>maxHeight)
                        maxHeight=abs(slot->metrics.height);
                }
            }

            maxHeight>>=6;
            maxAscender>>=6;

            int           pos=0,
                          fHeight=maxHeight,
                          yoffset=fHeight<itsFt.bmp.h ? (itsFt.bmp.h-fHeight)/2 : 0,
                          ascender=maxAscender;

            memset(itsFt.bmp.data, 0, itsFt.bmp.w*itsFt.bmp.h);

            for(ch=0; ch<str.length(); ++ch)
            {
                code=symbol | t1array ?
                                ( CKfiGlobal::fe().getType()==CFontEngine::TRUE_TYPE ?
                                      CKfiGlobal::fe().getGlyphIndexFt(str[ch].unicode()+CEncodings::MS_SYMBOL_MODIFIER) :
                                      ch
                                ) :
                                CKfiGlobal::fe().getGlyphIndexFt(str[ch].unicode());

                if(!FT_Load_Glyph(itsFt.face, code, FT_LOAD_DEFAULT) && !FT_Render_Glyph(itsFt.face->glyph, ft_render_mode_normal))
                {
                    int row,
                        ypos=(ascender-slot->bitmap_top)+yoffset,//(fHeight-(slot->bitmap_top+descender))+yoffset,
                        gWidth=slot->bitmap.width ? slot->bitmap.width : pointSize/4,
                        width=(pos+gWidth)<itsFt.bmp.w ? gWidth : itsFt.bmp.w-(pos+1);

                    if(width)
                        for(row=0; row<slot->bitmap.rows; ++row)
                        {
                            if(row+ypos>=0 && row+ypos<itsFt.bmp.h && ((row+ypos)*itsFt.bmp.w)+pos+width < (itsFt.bmp.w*itsFt.bmp.h))
                                memcpy(&(itsFt.bmp.data[((row+ypos)*itsFt.bmp.w)+pos]), &(slot->bitmap.buffer[row*slot->bitmap.pitch]), width);
                        }
                    else
                        break;

                    if(width<slot->bitmap.width)
                        break;
 
                    pos+=slot->bitmap.width+2;
                    if(pos>=itsFt.bmp.w)
                        break;
                }
            }

            static QRgb clut [256];
            static bool clutSetup=false;

            if(!clutSetup)
            {
                int i;
                for(i=0; i<256; i++)
                    clut[i]=qRgb(255-i, 255-i, 255-i);
                clutSetup=true;
            }

            QImage img(itsFt.bmp.data, itsFt.bmp.w, itsFt.bmp.h, 8, clut, 256, QImage::IgnoreEndian); // Endian????

            img.setColor(0, backgroundColour);
            pix=img;
        }
    }
 
    return pix;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    SPEEDO
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CFontEngine::openFontSpd(const QString &file, unsigned short mask)
{
    const int           constHeaderSize=420;
    const int           constShortNameOffset=298;
    const int           constShortNameNumBytes=16;
    const int           constSourceFontNameOffset=24;
    const int           constSourceFontNameNumBytes=70;
    const int           constClassificationOffset=265;
    const unsigned char constWidthMask=0x0f;
    const unsigned char constWeightMask=0xf0;
    const int           constWeightShift=4;
    const int           constItalicOffset=328;
    const int           constSpacingOffset=264;
    const int           constMonospaced=3;
    const int           constNoticeOffset=174;
    const int           constNoticeNumBytes=78;

    enum ESpeedoWidth
    {
        SPD_WIDTH_CONDENSED=4,
        SPD_WIDTH_SEMI_CONDENSED=6,
        SPD_WIDTH_NORMAL=8,
        SPD_WIDTH_SEMI_EXPANDED=10,
        SPD_WIDTH_EXPANDED=12
    };

    enum ESpeedoWeight
    {
        SPD_WEIGHT_UNKNOWN=0,
        SPD_WEIGHT_THIN,
        SPD_WEIGHT_ULTRA_LIGHT,
        SPD_WEIGHT_EXTRA_LIGHT,
        SPD_WEIGHT_LIGHT,
        SPD_WEIGHT_BOOK,
        SPD_WEIGHT_NORMAL,
        SPD_WEIGHT_MEDIUM,
        SPD_WEIGHT_SEMI_BOLD,
        SPD_WEIGHT_DEMI_BOLD,
        SPD_WEIGHT_BOLD,
        SPD_WEIGHT_EXTRA_BOLD,
        SPD_WEIGHT_ULTRA_BOLD,
        SPD_WEIGHT_HEAVY,
        SPD_WEIGHT_BLACK
    };

    bool     status=false;
    ifstream spd(file.local8Bit());

    if(spd)
    {
        char hdr[constHeaderSize];

        spd.read(hdr, sizeof(char)*constHeaderSize);
        if(spd.good())
        {
            //
            // The first 4 bytes of s Speedo font are assumed to be:
            //    1. 'D' (or maybe 'd' ?)
            //    2. A number (usually 1 or 4)
            //    3. Decimal point
            //    4. A number (usually 0)
            if((hdr[0]=='D' || hdr[0]=='d') && isdigit(hdr[1]) && hdr[2]=='.' && isdigit(hdr[3]))
            {
                char shortName[constShortNameNumBytes+1];
                memcpy(shortName, &hdr[constShortNameOffset], constShortNameNumBytes);
                shortName[constShortNameNumBytes]='\0';
                itsFamily=shortName;

                char sourceName[constSourceFontNameNumBytes+1];
                memcpy(sourceName, &hdr[constSourceFontNameOffset], constSourceFontNameNumBytes);
                sourceName[constSourceFontNameNumBytes]='\0';
                itsFullName=sourceName;

                itsFamily=createNames(itsFamily, itsFullName);

                itsPsName=constNoPsName;
                status=true;

                if(mask&NAME || mask&PROPERTIES)
                {
                    switch((hdr[constClassificationOffset]&constWeightMask)>>constWeightShift)
                    {
                        case SPD_WEIGHT_THIN:
                            itsWeight=WEIGHT_THIN;
                            break;
                        case SPD_WEIGHT_ULTRA_LIGHT:
                            itsWeight=WEIGHT_ULTRA_LIGHT;
                            break;
                        case SPD_WEIGHT_EXTRA_LIGHT:
                            itsWeight=WEIGHT_EXTRA_LIGHT;
                            break;
                        case SPD_WEIGHT_LIGHT:
                            itsWeight=WEIGHT_LIGHT;
                            break;
                        case SPD_WEIGHT_BOOK:
                            itsWeight=WEIGHT_BOOK;
                            break;
                        case SPD_WEIGHT_NORMAL:
                        case SPD_WEIGHT_MEDIUM:
                            itsWeight=WEIGHT_MEDIUM;
                            break;
                        case SPD_WEIGHT_SEMI_BOLD:
                            itsWeight=WEIGHT_SEMI_BOLD;
                            break;
                        case SPD_WEIGHT_DEMI_BOLD:
                            itsWeight=WEIGHT_DEMI_BOLD;
                            break;
                        case SPD_WEIGHT_BOLD:
                            itsWeight=WEIGHT_BOLD;
                            break;
                        case SPD_WEIGHT_EXTRA_BOLD:
                            itsWeight=WEIGHT_EXTRA_BOLD;
                            break;
                        case SPD_WEIGHT_ULTRA_BOLD:
                            itsWeight=WEIGHT_ULTRA_BOLD;
                            break;
                        case SPD_WEIGHT_HEAVY:
                            itsWeight=WEIGHT_HEAVY;
                            break;
                        case SPD_WEIGHT_BLACK:
                            itsWeight=WEIGHT_BLACK;
                            break;
                        case SPD_WEIGHT_UNKNOWN:
                        default:
                            itsWeight=WEIGHT_UNKNOWN;
                    }

                    itsItalic=(0==(hdr[constItalicOffset]<<8 + hdr[constItalicOffset+1])) ? ITALIC_NONE : ITALIC_ITALIC;

                    switch(hdr[constClassificationOffset]&constWidthMask)
                    {
                        case SPD_WIDTH_CONDENSED:
                            itsWidth=WIDTH_CONDENSED;
                            break;
                        case SPD_WIDTH_SEMI_CONDENSED:
                            itsWidth=WIDTH_SEMI_CONDENSED;
                            break;
                        case SPD_WIDTH_NORMAL:
                            itsWidth=WIDTH_NORMAL;
                            break;
                        case SPD_WIDTH_SEMI_EXPANDED:
                            itsWidth=WIDTH_SEMI_EXPANDED;
                            break;
                        case SPD_WIDTH_EXPANDED:
                            itsWidth=WIDTH_EXPANDED;
                            break;
                        default:
                            itsWidth=WIDTH_UNKNOWN;
                    }
                }

                if(mask&XLFD)
                {
                    itsSpacing=hdr[constSpacingOffset]==constMonospaced ? SPACING_MONOSPACED : SPACING_PROPORTIONAL;

                    hdr[constNoticeOffset+constNoticeNumBytes]='\0';

                    const TT1AndSpdFoundryMap *map;

                    itsFoundry=constDefaultFoundry;

                    for(map=constT1AndSpdFoundries; NULL != map->foundry; map++)
                        if(strstr((const char *)&hdr[constNoticeOffset], map->noticeStr)!=NULL)
                        {
                            itsFoundry=map->foundry;
                            break;
                        }
                }
            }
        }
        spd.close();
    }

    return status;
}

QStringList CFontEngine::getEncodingsSpd()
{
    QStringList enc;

    enc.append("iso8859-1"); 
    return enc;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//    BITMAP
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const unsigned int constBitmapMaxProps=1024;

bool CFontEngine::openFontBmp(const QString &file, unsigned short mask)
{
    itsFoundry=constDefaultFoundry;

    if(TEST==mask)
        mask=XLFD;

    if(isAPcf(file.local8Bit()))
        return openFontPcf(file, mask);
    if(isABdf(file.local8Bit()))
        return openFontBdf(file, mask);
    if(isASnf(file.local8Bit()))
        return openFontSnf(file, mask);
    return false;
}

void CFontEngine::createNameBmp(int pointSize, int res, const QString &enc)
{
    QString ptStr,
            resStr;

    ptStr.setNum(pointSize/10);
    resStr.setNum(res);
    itsFullName=itsFamily+" "+weightStr(itsWeight)+(ITALIC_ITALIC==itsItalic ? constBmpItalic : ITALIC_OBLIQUE==itsItalic ? constBmpOblique : constBmpRoman)+
                      " ("+ptStr+"pt / "+resStr+"dpi / "+enc+")";
}

static CFontEngine::EItalic charToItalic(char c)
{
    switch(c)
    {
        case 'i':
        case 'I':
            return CFontEngine::ITALIC_ITALIC;
        case 'o':
        case 'O':
            return CFontEngine::ITALIC_OBLIQUE;
        case 'r':
        case 'R':
        default:
            return CFontEngine::ITALIC_NONE;
    }
}

void CFontEngine::createNameFromXlfdBmp()
{
    enum EXlfd
    {
        XLFD_FOUNDRY=0,
        XLFD_FAMILY,
        XLFD_WEIGHT,
        XLFD_SLANT,
        XLFD_WIDTH,
        XLFD_UKNOWN,
        XLFD_PIXEL_SIZE,
        XLFD_POINT_SIZE,
        XLFD_RESX,
        XLFD_RESY,
        XLFD_SPACING,
        XLFD_AV_WIDTH,
        XLFD_ENCODING,
        XLFD_END
    };

    int     pos=0,
            oldPos=0,
            pointSize=0,
            res=0;
    QString enc;
    int     entry;

    // XLFD:
    //     -foundry-family-weight-slant-width-?-pixelSize-pointSize-resX-resY-spacing-avWidth-csReg-csEnc

    for(entry=XLFD_FOUNDRY; -1!=(pos=itsXlfd.find('-', pos+1)) && entry<XLFD_END; ++entry)
    {
        switch(entry)
        {
            default:
            case XLFD_FOUNDRY:
                break;
            case XLFD_FAMILY:
                itsFamily=itsXlfd.mid(oldPos, pos-oldPos);
                break;
            case XLFD_WEIGHT:
                itsWeight=strToWeight(itsXlfd.mid(oldPos, pos-oldPos).local8Bit());
                break;
            case XLFD_SLANT:
                if(pos>0)
                    itsItalic=charToItalic(itsXlfd[pos-1].latin1());
                break;
            case XLFD_WIDTH:
            case XLFD_UKNOWN:
            case XLFD_PIXEL_SIZE:
                break;
            case XLFD_POINT_SIZE:
                pointSize=itsXlfd.mid(oldPos, pos-oldPos).toInt();
                break;
            case XLFD_RESX:
                res=itsXlfd.mid(oldPos, pos-oldPos).toInt();
                break;
            case XLFD_RESY:
            case XLFD_SPACING:
            case XLFD_AV_WIDTH:
                break;
            case XLFD_ENCODING:
                enc=itsXlfd.mid(oldPos);
                break;
        }

        oldPos=pos+1;
    }

    if(XLFD_END==entry)
        createNameBmp(pointSize, res, enc);
    else
        itsFullName=itsXlfd;
}

bool CFontEngine::getFileEncodingBmp(const char *str)
{
    if(strlen(str)) 
    {
         int ch,
             numDash=0;

        for(ch=strlen(str)-1; ch>=0; ch--)
            if(str[ch]=='-')
                if(++numDash==2)
                {
                    itsEncoding=&str[ch+1];
                    return true;
                }
    }

    return false;
}

static const char * getTokenBdf(const char *str, const char *key, bool noquotes=false)
{
    char         *s=NULL;
    unsigned int keyLen=strlen(key),
                 sLen=strlen(str);

    if(keyLen+1<sLen && NULL!=(s=strstr(str, key)) && (s==str || (!isalnum(s[-1]) && '_'!=s[-1])) && (!noquotes || (noquotes && s[keyLen+1]=='-')))
    {
        const int   constMaxTokenSize=256;
        static char token[constMaxTokenSize];

        char        *end=NULL;
 
        strncpy(token, s, constMaxTokenSize);
        token[constMaxTokenSize-1]='\0';

        if(noquotes)
        {
            s+=strlen(key)+1;
            if(NULL!=(end=strchr(s, '\n')))
            {
                *end='\0';
                return s;
            }
        }
        else 
            if(NULL!=(s=strchr(token, '\"')))
            {
                s++;
                if(NULL!=(end=strchr(s, '\"')))
                {
                    *end='\0';
                    return s;
                }
            }
    }
 
    return NULL;
}

bool CFontEngine::openFontBdf(const QString &file, unsigned short mask)
{
    bool            status=false;
    CCompressedFile bdf(file.local8Bit());

    if(bdf)
    {
        const int constMaxLineLen=1024;

        char    buffer[constMaxLineLen];
        bool    foundFamily=false,
                foundWeight=false,
                foundRes=false,
                foundSlant=false,
                foundPt=false,
                foundEncoding=false,
                foundCsReg=false,
                foundCsEnc=false,
                foundXlfd=false;
        QString csReg,
                csEnc;
        int     res=0,
                pointSize=0;

        while(NULL!=bdf.getString(buffer, constMaxLineLen) && !status)
        {
            const char *str;

            if(XLFD!=mask)
            {
                if(!foundFamily && NULL!=(str=getTokenBdf(buffer, "FAMILY_NAME")))
                {
                    itsFamily=str;
                    foundFamily=true;
                }
                else if(!foundWeight && NULL!=(str=getTokenBdf(buffer, "WEIGHT_NAME")))
                {
                    itsWeight=strToWeight(str); 
                    foundWeight=true;
                }
                else if(!foundSlant && NULL!=(str=getTokenBdf(buffer, "SLANT")))
                {
                    itsItalic=charToItalic(str[0]);
                    foundSlant=true;
                }
                else if(!foundRes && NULL!=(str=strstr(buffer, "RESOLUTION_X")))
                {
                    res=atoi(str+strlen("RESOLUTION_X"));
                    foundRes=true;
                }
                else if(!foundPt && NULL!=(str=strstr(buffer, "POINT_SIZE")))
                {
                    pointSize=atoi(str+strlen("POINT_SIZE"));
                    foundPt=true;
                }
                else if(!foundCsReg && NULL!=(str=getTokenBdf(buffer, "CHARSET_REGISTRY")))
                {
                    if(strlen(str))
                    {
                        csReg=str;
                        foundCsReg=true;
                    }
                }
                else if(!foundCsEnc && NULL!=(str=getTokenBdf(buffer, "CHARSET_ENCODING")))
                {
                    if(strlen(str))
                    {
                        csEnc=str;
                        foundCsEnc=true;
                    }
                }
            }

            if(!foundXlfd && NULL!=(str=getTokenBdf(buffer, "FONT", true)))   // "FONT" does not have quotes!
            {
                if(strlen(str))
                {
                    itsXlfd=str;
                    foundXlfd=true;
                }
            }
            else if(!foundCsEnc && !foundCsReg && foundXlfd)
                foundCsEnc=foundCsReg=foundEncoding=getFileEncodingBmp(itsXlfd.local8Bit());

            if(!foundEncoding && foundCsReg && foundCsEnc)
            {
                itsEncoding=csReg+"-"+csEnc;
                foundEncoding=true;
            }

            if(mask==XLFD && foundXlfd)
                status=true;
            else
                if(mask!=XLFD && (foundFamily && foundWeight && foundRes && foundSlant && foundPt && foundEncoding && (!(mask&XLFD) || (mask&XLFD && foundXlfd))))
                {
                    createNameBmp(pointSize, res, itsEncoding);
                    status=true;
                }
        }

        if(!status && foundXlfd)
        {
            createNameFromXlfdBmp();
            status=true;
        }
    }

    return status;
}

static const char * readStrSnf(CCompressedFile &f)
{
    static const int constMaxChars=512;
 
    static char buffer[constMaxChars];
    int         pos=0;
    char        ch;
 
    buffer[0]='\0';
 
    while(-1!=(ch=f.getChar()))
    {
        buffer[pos++]=ch;
        if('\0'==ch)
            break;
    }
 
    return buffer;
}

bool CFontEngine::openFontSnf(const QString &file, unsigned short mask)
{
    bool status=false;
 
    struct TCharInfo
    {
        bool         exists()     { return ntohl(misc)&0x80; }
        unsigned int byteOffset() { return (ntohl(misc)&0xFFFFFF00) >> 8; }
 
        short        leftSideBearing,
                     rightSideBearing,
                     characterWidth,
                     ascent,
                     descent,
                     attributes;
        unsigned int misc;
    };
 
    struct TGenInfo
    {
        unsigned int version1,
                     allExist,
                     drawDirection,
                     noOverlap,
                     constantMetrics,
                     terminalFont;
        unsigned int misc;
        //unsigned int linear        : 1,
        //             constantWidth : 1,
        //             inkInside     : 1,
        //             inkMetrics    : 1,
        //             padding       : 28;
        unsigned int firstCol,
                     lastCol,
                     firstRow,
                     lastRow,
                     numProps,
                     lenStrings,
                     defaultChar;
        int          fontAscent,
                     fontDescent;
        TCharInfo    minBounds,
                     maxBounds;
        unsigned int pixDepth,
                     glyphSets,
                     version2;
    };
 
    struct TProp
    {
        unsigned int name,      // string offset of name
                     value,     // Num or string offset
                     indirect;  // true if value is a string
    };

    CCompressedFile snf(file.local8Bit());
 
    if(snf)
    {
        TGenInfo genInfo;
        bool     foundXlfd=false;
 
        if((snf.read(&genInfo, sizeof(TGenInfo))==sizeof(TGenInfo)) && (ntohl(genInfo.version1)==ntohl(genInfo.version2))
           && ntohl(genInfo.numProps)<constBitmapMaxProps)
        {
            TProp        *props=new TProp[ntohl(genInfo.numProps)];
            unsigned int numChars=((ntohl(genInfo.lastCol) - ntohl(genInfo.firstCol)) + 1) * ((ntohl(genInfo.lastRow) - ntohl(genInfo.firstRow)) + 1),
                         glyphInfoSize=((genInfo.maxBounds.byteOffset()+3) & ~0x3);

            if(props)
            {
                if(-1!=snf.seek(numChars*sizeof(TCharInfo)+glyphInfoSize, SEEK_CUR))  // Skip past character info & glyphs...
                {
                    bool         foundFamily=false,
                                 foundWeight=false,
                                 foundRes=false,
                                 foundSlant=false,
                                 foundPt=false,
                                 foundEncoding=false,
                                 foundCsReg=false,
                                 foundCsEnc=false,
                                 error=false;
                    QString      csReg,
                                 csEnc;
                    unsigned int p,
                                 res=0,
                                 pointSize=0;
 
                    // Now read properties data...
                    for(p=0; p<ntohl(genInfo.numProps); ++p)
                        if(snf.read(&props[p], sizeof(TProp))!=sizeof(TProp))
                        {
                            error=true;
                            break;
                        }
                    if(!error)
                    {
                        const unsigned int constMaxLen=1024;
 
                        char       buffer[constMaxLen];
                        const char *value=NULL,
                                   *name=NULL;
 
                        for(p=0; p<ntohl(genInfo.numProps) && !status; ++p)
                        {
                            if(ntohl(props[p].indirect))
                            {
                                if((ntohl(props[p].value)-ntohl(props[p].name))<=constMaxLen &&
                                   -1!=snf.read(buffer, ntohl(props[p].value)-ntohl(props[p].name)))
                                {
                                    name=buffer;
                                    value=readStrSnf(snf);

                                    if(mask!=XLFD)
                                    {
                                        if(!foundFamily && CMisc::stricmp(name, "FAMILY_NAME")==0)
                                        {
                                            foundFamily=true;
                                            itsFamily=value;
                                        }
                                        else if(!foundWeight && CMisc::stricmp(name, "WEIGHT_NAME")==0)
                                        {
                                            foundWeight=true;
                                            itsWeight=strToWeight(value);
                                        }
                                        else if(!foundSlant && CMisc::stricmp(name, "SLANT")==0)
                                        {
                                             foundSlant=true;
                                             itsItalic=charToItalic(value[0]);
                                        }
                                        else if(!foundCsReg && CMisc::stricmp(name, "CHARSET_REGISTRY")==0)
                                        {
                                            if(strlen(value))
                                            {
                                                foundCsReg=true;
                                                csReg=value;
                                            }
                                        }
                                        else if(!foundCsEnc && CMisc::stricmp(name, "CHARSET_ENCODING")==0)
                                        {
                                            if(strlen(value))
                                            {
                                                foundCsEnc=true;
                                                csEnc=value;
                                            }
                                        }
                                        else if(!foundCsEnc && !foundCsReg && foundXlfd)
                                            foundCsEnc=foundCsReg=foundEncoding=getFileEncodingBmp(itsXlfd.local8Bit());
                                    }

                                    if(!foundXlfd && CMisc::stricmp(name, "FONT")==0)
                                    {
                                        if(strlen(value))
                                        {
                                            foundXlfd=true;
                                            itsXlfd=value;
                                        }
                                    }
                                }
                                else
                                    break;
                            }
                            else
                                if(mask!=XLFD)
                                {
                                    name=readStrSnf(snf);
                                    if(!foundRes && CMisc::stricmp(name, "RESOLUTION_X")==0)
                                    {
                                        foundRes=true;
                                        res=ntohl(props[p].value);
                                    }
                                    else if(!foundPt && CMisc::stricmp(name, "POINT_SIZE")==0)
                                    {
                                        foundPt=true;
                                        pointSize=ntohl(props[p].value);
                                    }
                                }

                            if(!foundEncoding && foundCsReg && foundCsEnc)
                            {
                                itsEncoding=csReg+"-"+csEnc;
                                foundEncoding=true;
                            }

                            if(mask==XLFD && foundXlfd)
                                status=true;
                            else
                                if(mask!=XLFD &&
                                   (foundFamily && foundWeight && foundRes && foundSlant && foundPt && foundEncoding && (!(mask&XLFD) || (mask&XLFD && foundXlfd))))
                                {
                                    createNameBmp(pointSize, res, itsEncoding);
                                    status=true;
                                }
                        }
                    }
                }
                delete props;
            }
        }

        if(!status && foundXlfd)
        {
            createNameFromXlfdBmp();
            status=true;
        }
    }
 
    return status;;
}

static unsigned int readLsb32(CCompressedFile &f)
{
    unsigned char num[4];

    if(4==f.read(num, 4))
        return (num[0])+(num[1]<<8)+(num[2]<<16)+(num[3]<<24);
    else
        return 0;
}

static unsigned int read32(CCompressedFile &f, bool msb)
{
    if(msb)
    {
        unsigned char num[4];

        if(4==f.read(num, 4))
            return (num[0]<<24)+(num[1]<<16)+(num[2]<<8)+(num[3]);
        else
            return 0;
    }
    else
        return readLsb32(f);
}

bool CFontEngine::openFontPcf(const QString &file, unsigned short mask)
{
    bool status=false;

    CCompressedFile pcf(file.local8Bit());

    if(pcf)
    {
        const unsigned int contPcfVersion=(('p'<<24)|('c'<<16)|('f'<<8)|1);

        bool foundXlfd=false;

        if(contPcfVersion==readLsb32(pcf))
        {
            const unsigned int constPropertiesType=1;

            unsigned int numTables=readLsb32(pcf),
                         table,
                         type,
                         format,
                         size,
                         offset;

            for(table=0; table<numTables && !pcf.eof() && !status; ++table)
            {
                type=readLsb32(pcf);
                format=readLsb32(pcf);
                size=readLsb32(pcf);
                offset=readLsb32(pcf);
                if(constPropertiesType==type)
                {
                    if(pcf.seek(offset, SEEK_SET)!=-1)
                    {
                        const unsigned int constFormatMask=0xffffff00;

                        format=readLsb32(pcf);
                        if(0==(format&constFormatMask))
                        {
                            const unsigned int constByteMask=0x4;

                            bool         msb=format&constByteMask;
                            unsigned int numProps=read32(pcf, msb);

                            if(numProps>0 && numProps<constBitmapMaxProps)
                            {
                                unsigned int strSize,
                                             skip;

                                struct TProp
                                {
                                    unsigned int name,
                                                 value;
                                    bool         isString;
                                } *props=new struct TProp [numProps];

                                if(props)
                                {
                                    char           tmp;
                                    unsigned short prop;

                                    for(prop=0; prop<numProps; ++prop)
                                    {
                                        props[prop].name=read32(pcf, msb);
                                        pcf.read(&tmp, 1);
                                        props[prop].isString=tmp ? true : false;
                                        props[prop].value=read32(pcf, msb);
                                    }

                                    skip=4-((numProps*9)%4);
                                    if(skip!=4)
                                        pcf.seek(skip, SEEK_CUR);

                                    strSize=read32(pcf, msb);

                                    if(strSize>0)
                                    {
                                        char *str=new char [strSize];
                                        if(pcf.read(str, strSize)==(int)strSize)
                                        {
                                            // Finally we have the data............
                                            const int constMaxStrLen=1024;

                                            char    tmp[constMaxStrLen];
                                            bool    foundFamily=false,
                                                    foundSlant=false,
                                                    foundWeight=false,
                                                    foundRes=false,
                                                    foundPt=false,
                                                    foundEncoding=false,
                                                    foundCsReg=false,
                                                    foundCsEnc=false;
                                            QString csReg,
                                                    csEnc;
                                            int     res=0,
                                                    pointSize=0;

                                            for(prop=0; prop<numProps && !status; ++prop)
                                            {
                                                if(mask!=XLFD)
                                                {
                                                    if(!foundFamily && (CMisc::stricmp(&str[props[prop].name], "FAMILY_NAME")==0 ||
                                                                        CMisc::stricmp(&str[props[prop].name], "FAMILY")==0) )
                                                    {
                                                        if(props[prop].isString)
                                                        {
                                                            foundFamily=true;
                                                            strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                            tmp[constMaxStrLen-1]='\0';
                                                            itsFamily=tmp;
                                                        }
                                                    }
                                                    else if(!foundWeight && CMisc::stricmp(&str[props[prop].name], "WEIGHT_NAME")==0)
                                                    {
                                                        if(props[prop].isString)
                                                        {
                                                            foundWeight=true;
                                                            strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                            tmp[constMaxStrLen-1]='\0';
                                                            itsWeight=strToWeight(tmp);
                                                        }
                                                    }
                                                    else if(!foundSlant && CMisc::stricmp(&str[props[prop].name], "SLANT")==0)
                                                    {
                                                        if(props[prop].isString)
                                                        {
                                                            foundSlant=true;
                                                            itsItalic=charToItalic(str[props[prop].value]);
                                                        }
                                                    }
                                                    else if(!foundRes && CMisc::stricmp(&str[props[prop].name], "RESOLUTION_X")==0)
                                                    {
                                                        if(!props[prop].isString)
                                                        {
                                                            foundRes=true;
                                                            res=props[prop].value;
                                                        }
                                                    }
                                                    else if(!foundPt && CMisc::stricmp(&str[props[prop].name], "POINT_SIZE")==0)
                                                    {
                                                        if(!props[prop].isString)
                                                        {
                                                            foundPt=true;
                                                            pointSize=props[prop].value;
                                                        }
                                                    }
                                                    else if(!foundCsReg && CMisc::stricmp(&str[props[prop].name], "CHARSET_REGISTRY")==0)
                                                    {
                                                        if(props[prop].isString && strlen(&str[props[prop].value]))
                                                        {
                                                            foundCsReg=true;
                                                            strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                            tmp[constMaxStrLen-1]='\0';
                                                            csReg=tmp;
                                                        }
                                                    }
                                                    else if(!foundCsEnc && CMisc::stricmp(&str[props[prop].name], "CHARSET_ENCODING")==0)
                                                    {
                                                        if(props[prop].isString && strlen(&str[props[prop].value]))
                                                        {
                                                            foundCsEnc=true;
                                                            strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                            tmp[constMaxStrLen-1]='\0';
                                                            csEnc=tmp;
                                                        }
                                                    }
                                                }

                                                if(!foundXlfd && CMisc::stricmp(&str[props[prop].name], "FONT")==0)
                                                {
                                                    if(props[prop].isString && strlen(&str[props[prop].value]))
                                                    {
                                                        foundXlfd=true;
                                                        strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                        tmp[constMaxStrLen-1]='\0';
                                                        itsXlfd=tmp;
                                                    }
                                                }
                                                else if(!foundCsEnc && !foundCsReg && CMisc::stricmp(&str[props[prop].name], "FONT")==0)
                                                        foundCsEnc=foundCsReg=foundEncoding=getFileEncodingBmp(&str[props[prop].value]);

                                                if(!foundEncoding && foundCsReg && foundCsEnc)
                                                {
                                                    itsEncoding=csReg+"-"+csEnc;
                                                    foundEncoding=true;
                                                }

                                                if(mask==XLFD && foundXlfd)
                                                    status=true;
                                                else
                                                    if(mask!=XLFD &&
                                                         (foundFamily && foundWeight && foundRes && foundSlant && foundPt && foundEncoding &&
                                                           (!(mask&XLFD) || (mask&XLFD && foundXlfd))))
                                                    {
                                                        createNameBmp(pointSize, res, itsEncoding);
                                                        status=true;
                                                    }
                                            }
                                        }
                                        delete str;
                                    }
                                    delete props;
                                }
                            }
                        }
                    }
                    break;   // Forget the other tables...
                }
            }
        }

        if(!status && foundXlfd)
        {
            createNameFromXlfdBmp();
            status=true;
        }
    }
    return status;
}

