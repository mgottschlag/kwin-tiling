#ifndef __FONT_ENGINE_H__
#define __FONT_ENGINE_H__

////////////////////////////////////////////////////////////////////////////////
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fontconfig/fontconfig.h>

#if (FC_VERSION>=20402)
#define HAVE_FcFreeTypeQueryFace
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include <qstring.h>
#include <qstringlist.h>
#include <strigi/inputstream.h>

namespace KFI
{

class CFontEngine
{
    public:

    enum EType
    {
        TYPE_OTF,
        TYPE_TTF,
        TYPE_TTC,
        TYPE_TYPE1,
        TYPE_PCF,
        TYPE_BDF,
        TYPE_AFM,

        TYPE_UNKNOWN
    };

    private:

    struct TFtData
    {
        TFtData();
        ~TFtData();

        FT_Library      library;
        FT_Face         face;
        bool            open;
    };

    public:

    CFontEngine()                     { }
    ~CFontEngine()                    { closeFont(); }

    static int     strToWeight(const QString &str);
    static int     strToWidth(const QString &str);

    //
    // General functions - these should be used instead of specfic ones below...
    //
    static EType    getType(const char *fileName, jstreams::InputStream *in);
    bool            openFont(EType type, jstreams::InputStream *in, const char *fileName, int face=0);
    void            closeFont();

    //
    // These are only for non-bitmap fonts...
    const QString & getFamilyName()   { return itsFamily; }
    int             getWeight()       { return itsWeight; }
    int             getWidth()        { return itsWidth; }
    int             getItalic()       { return itsItalic; }
    int             getSpacing()      { return itsSpacing; }
    const QString & getFoundry()      { return itsFoundry; }
    const QString & getVersion()      { return itsVersion; }
    int             getNumFaces()     { return itsFt.open ? itsFt.face->num_faces : 1; }

    private:

    bool            openFontFt(jstreams::InputStream *in, const char *fileName, int face);
    bool            openFontAfm(jstreams::InputStream *in);
    void            parseXlfdBmp(const QString &lfd);
    bool            openFontBdf(jstreams::InputStream *in);
    bool            openFontPcf(jstreams::InputStream *in);

    private:

    int        itsWeight,
               itsWidth,
               itsItalic,
               itsSpacing;
    QString    itsFamily,
               itsFoundry,
               itsVersion;
    TFtData    itsFt;
};

}

#endif
