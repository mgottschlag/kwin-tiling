///////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontEngine
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

//
// This class contains code inspired/copied/nicked from mkfontscale. Specifically
// the getName(), lookupName(), and the getFoundry() routines...
//
// mkfontscale's (C) notice is:
/*
  Copyright (c) 2002 by Juliusz Chroboczek

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
    
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "FontEngine.h"
#include "Misc.h"
#include "Fc.h"
#include <kglobal.h>
#include <kascii.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ft2build.h>
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPE1_TABLES_H

static const char *constOblique = "Oblique";
static const char *constSlanted = "Slanted";

namespace KFI
{

static bool readChar(jstreams::InputStream *in, char *val)
{
    const char *ch;

    if(1==in->read(ch, 1, 1))
    {
        *val=*ch;
        return true;
    }

    return false;
}

static const char * readString(jstreams::InputStream *in, char *buf, int len)
{
    char *b=buf;

    if (NULL==buf || len <= 0)
        return NULL;

    while(--len > 0 && readChar(in, buf) && *buf++ != '\n')
        ;
    *buf = '\0';

    return b == buf && len > 0 ? NULL : b;
}

static unsigned long ftStreamRead(FT_Stream stream, unsigned long offset, unsigned char *buffer, unsigned long count)
{
    jstreams::InputStream *in((jstreams::InputStream *)stream->descriptor.pointer);
    int64_t               pos(in->getPosition()),
                          bytesRead(0);

    if(offset<pos)
    {
        in->reset(0);
        in->skip(offset);
    }
    else if(offset>pos)
        in->skip(offset-pos);

    if (count > 0)
    {
        const char *h;

        bytesRead=in->read(h, (int32_t)count, (int32_t)count);

        if(bytesRead>0)
            memcpy(buffer, h, bytesRead);
    }

    return bytesRead;
}

static void ftStreamClose(FT_Stream stream)
{
    jstreams::InputStream *in = (jstreams::InputStream *)stream->descriptor.pointer;

    in->reset(0);
}

static FT_Error openFtFace(FT_Library library, jstreams::InputStream *in, FT_Long index, FT_Face *face)
{
    FT_Open_Args args;
    FT_Stream    stream;
    FT_Error     error;

    if(NULL==(stream=(FT_Stream)calloc(1, sizeof(*stream))))
        return FT_Err_Out_Of_Memory;

    stream->descriptor.pointer = in;
    stream->pathname.pointer   = NULL;
    stream->size               = in->getSize();
    stream->pos                = 0;

    stream->read  = ftStreamRead;
    stream->close = ftStreamClose;
    args.flags  = FT_OPEN_STREAM;
    args.stream = stream;

    error = FT_Open_Face(library, &args, index, face);

    if (FT_Err_Ok!=error)
    {
        stream->close(stream);
        free(stream);
    }
    else
        (*face)->face_flags &= ~FT_FACE_FLAG_EXTERNAL_STREAM;

    return error;
}


CFontEngine::EType CFontEngine::getType(const char *fileName, jstreams::InputStream *in)
{
    // TTF  : 00 01 00 00 00                         5
    //      : FFIL                                   4
    //      : <65>  FFIL                            69
    // TTC  : ttcf                                   4
    // OTF  : OTTO                                   4
    // Type1: %!PS-AdobeFont-1.                     17
    //      : ?? ?? ?? ?? ?? ?? %!PS-AdobeFont-1.   23
    //      : %!FontType1-1.                        14
    //      : ?? ?? ?? ?? ?? ?? %!FontType1-1.      20
    //      : LWFN                                   4
    //      : <65> LWFN                             69
    // AFM  : StartFontMetrics                      16
    // BDF  : STARTFONT 20                          10
    // PCF  : 01 fcp                                 4
    static const int constHeaderLen=69;
    const char * h;

    if(in->read(h, constHeaderLen, constHeaderLen)>=constHeaderLen)
    {
        const unsigned char *hdr=(const unsigned char *)h;

        if( (0x00==hdr[0] && 0x01==hdr[1] && 0x00==hdr[2] && 0x00==hdr[3] && 0x00==hdr[4]) ||
            ('F'==hdr[0] && 'F'==hdr[1] && 'I'==hdr[2] && 'L'==hdr[3]) ||
            ('F'==hdr[65] && 'F'==hdr[66] && 'I'==hdr[67] && 'L'==hdr[68]))
            return TYPE_TTF;

        if('t'==hdr[0] && 't'==hdr[1] && 'c'==hdr[2] && 'f'==hdr[3])
            return TYPE_TTC;

        if('O'==hdr[0] && 'T'==hdr[1] && 'T'==hdr[2] && 'O'==hdr[3])
            return TYPE_OTF;

        if(0x01==hdr[0] && 'f'==hdr[1] && 'c'==hdr[2] && 'p'==hdr[3])
            return TYPE_PCF;

        if(0==memcmp(hdr, "STARTFONT", 9) && 0x20==hdr[9])
            return TYPE_BDF;

        if(0==memcmp(hdr, "%!PS-AdobeFont-1.", 17) ||
           0==memcmp(&hdr[6], "%!PS-AdobeFont-1.", 17) ||
           0==memcmp(hdr, "%!FontType1-1.", 14) ||
           0==memcmp(&hdr[6], "%!FontType1-1.", 14) ||
           ('L'==hdr[0] && 'W'==hdr[1] && 'F'==hdr[2] && 'N'==hdr[3]) ||
           ('L'==hdr[65] && 'W'==hdr[66] && 'F'==hdr[67] && 'N'==hdr[68]))
            return TYPE_TYPE1;

        if(0==memcmp(hdr, "StartFontMetrics", 16))
            return TYPE_AFM;
    }

    // Right mime 'magic' failed, try by extension...
    if(Misc::checkExt(fileName, "ttf"))
        return TYPE_TTF;

    if(Misc::checkExt(fileName, "ttc"))
        return TYPE_TTC;

    if(Misc::checkExt(fileName, "otf"))
        return TYPE_OTF;

    if(Misc::checkExt(fileName, "pfa") || Misc::checkExt(fileName, "pfb"))
        return TYPE_TYPE1;

    if(Misc::checkExt(fileName, "pcf") || Misc::checkExt(fileName, "pcf.gz"))
        return TYPE_PCF;

    if(Misc::checkExt(fileName, "bdf") || Misc::checkExt(fileName, "bdf.gz"))
        return TYPE_BDF;

    if(Misc::checkExt(fileName, "afm"))
        return TYPE_AFM;

    return TYPE_UNKNOWN;
}

bool CFontEngine::openFont(CFontEngine::EType type, jstreams::InputStream *in, const char *fileName, int face)
{
    bool ok=false;

    closeFont();
    in->reset(0);

    itsWeight=FC_WEIGHT_MEDIUM;
    itsWidth=FC_WIDTH_NORMAL;
    itsSpacing=FC_PROPORTIONAL;
    itsItalic=FC_SLANT_ROMAN;
    itsFamily=itsFoundry=itsVersion=QString();

    switch(type)
    {
        default:
            ok=openFontFt(in, fileName, face);
            break;
        case TYPE_PCF:
            ok=openFontPcf(in);
            break;
        case TYPE_BDF:
            ok=openFontBdf(in);
            break;
        case TYPE_AFM:
            ok=openFontAfm(in);
    }

    return ok;
}

void CFontEngine::closeFont()
{
#ifndef HAVE_FcFreeTypeQueryFace
    if(itsFt.open)
    {
        FT_Done_Face(itsFt.face);
        itsFt.open=false;
    }
#endif
}

int CFontEngine::strToWeight(const QString &str)
{
    if(str.isEmpty())
        return FC_WEIGHT_MEDIUM;
    else if(str.contains("Bold", Qt::CaseInsensitive))
        return FC_WEIGHT_BOLD;
    else if(str.contains("Heavy", Qt::CaseInsensitive))
        return FC_WEIGHT_HEAVY;
    else if(str.contains("Black", Qt::CaseInsensitive))
        return FC_WEIGHT_BLACK;
    else if(str.contains("ExtraBold", Qt::CaseInsensitive))
        return FC_WEIGHT_EXTRABOLD;
    else if(str.contains("UltraBold", Qt::CaseInsensitive))
        return FC_WEIGHT_ULTRABOLD;
    else if(str.contains("ExtraLight", Qt::CaseInsensitive))
        return FC_WEIGHT_EXTRALIGHT;
    else if(str.contains("UltraLight", Qt::CaseInsensitive))
        return FC_WEIGHT_ULTRALIGHT;
    else if(str.contains("Light", Qt::CaseInsensitive))
        return FC_WEIGHT_LIGHT;
    else if(str.contains("Medium", Qt::CaseInsensitive) ||
                    str.contains("Normal", Qt::CaseInsensitive) ||
                    str.contains("Roman", Qt::CaseInsensitive))
        return FC_WEIGHT_MEDIUM;
    else if(str.contains("Regular", Qt::CaseInsensitive))
        return FC_WEIGHT_REGULAR;
    else if(str.contains("SemiBold", Qt::CaseInsensitive))
        return FC_WEIGHT_SEMIBOLD;
    else if(str.contains("DemiBold", Qt::CaseInsensitive))
        return FC_WEIGHT_DEMIBOLD;
    else if(str.contains("Thin", Qt::CaseInsensitive))
        return FC_WEIGHT_THIN;
    else if(str.contains("Book", Qt::CaseInsensitive))
        return FC_WEIGHT_NORMAL;
    else if(str.contains("Demi", Qt::CaseInsensitive))
        return FC_WEIGHT_NORMAL;
    else
        return FC_WEIGHT_MEDIUM;
}

int CFontEngine::strToWidth(const QString &str)
{
    if(str.isEmpty())
        return KFI_FC_WIDTH_NORMAL;
    else if(str.contains("UltraCondensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_ULTRACONDENSED;
    else if(str.contains("ExtraCondensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_EXTRACONDENSED;
    else if(str.contains("SemiCondensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_SEMICONDENSED;
    else if(str.contains("Condensed", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_CONDENSED;
    else if(str.contains("SemiExpanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_SEMIEXPANDED;
    else if(str.contains("UltraExpanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_ULTRAEXPANDED;
    else if(str.contains("ExtraExpanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_EXTRAEXPANDED;
    else if(str.contains("Expanded", Qt::CaseInsensitive))
        return KFI_FC_WIDTH_EXPANDED;
    else
        return KFI_FC_WIDTH_NORMAL;
}

static int checkItalic(int it, const QString &full)
{
    return (FC_SLANT_ITALIC==it && (-1!=full.find(constOblique) || -1!=full.find(constSlanted)))
           ? FC_SLANT_OBLIQUE
           : it;
}

static const char * getFoundry(const char *notice)
{
    struct Map
    {
        const char *noticeStr,
                   *foundry;
    };

    static const Map map[]=   // These are (mainly) taken from type1inst
    {
        { "Bigelow",                            "B&H"},
        { "Adobe",                              "Adobe"},
        { "Bitstream",                          "Bitstream"},
        { "Monotype",                           "Monotype"},
        { "Linotype",                           "Linotype"},
        { "LINOTYPE-HELL",                      "Linotype"},
        { "IBM",                                "IBM"},
        { "URW",                                "URW"},
        { "International Typeface Corporation", "ITC"},
        { "Tiro Typeworks",                     "Tiro"},
        { "XFree86",                            "XFree86"},
        { "Microsoft",                          "Microsoft"},
        { "Omega",                              "Omega"},
        { "Font21",                             "Hwan"},
        { "HanYang System",                     "Hanyang"},
        { "Richard Mitchell",                   "Mitchell" },
        { "Doug Miles",                         "Miles" },
        { "Hank Gillette",                      "Gillette" },
        { "Three Islands Press",                "3ip" },
        { "MacroMind",                          "Macromind" },
        { "MWSoft",                             "MWSoft" },
        { "Digiteyes Multimedia",               "DigitEyes" },
        { "ZSoft",                              "ZSoft" },
        { "Title Wave",                         "Titlewave" },
        { "Southern Software",                  "Southern" },
        { "Reasonable Solutions",               "Reasonable" },
        { "David Rakowski",                     "Rakowski" },
        { "D. Rakowski",                        "Rakowski" },
        { "S. G. Moye",                         "Moye" },
        { "S.G. Moye",                          "Moye" },
        { "Andrew s. Meit",                     "Meit" },
        { "A.S.Meit",                           "Meit" },
        { "Hershey",                            "Hershey" },
        { "FontBank",                           "FontBank" },
        { "A. Carr",                            "Carr" },
        { "Brendel Informatik",                 "Brendel" },
        { "Jonathan Brecher",                   "Brecher" },
        { "SoftMaker",                          "Softmaker" },
        { "LETRASET",                           "Letraset" },
        { "Corel Corp",                         "Corel"},
        { "PUBLISHERS PARADISE",                "Paradise" },
        { "Publishers Paradise",                "Paradise" },
        { "Allied Corporation",                 "Allied" },
        { NULL,                                 NULL }
    };

    const Map *entry;

    if(notice)
        for(entry=map; NULL!=entry->foundry; entry++)
            if(strstr(notice, entry->noticeStr)!=NULL)
                return entry->foundry;

    return NULL;
}

#ifndef HAVE_FcFreeTypeQueryFace
static bool lookupName(FT_Face face, int nid, int pid, int eid, FT_SfntName *nameReturn)
{
    int n = FT_Get_Sfnt_Name_Count(face);

    if(n>0)
    {
        int         i;
        FT_SfntName name;

        for(i=0; i<n; i++)
            if(0==FT_Get_Sfnt_Name(face, i, &name) && name.name_id == nid && name.platform_id == pid &&
               (eid < 0 || name.encoding_id == eid))
            {
                switch(name.platform_id)
                {
                    case TT_PLATFORM_APPLE_UNICODE:
                    case TT_PLATFORM_MACINTOSH:
                        if(name.language_id != TT_MAC_LANGID_ENGLISH)
                            continue;
                        break;
                    case TT_PLATFORM_MICROSOFT:
                        if(name.language_id != TT_MS_LANGID_ENGLISH_UNITED_STATES &&
                           name.language_id != TT_MS_LANGID_ENGLISH_UNITED_KINGDOM)
                            continue;
                        break;
                    default:
                        continue;
                }

                if(name.string_len > 0)
                {
                    *nameReturn = name;
                    return true;
                }
            }
    }

    return false;
}

static QByteArray getName(FT_Face face, int nid)
{
    FT_SfntName name;
    QByteArray  str;

    if(lookupName(face, nid, TT_PLATFORM_MICROSOFT, TT_MS_ID_UNICODE_CS, &name) ||
       lookupName(face, nid, TT_PLATFORM_APPLE_UNICODE, -1, &name))
        for(unsigned int i=0; i < name.string_len / 2; i++)
            str+=0 == name.string[2*i] ? name.string[(2*i)+1] : '_';
    else if(lookupName(face, nid, TT_PLATFORM_MACINTOSH, TT_MAC_ID_ROMAN, &name)) // Pretend that Apple Roman is ISO 8859-1
        for(unsigned int i=0; i < name.string_len; i++)
            str+=name.string[i];

    return str;
}

static const char * getFoundry(const FT_Face face, TT_OS2 *os2)
{
    struct Map
    {
        const char *vendorId,
                   *foundry;
    };

    static const int constVendLen=4;

    // These are taken from ttmkfdir
    // Removed any commas - StarOffice doesn't like these...
    // Shortened quite a few entires to help with StarOffice
    static const Map map[]=
    {
        { "ADBE", "Adobe"},
        { "AGFA", "Agfa"},
        { "ALTS", "Altsys"},
        { "APPL", "Apple"},
        { "ARPH", "Arphic"},
        { "ATEC", "Alltype"},
        { "B&H",  "B&H"},
        { "BITS", "Bitstream"},
        { "CANO", "Cannon"},
        { "DYNA", "DynaLab"},
        { "EPSN", "Epson"},
        { "FJ",   "Fujitsu"},
        { "IBM",  "IBM"},
        { "ITC",  "ITC"},
        { "IMPR", "Impress"},
        { "LARA", "Larabie"},
        { "LEAF", "Interleaf"},
        { "LETR", "letraset"},
        { "LINO", "Linotype"},
        { "MACR", "Macromedia"},
        { "MONO", "Monotype"},
        { "MS",   "Microsoft"},
        { "MT",   "Monotype"},
        { "NEC",  "NEC"},
        { "PARA", "ParaType"},
        { "QMSI", "QMS"},
        { "RICO", "Ricoh"},
        { "URW",  "URW"},
        { "Y&Y" , "Z&Y"},
        { "2REB", "2Rebels"}, 
        { "3IP" , "3ip"},
        { "ABC" , "Altek Inst"},
        { "ACUT", "AcuteType"},
        { "AOP" , "Art Of Penguin"},
        { "AZLS", "Azalea"},
        { "BARS", "CIA UK"},
        { "BERT", "Berthold"},
        { "BITM", "Bitmap Soft"},
        { "BLAH", "MisterBlas"},
        { "BOYB", "I Frances"},
        { "BRTC", "Bear Rock"},
        { "BWFW", "B/W Fontworks"},
        { "C&C",  "Carter&Cone"},
        { "CAK" , "Pluginfonts"},
        { "CASL", "H W Caslon"},
        { "COOL", "CoolFonts"},
        { "CTDL", "ChinaType"},
        { "DAMA", "DM Ltd"},
        { "DS"  , "Dainippon"},
        { "DSCI", "Design Science"},
        { "DTC",  "Digital Typeface"},
        { "DTPS", "DTP Software"},
        { "DUXB", "Duxbury"},
        { "ECF" , "Emerald City"},
        { "EDGE", "Rivers Edge"},
        { "EFF" , "Electronic"},
        { "EFNT", "E Fonts"},
        { "ELSE", "Elseware"},
        { "ERAM", "Eraman"},
        { "ESIG", "E Signature"},
        { "FBI",  "Font Bureau"},
        { "FCAB", "Font Cabinet"},
        { "FONT", "Font Source"},
        { "FS"  , "Formula"},
        { "FSE" , "Font Source Europe"},
        { "FSI" , "FSI GmbH"},
        { "FTFT", "FontFont"},
        { "FWRE", "Fontware"},
        { "GALA", "Galapagos"},
        { "GD"  , "GDFonts"},
        { "GLYF", "Glyph Systems"},
        { "GPI",  "Gamma Productions"},
        { "HY",   "HanYang System"},
        { "HILL", "Hill Systems"},
        { "HOUS", "House Industries"},
        { "HP",   "HP"},
        { "IDEE", "IDEE"},
        { "IDF",  "Digital Fonts"},
        { "ILP",  "Indigenous Lang"},
        { "ITF" , "Int Type Founders"},
        { "KATF", "Kingsley"},
        { "LANS", "Lanston"},
        { "LGX" , "Logix Research"},
        { "LING", "Linguists"},
        { "LP",   "LetterPerfect"},
        { "LTRX", "Lighttracks"},
        { "MC"  , "Cerajewski"},
        { "MILL", "Millan"},
        { "MJ"  , "Majus"},
        { "MLGC", "Micrologic"},
        { "MSCR", "Majus"},
        { "MTY" , "Motoya"},
        { "MUTF", "CACHE"},
        { "NB"  , "No Bodoni"},
        { "NDTC", "Neufville Digital"},
        { "NIS" , "NIS"},
        { "ORBI", "Orbit"},
        { "P22" , "P22"},
        { "PDWX", "Parsons Design"},
        { "PF"  , "Phils Fonts"},
        { "PRFS", "Production"},
        { "RKFN", "R K Fonts"},
        { "ROBO", "Buro Petr"},
        { "RUDY", "Rudyn Fluffy"},
        { "SAX" , "SAX gmbh"},
        { "SEAN", "The FontSite"},
        { "SFI" , "Software Friends"},
        { "SFUN", "Soft Union"},
        { "SG"  , "Scooter Graphics"},
        { "SIG" , "Signature"},
        { "SKZ" , "Celtic Ladys"},
        { "SN"  , "SourceNet"},
        { "SOHO", "SoftHorizons"},
        { "SOS" , "Standing Ovations"},
        { "STF" , "Brian Sooy"},
        { "STON", "ZHUHAI"},
        { "SUNW", "Sunwalk"},
        { "SWFT", "Swfte"},
        { "SYN" , "SynFonts"},
        { "TDR" , "Tansin A Darcos"},
        { "TF"  , "Treacyfaces"},
        { "TILD", "SIA Tilde"},
        { "TPTC", "Test Pilot"},
        { "TR"  , "Type Revivals"},
        { "TS"  , "TamilSoft"},
        { "UA"  , "UnAuthorized Type"},
        { "VKP" , "Vijay K Patel"},
        { "VLKF", "Visualogik"},
        { "VOG" , "Martin Vogel"},
        { "ZEGR", "Zebra Font Facit"},
        { "ZETA", "Tangram Studio"},
        { "ZSFT", "ZSoft"},
        { NULL  ,  NULL}
    };

    static char vendor[constVendLen+1];

    vendor[0]='\0';

    if(NULL!=os2 && 0xFFFF!=os2->version)
    {
        const Map *entry;
        char      vend[constVendLen+1];

        strncpy(vendor, (const char *)(os2->achVendID), constVendLen);
        vendor[constVendLen]='\0';

        for(int i=0; i<constVendLen; ++i)
            vend[i]=toupper(vendor[i]);

        for(entry=map; NULL!=entry->vendorId; entry++)
        {
            unsigned int len=strlen(entry->vendorId);

            if(0==memcmp(entry->vendorId, vend, len))
            {
                bool ok=true;

                for(int i=len; i<constVendLen && ok; i++)
                    if(vend[i]!=' ' && entry->vendorId[i]!='\0')
                        ok=false;

                if(ok)
                    return entry->foundry;
            }
        }
    }

    const char *foundry=NULL;

    if(!foundry)
        foundry=getFoundry(getName(face, TT_NAME_ID_TRADEMARK));

    if(!foundry)
        foundry=getFoundry(getName(face, TT_NAME_ID_MANUFACTURER));

    if(!foundry && vendor[0] && !isspace(vendor[0]) && '-'!=vendor[0])  // Some fonts have a totally blank vendor field
    {
       int i;

        // Remove any dashes...
        for(int i=constVendLen-1; i>0; i--)
            if('-'==vendor[i])
                vendor[i]=' ';

        // Strip any trailing whitepace
        for(i=constVendLen-1; i>0; i--)
            if(isspace(vendor[i]))
               vendor[i]='\0';
            else
               break;

        foundry=vendor;
    }

    return foundry ;
}
#endif

bool CFontEngine::openFontFt(jstreams::InputStream *in, const char *fileName, int face)
{
    bool status=openFtFace(itsFt.library, in, face, &itsFt.face) ? false : true;

    if(status)
        itsFt.open=true;

    if(status)
    {
#ifdef HAVE_FcFreeTypeQueryFace
        FcPattern *pat=FcFreeTypeQueryFace(itsFt.face, (FcChar8*) fileName, face, NULL);

        itsWeight=FC_WEIGHT_REGULAR;
        itsWidth=KFI_FC_WIDTH_NORMAL;
        itsSpacing=FC_PROPORTIONAL;

        if(pat)
        {
            int version;

            itsFamily=FC::getFcString(pat, FC_FAMILY, face);
            FcPatternGetInteger(pat, FC_WEIGHT, face, &itsWeight);
#ifndef KFI_FC_NO_WIDTHS
            FcPatternGetInteger(pat, FC_WIDTH, face, &itsWidth);
#endif
            FcPatternGetInteger(pat, FC_SLANT, face, &itsItalic);
            FcPatternGetInteger(pat, FC_SPACING, face, &itsSpacing);
            itsFoundry=FC::getFcString(pat, FC_FOUNDRY, face);
            FcPatternGetInteger(pat, FC_FONTVERSION, face, &version);
            FcPatternDestroy(pat);

            // Try to make foundry similar to that of AFMs...
            if(itsFoundry== QString::fromLatin1("ibm"))
                itsFoundry= QString::fromLatin1("IBM");
            else if(itsFoundry== QString::fromLatin1("urw"))
                itsFoundry= QString::fromLatin1("URW");
            else if(itsFoundry== QString::fromLatin1("itc"))
                itsFoundry= QString::fromLatin1("ITC");
            else
            {
                QChar *ch(itsFoundry.data());
                int   len(itsFoundry.length());
                bool  isSpace(true);

                while(len--)
                {
                    if (isSpace)
                        *ch=ch->toUpper();

                    isSpace=ch->isSpace();
                    ++ch;
                }
            }
        }
        else
            status=false;

#else

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

        bool type1(false);

        PS_FontInfoRec t1info;
        QString        fullName;

        if(0==FT_Get_PS_Font_Info(itsFt.face, &t1info))
        {
            fullName=t1info.full_name;
            itsFamily=t1info.family_name;
            type1=true;
        }
        else
        {
            fullName=getName(itsFt.face, TT_NAME_ID_FULL_NAME);
            itsFamily=getName(itsFt.face, TT_NAME_ID_FONT_FAMILY);
        }

        if(itsFamily.isEmpty())
            if(fullName.isEmpty())
                itsFamily=fullName=FT_Get_Postscript_Name(itsFt.face);
            else
                itsFamily=fullName;
        else
            if(fullName.isEmpty())
                fullName=itsFamily;

        if(fullName.isEmpty())
            status=false;   // Hmm... couldn't find any of the names!

        if(status)
        {
            if(type1)
            {
                itsWeight=strToWeight(t1info.weight);
                itsItalic=t1info.italic_angle <= -4 || t1info.italic_angle >= 4 ? FC_SLANT_ITALIC : FC_SLANT_ROMAN;
                itsWidth=strToWidth(fullName);
                itsItalic=checkItalic(itsItalic, fullName);
                itsSpacing=t1info.is_fixed_pitch ? FC_MONO : FC_PROPORTIONAL;
                itsFoundry=KFI::getFoundry(t1info.notice);
                itsVersion=t1info.version;
            }
            else // TrueType...
            {
                #define WEIGHT_UNKNOWN 0xFFFF
                #define WIDTH_UNKNOWN  0xFFFF

                struct TFixed
                {
                    TFixed(unsigned long v) : upper(v>>16), lower(v&0xFFFF) {}

                    short upper,
                          lower;

                    float value() { return upper+(lower/65536.0); }
                };

                TT_Postscript *post=(TT_Postscript *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post);
                TT_OS2        *os2=(TT_OS2 *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2);
                TT_Header     *head=(TT_Header *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head);
                bool          gotItalic=false;

                itsWidth=WIDTH_UNKNOWN;
                itsWeight=WEIGHT_UNKNOWN;

                if(NULL!=os2 && 0xFFFF!=os2->version)
                {
                    if (os2->usWeightClass == 0)
                        ;
                    else if (os2->usWeightClass < 150)
                        itsWeight=FC_WEIGHT_THIN;
                    else if (os2->usWeightClass < 250)
                        itsWeight=FC_WEIGHT_EXTRALIGHT;
                    else if (os2->usWeightClass < 350)
                        itsWeight=FC_WEIGHT_LIGHT;
                    else if (os2->usWeightClass < 450)
                        itsWeight=FC_WEIGHT_REGULAR;
                    else if (os2->usWeightClass < 550)
                        itsWeight=FC_WEIGHT_MEDIUM;
                    else if (os2->usWeightClass < 650)
                        itsWeight=FC_WEIGHT_SEMIBOLD;
                    else if (os2->usWeightClass < 750)
                        itsWeight=FC_WEIGHT_BOLD;
                    else if (os2->usWeightClass < 850)
                        itsWeight=FC_WEIGHT_EXTRABOLD;
                    else if (os2->usWeightClass < 950)
                        itsWeight=FC_WEIGHT_BLACK;

                    switch(os2->usWidthClass)
                    {
                        case TTF_WIDTH_ULTRA_CONDENSED:
                            itsWidth=FC_WIDTH_ULTRACONDENSED;
                        case TTF_WIDTH_EXTRA_CONDENSED:
                            itsWidth=FC_WIDTH_EXTRACONDENSED;
                        case TTF_WIDTH_CONDENSED:
                            itsWidth=FC_WIDTH_CONDENSED;
                        case TTF_WIDTH_SEMI_CONDENSED:
                            itsWidth=FC_WIDTH_SEMICONDENSED;
                        case TTF_WIDTH_NORMAL:
                            itsWidth=FC_WIDTH_NORMAL;
                        case TTF_WIDTH_SEMI_EXPANDED:
                            itsWidth=FC_WIDTH_SEMIEXPANDED;
                        case TTF_WIDTH_EXPANDED:
                            itsWidth=FC_WIDTH_EXPANDED;
                        case TTF_WIDTH_EXTRA_EXPANDED:
                            itsWidth=FC_WIDTH_EXTRAEXPANDED;
                        case TTF_WIDTH_ULTRA_EXPANDED:
                            itsWidth=FC_WIDTH_ULTRAEXPANDED;
                    }

                    itsItalic=os2->fsSelection&(1 << 0) ? FC_SLANT_ITALIC : FC_SLANT_ROMAN;
                    gotItalic=true;
                }

                if(WEIGHT_UNKNOWN==itsWeight)
                    itsWeight=itsFt.face->style_flags&FT_STYLE_FLAG_BOLD
                                ? FC_WEIGHT_BOLD
                                : FC_WEIGHT_MEDIUM;

                if(WIDTH_UNKNOWN==itsWidth)
                    itsWidth=FC_WIDTH_NORMAL;

                if(!gotItalic && head!=NULL)
                {
                    gotItalic=true;
                    itsItalic=head->Mac_Style & 2 ? FC_SLANT_ITALIC : FC_SLANT_ROMAN;
                }

                if(head)
                    itsVersion.setNum(TFixed((unsigned long)head->Font_Revision).value());

                if(!gotItalic && NULL!=post)
                {
                    gotItalic=true;
                    itsItalic=0.0f==((TFixed)post->italicAngle).value() ? FC_SLANT_ROMAN : FC_SLANT_ITALIC;
                }

                itsItalic=checkItalic(itsItalic, fullName);

                if(NULL!=post &&  post->isFixedPitch)
                {
                    TT_HoriHeader *hhea=NULL;

                    if(NULL!=(hhea=(TT_HoriHeader *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_hhea)) &&
                    hhea->min_Left_Side_Bearing >= 0 && hhea->xMax_Extent <= hhea->advance_Width_Max)
                        itsSpacing=FC_CHARCELL;
                    else
                        itsSpacing=FC_MONO;
                }
                else
                    itsSpacing=FC_PROPORTIONAL;

                itsFoundry=KFI::getFoundry(itsFt.face, os2);
            }
        }
#endif
    }
    if(!status)
        closeFont();

    return status;
}

bool CFontEngine::openFontAfm(jstreams::InputStream *in)
{
    bool      inMetrics=false;
    const int constMaxLineLen=1024;
    char      buffer[constMaxLineLen];
    QString   full;

    while(NULL!=readString(in, buffer, constMaxLineLen))
    {
        QString line(QString::fromLatin1(buffer));
        line=line.simplified();

        if(inMetrics)
        {
            if(0==line.indexOf("FullName "))
            {
                full=line.mid(9);
                itsWidth=strToWidth(full);
            }
            else if(0==line.indexOf("FamilyName "))
                itsFamily=line.mid(11);
            else if(0==line.indexOf("Weight "))
                itsWeight=strToWeight(line.mid(7));
            else if(0==line.indexOf("ItalicAngle "))
                itsItalic=0.0f==line.mid(12).toFloat() ? FC_SLANT_ROMAN : FC_SLANT_ITALIC;
            else if(0==line.indexOf("IsFixedPitch "))
                itsSpacing= ( line.mid(13).contains("false", Qt::CaseInsensitive)
                            ? FC_PROPORTIONAL : FC_MONO );
            else if(0==line.indexOf("Notice "))
                itsFoundry=KFI::getFoundry(line.mid(7).toLatin1());
             else if(0==line.indexOf("Version "))
                 itsVersion=line.mid(8);
            else if(0==line.indexOf("StartCharMetrics"))
                break;
        }
        else
            if(0==line.indexOf("StartFontMetrics"))
                inMetrics=true;
    };

    if(itsFamily.isEmpty() && !full.isEmpty())
        itsFamily=full;

    checkItalic(itsItalic, full);

    return !full.isEmpty() && !itsFamily.isEmpty();
}

static const unsigned int constBitmapMaxProps=1024;

static int charToItalic(char c)
{
    switch(c)
    {
        case 'i':
        case 'I':
            return FC_SLANT_ITALIC;
        case 'o':
        case 'O':
            return FC_SLANT_OBLIQUE;
        case 'r':
        case 'R':
        default:
            return FC_SLANT_ROMAN;
    }
}

static int charToSpacing(char c)
{
    switch(c)
    {
        case 'm':
        case 'M':
            return FC_MONO;
        case 'c':
        case 'C':
            return FC_CHARCELL;
        default:
        case 'p':
        case 'P':
            return FC_PROPORTIONAL;
    }
}

void CFontEngine::parseXlfdBmp(const QString &xlfd)
{
    enum EXlfd
    {
        XLFD_FOUNDRY=0,
        XLFD_FAMILY,
        XLFD_WEIGHT,
        XLFD_SLANT,
        XLFD_WIDTH,
        XLFD_STYLE,
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
            oldPos=1;
    int     entry;

    // XLFD:
    //     -foundry-family-weight-slant-width-?-pixelSize-pointSize-resX-resY-spacing-avWidth-csReg-csEnc
    for(entry=XLFD_FOUNDRY; -1!=(pos=xlfd.find('-', pos+1)) && entry<XLFD_END; ++entry)
    {
        switch(entry)
        {
            default:
                break;
            case XLFD_FOUNDRY:
                itsFoundry=xlfd.mid(oldPos, pos-oldPos);
                break;
            case XLFD_FAMILY:
                itsFamily=xlfd.mid(oldPos, pos-oldPos);
                break;
            case XLFD_WEIGHT:
                itsWeight=strToWeight(xlfd.mid(oldPos, pos-oldPos).local8Bit());
                break;
            case XLFD_SLANT:
                if(pos>0)
                    itsItalic=charToItalic(xlfd[pos-1].latin1());
                break;
            case XLFD_WIDTH:
                itsWidth=strToWidth(xlfd.mid(oldPos, pos-oldPos));
                break;
            case XLFD_STYLE:
            case XLFD_PIXEL_SIZE:
            case XLFD_POINT_SIZE:
            case XLFD_RESX:
            case XLFD_RESY:
                break;
            case XLFD_SPACING:
                if(pos>0)
                    itsSpacing=charToSpacing(xlfd[pos-1].latin1());
                break;
            case XLFD_AV_WIDTH:
            case XLFD_ENCODING:
                break;
        }

        oldPos=pos+1;
    }
}

static const char * getTokenBdf(const char *str, const char *key, bool noquotes=false)
{
    const char   *s=NULL;
    unsigned int keyLen=strlen(key),
                 sLen=strlen(str);

    if(keyLen+1<sLen && NULL!=(s=strstr(str, key)) && (s==str || (!isalnum(s[-1]) && '_'!=s[-1])) &&
      (!noquotes || (noquotes && s[keyLen+1]=='-')))
    {
        const int   constMaxTokenSize=256;
        static char tokenBuffer[constMaxTokenSize];

        char        *end=NULL,
                    *token;

        strncpy(tokenBuffer, s, constMaxTokenSize);
        tokenBuffer[constMaxTokenSize-1]='\0';
        token=tokenBuffer;

        if(noquotes)
        {
            token+=strlen(key)+1;
            if(NULL!=(end=strchr(token, '\n')))
            {
                *end='\0';
                return token;
            }
        }
        else
            if(NULL!=(token=strchr(token, '\"')))
            {
                token++;
                if(NULL!=(end=strchr(token, '\"')))
                {
                    *end='\0';
                    return token;
                }
            }
    }

    return NULL;
}

bool CFontEngine::openFontBdf(jstreams::InputStream *in)
{
    bool      foundXlfd=false;
    const int constMaxLineLen=1024;
    char      buffer[constMaxLineLen];

    while(NULL!=readString(in, buffer, constMaxLineLen) && !foundXlfd)
    {
        const char *str;

        if(!foundXlfd && NULL!=(str=getTokenBdf(buffer, "FONT", true)))   // "FONT" does not have quotes!
        {
            if(strlen(str))
            {
                parseXlfdBmp(str);
                foundXlfd=true;
            }
            break;
        }
    }

    return foundXlfd;
}

static unsigned int readLsb32(jstreams::InputStream *in)
{
    const char *n;

    if(4==in->read(n, 4, 4))
    {
        const unsigned char *num=(const unsigned char *)n;
        return (num[0])+(num[1]<<8)+(num[2]<<16)+(num[3]<<24);
    }

    return 0;
}

static unsigned int read32(jstreams::InputStream *in, bool msb)
{
    if(msb)
    {
        const char *n;

        if(4==in->read(n, 4, 4))
        {
            const unsigned char *num=(const unsigned char *)n;
            return (num[0]<<24)+(num[1]<<16)+(num[2]<<8)+(num[3]);
        }
        else
            return 0;
    }

    return readLsb32(in);
}

bool CFontEngine::openFontPcf(jstreams::InputStream *in)
{
    bool               foundXlfd=false;
    const unsigned int contPcfVersion=(('p'<<24)|('c'<<16)|('f'<<8)|1);

    if(contPcfVersion==readLsb32(in))
    {
        const unsigned int constPropertiesType=1;

        unsigned int numTables=readLsb32(in),
                     table,
                     type,
                     format,
                     size,
                     offset;

        for(table=0; table<numTables && jstreams::Ok==in->getStatus() && !foundXlfd; ++table)
        {
            type=readLsb32(in);
            format=readLsb32(in);
            size=readLsb32(in);
            offset=readLsb32(in);
            if(constPropertiesType==type)
            {
                in->reset(0);
                in->skip(offset);
                if(jstreams::Ok==in->getStatus())
                {
                    const unsigned int constFormatMask=0xffffff00;

                    format=readLsb32(in);
                    if(0==(format&constFormatMask))
                    {
                        const unsigned int constByteMask=0x4;

                        bool         msb=format&constByteMask;
                        unsigned int numProps=read32(in, msb);

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
                                const char     *tmp;
                                unsigned short prop;

                                for(prop=0; prop<numProps; ++prop)
                                {
                                    props[prop].name=read32(in, msb);
                                    in->read(tmp, 1, 1);
                                    props[prop].isString=tmp ? true : false;
                                    props[prop].value=read32(in, msb);
                                }

                                skip=4-((numProps*9)%4);
                                if(skip!=4)
                                    in->skip(skip);

                                strSize=read32(in, msb);

                                if(strSize>0)
                                {
                                    const char *str;

                                    if(in->read(str, strSize, strSize)==(int)strSize)
                                    {
                                        // Finally we have the data............
                                        const int constMaxStrLen=1024;

                                        char tmp[constMaxStrLen];

                                        for(prop=0; prop<numProps && !foundXlfd; ++prop)
                                            if(kasciistricmp(&str[props[prop].name], "FONT")==0)
                                            {
                                                if(props[prop].isString && strlen(&str[props[prop].value]))
                                                {
                                                    foundXlfd=true;
                                                    strncpy(tmp, &str[props[prop].value], constMaxStrLen);
                                                    tmp[constMaxStrLen-1]='\0';
                                                    parseXlfdBmp(tmp);
                                                }
                                                break;
                                            }
                                    }
                                }
                                delete [] props;
                            }
                        }
                    }
                }
                break;   // Forget the other tables...
            }
        }
    }


    return foundXlfd;
}

CFontEngine::TFtData::TFtData()
                    : open(false)
{
    if(FT_Init_FreeType(&library))
    {
        std::cerr << "ERROR: FreeType2 failed to initialise\n";
        exit(0);
    }
}

CFontEngine::TFtData::~TFtData()
{
    FT_Done_FreeType(library);
}

}
