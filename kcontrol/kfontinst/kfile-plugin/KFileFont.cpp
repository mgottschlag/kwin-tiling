////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFileFont
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 20/03/2003
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
// (C) Craig Drummond, 2003
////////////////////////////////////////////////////////////////////////////////

#include "KFileFont.h"
#include "FontEngine.h"
#include "Misc.h"
#include "CompressedFile.h"
#include "Global.h"
#include "Config.h"
#include "KioFonts.h"
#include <kgenericfactory.h>
#include <qfile.h>
#include <kio/netaccess.h>

typedef KGenericFactory<KFileFontPlugin> KFileFontPluginFactory;
K_EXPORT_COMPONENT_FACTORY(kfile_font, KFileFontPluginFactory("kfile_font"))

KFileFontPlugin::KFileFontPlugin(QObject *parent, const char *name, const QStringList& args)
               : KFilePlugin(parent, name, args)
{
    addMimeType("application/x-font-ttf"),
    addMimeType("application/x-font-type1");
    addMimeType("application/x-font-speedo");
    addMimeType("application/x-font-bdf");
    addMimeType("application/x-font-pcf");
    addMimeType("application/x-font-snf");
    addMimeType("application/x-font-otf");
    addMimeType("application/x-font-ttc");
}

KFileFontPlugin::~KFileFontPlugin()
{
    CGlobal::destroy();
}

void KFileFontPlugin::addMimeType(const char *mime)
{
    KFileMimeTypeInfo            *info=addMimeTypeInfo(mime);
    KFileMimeTypeInfo::GroupInfo *group=addGroupInfo(info, "General", i18n("General"));

    addItemInfo(group, "Full", i18n("Full Name"), QVariant::String);
    addItemInfo(group, "Family", i18n("Family"), QVariant::String);
    addItemInfo(group, "PostScript", i18n("PostScript Name"), QVariant::String);
    addItemInfo(group, "Foundry", i18n("Foundry"), QVariant::String);
    addItemInfo(group, "Weight", i18n("Weight"),  QVariant::String);
    addItemInfo(group, "Width", i18n("Width"), QVariant::String);
    addItemInfo(group, "Spacing", i18n("Spacing"),  QVariant::String);
    addItemInfo(group, "Slant", i18n("Slant"), QVariant::String);
}

static QString toStr(CFontEngine::ESpacing v)
{
    switch(v)
    {
        case CFontEngine::SPACING_MONOSPACED:
            return i18n("Monospaced");
        case CFontEngine::SPACING_PROPORTIONAL:
            return i18n("Proportional");
        case CFontEngine::SPACING_CHARCELL:
            return i18n("Charcell");
        default:
            return i18n("<ERROR>");
    }
}

static QString toStr(CFontEngine::EItalic v)
{
    switch(v)
    {
        case CFontEngine::ITALIC_NONE:
            return i18n("Roman");
        case CFontEngine::ITALIC_ITALIC:
            return i18n("Italic");
        case CFontEngine::ITALIC_OBLIQUE:
            return i18n("Oblique");
        default:
            return i18n("<ERROR>");
    }
}

QString toStr(enum CFontEngine::EWeight v)
{
    switch(v)
    {
        case CFontEngine::WEIGHT_THIN:
            return i18n("Thin");
        case CFontEngine::WEIGHT_ULTRA_LIGHT:
            return i18n("Ultra Light");
        case CFontEngine::WEIGHT_EXTRA_LIGHT:
            return i18n("Extra Light");
        case CFontEngine::WEIGHT_DEMI:
            return i18n("Demi");
        case CFontEngine::WEIGHT_LIGHT:
            return i18n("Light");
        case CFontEngine::WEIGHT_BOOK:
            return i18n("Book");
        case CFontEngine::WEIGHT_MEDIUM:
            return i18n("Medium");
        case CFontEngine::WEIGHT_REGULAR:
            return i18n("Regular");
        case CFontEngine::WEIGHT_SEMI_BOLD:
            return i18n("Semi Bold");
        case CFontEngine::WEIGHT_DEMI_BOLD:
            return i18n("Demi Bold");
        case CFontEngine::WEIGHT_BOLD:
            return i18n("Bold");
        case CFontEngine::WEIGHT_EXTRA_BOLD:
            return i18n("Extra Bold");
        case CFontEngine::WEIGHT_ULTRA_BOLD:
            return i18n("Ultra Bold");
        case CFontEngine::WEIGHT_HEAVY:
            return i18n("Heavy");
        case CFontEngine::WEIGHT_BLACK:
            return i18n("Black");
        case CFontEngine::WEIGHT_UNKNOWN:
        default:
            return i18n("Medium");
    }
}

QString toStr(CFontEngine::EWidth v)
{   
    switch(v)
    {
        case CFontEngine::WIDTH_ULTRA_CONDENSED:
            return i18n("Ultra Condensed");
        case CFontEngine::WIDTH_EXTRA_CONDENSED:
            return i18n("Extra Condensed");
        case CFontEngine::WIDTH_CONDENSED:
            return i18n("Condensed");
        case CFontEngine::WIDTH_SEMI_CONDENSED:
            return i18n("Semi Condensed");
        case CFontEngine::WIDTH_SEMI_EXPANDED:
            return i18n("Semi Expanded");
        case CFontEngine::WIDTH_EXPANDED:
            return i18n("Expanded");
        case CFontEngine::WIDTH_EXTRA_EXPANDED:
            return i18n("Extra Expanded");
        case CFontEngine::WIDTH_ULTRA_EXPANDED:
            return i18n("Ultra Expanded");
        case CFontEngine::WIDTH_NORMAL:
        case CFontEngine::WIDTH_UNKNOWN:
        default:
            return i18n("Normal");
    }
}

static void addEntry(int face, QString &existing, const QString &add)
{
    if(face>0)
        existing.append(", ");
    existing.append(add);
}

#define KFILEITEM_SUPPORTS_URLS

bool KFileFontPlugin::readInfo(KFileMetaInfo& info, uint what)
{
    QString full,
            family,
            ps,
            foundry,
            weight,
            width,
            spacing,
            slant;

#ifdef KFILEITEM_SUPPORTS_URLS
    KURL    url(info.url());
    QString fName;
    bool    fontsProt  = KIO_FONTS_PROTOCOL == url.protocol(),
            fileProt   = "file"             == url.protocol(),
            downloaded = false,
            status     = false;

    what=0;

    if(!fontsProt && !fileProt && KIO::NetAccess::download(url, fName))
        downloaded=true;
    else
        fName=url.path();

    if(downloaded || fontsProt || fileProt)
    {
#else

    QString fName  = info.path();
    bool    status = false;

    what=0;

#endif
        int face=0,
            numFaces=0;

        do
        {
            if(CGlobal::fe().openKioFont(fName , KFileMetaInfo::Fastest==what
                                                       ? CFontEngine::NAME
                                                       : CFontEngine::NAME|CFontEngine::PROPERTIES|CFontEngine::XLFD, true, face))
            {
                numFaces=CGlobal::fe().getNumFaces(); // Only really for TTC

                addEntry(face, full, CGlobal::fe().getFullName());

                if(KFileMetaInfo::Fastest!=what)
                {
                    addEntry(face, family, CGlobal::fe().getFamilyName());
                    addEntry(face, ps, CGlobal::fe().hasPsInfo() ? CGlobal::fe().getPsName() : i18n("Not Applicable"));
                    if(0==face)
                        foundry=CGlobal::fe().getFoundry();
                    addEntry(face, weight, toStr(CGlobal::fe().getWeight()));
                    addEntry(face, width, toStr(CGlobal::fe().getWidth()));
                    addEntry(face, spacing, toStr(CGlobal::fe().getSpacing()));
                    addEntry(face, slant, toStr(CGlobal::fe().getItalic()));
                }

                CGlobal::fe().closeFont();
                status=true;
            }
        }
        while(++face<numFaces);

        KFileMetaInfoGroup group;

        group=appendGroup(info, "General");
        appendItem(group, "Full", full);

        if(KFileMetaInfo::Fastest!=what)
        {
            appendItem(group, "Family", family);
            appendItem(group, "PostScript", ps);
            appendItem(group, "Foundry", foundry);
            appendItem(group, "Weight", weight);
            appendItem(group, "Width", width);
            appendItem(group, "Spacing", spacing);
            appendItem(group, "Slant", slant);
        }

#ifdef KFILEITEM_SUPPORTS_URLS
        if(downloaded)
            KIO::NetAccess::removeTempFile(fName);
    }
#endif

    return status;
}
