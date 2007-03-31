/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2007 Craig Drummond <craig@kde.org>
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

#define STRIGI_IMPORT_API
#include <strigi/streamthroughanalyzer.h>
#include <strigi/analyzerplugin.h>
#include <strigi/fieldtypes.h>
#include <strigi/analysisresult.h>
#include "FontThroughAnalyzer.h"
#include "Fc.h"
#include "KfiConstants.h"
#include "Misc.h"
#include "FontEngine.h"
#include <QByteArray>
#include <QFile>
#include <kio/netaccess.h>
#include <list>

using namespace Strigi;
using namespace std;
using namespace KFI;

class FontThroughAnalyzerFactory : public StreamThroughAnalyzerFactory
{
    friend class FontThroughAnalyzer;

    public:

    static const cnstr      constFamilyName;
    const RegisteredField * constFamilyNameField;
    static const cnstr      constFoundry;
    const RegisteredField * constFoundryField;
    static const cnstr      constWeight;
    const RegisteredField * constWeightField;
    static const cnstr      constWidth;
    const RegisteredField * constWidthField;
    static const cnstr      constSpacing;
    const RegisteredField * constSpacingField;
    static const cnstr      constSlant;
    const RegisteredField * constSlantField;
    static const cnstr      constVersion;
    const RegisteredField * constVersionField;

    const char * name() const
    {
        return "FontThroughAnalyzer";
    }

    StreamThroughAnalyzer * newInstance() const
    {
        return new FontThroughAnalyzer(this);
    }

    void registerFields(FieldRegister &reg);
};

const cnstr FontThroughAnalyzerFactory::constFamilyName("Family");
const cnstr FontThroughAnalyzerFactory::constFoundry("Foundry");
const cnstr FontThroughAnalyzerFactory::constWeight("Weight");
const cnstr FontThroughAnalyzerFactory::constWidth("Width");
const cnstr FontThroughAnalyzerFactory::constSpacing("Spacing");
const cnstr FontThroughAnalyzerFactory::constSlant("Slant");
const cnstr FontThroughAnalyzerFactory::constVersion("Version");

void FontThroughAnalyzerFactory::registerFields(FieldRegister &reg)
{
    constFamilyNameField=reg.registerField(constFamilyName, FieldRegister::stringType, 1, 0);
    constWeightField=reg.registerField(constWeight, FieldRegister::stringType, 1, 0);
    constSlantField=reg.registerField(constSlant, FieldRegister::stringType, 1, 0);
    constWidthField=reg.registerField(constWidth, FieldRegister::stringType, 1, 0);
    constSpacingField=reg.registerField(constSpacing, FieldRegister::stringType, 1, 0);
    constFoundryField=reg.registerField(constFoundry, FieldRegister::stringType, 1, 0);
    constVersionField=reg.registerField(constVersion, FieldRegister::stringType, 1, 0);
}

class Factory : public AnalyzerFactoryFactory
{
    public:

    list<StreamThroughAnalyzerFactory *> streamThroughAnalyzerFactories() const
    {
        list<StreamThroughAnalyzerFactory *> af;

        af.push_back(new FontThroughAnalyzerFactory());
        return af;
    }
};

// macro that initializes the Factory when the plugin is loaded
STRIGI_ANALYZER_FACTORY(Factory)

static QString getFamily(const QString &font)
{
    int     commaPos=font.lastIndexOf(',');
    return -1==commaPos ? font : font.left(commaPos);
}

static QString toMime(CFontEngine::EType type)
{
    switch(type)
    {
        case CFontEngine::TYPE_OTF:
            return "application/x-font-otf";
        case CFontEngine::TYPE_TTF:
        case CFontEngine::TYPE_TTC:
            return "application/x-font-ttf";
        case CFontEngine::TYPE_TYPE1:
            return "application/x-font-type1";
        case CFontEngine::TYPE_PCF:
            return "application/x-font-pcf";
        case CFontEngine::TYPE_BDF:
            return "application/x-font-bdf";
        case CFontEngine::TYPE_AFM:
            return "application/x-font-afm";
        default:
            break;
    }

    return QString();
}

FontThroughAnalyzer::FontThroughAnalyzer(const FontThroughAnalyzerFactory *f)
                   : factory(f)
{
}

InputStream * FontThroughAnalyzer::connectInputStream(InputStream *in)
{
    KUrl    url(analysisResult->path().c_str());
    bool    fontsProt = KFI_KIO_FONTS_PROTOCOL == url.protocol(),
            fileProt  = "file"                 == url.protocol() || url.protocol().isEmpty();
    int     face(0);

    if( fontsProt )
    {
        // OK, its a fonts:/ url - are we passed any data in the query?
        QString      path=url.queryItem(KFI_FILE_QUERY),
                     name=url.queryItem(KFI_NAME_QUERY),
                     mime=url.queryItem(KFI_MIME_QUERY);
        unsigned int styleInfo=Misc::getIntQueryVal(url, KFI_STYLE_QUERY, KFI_NO_STYLE_INFO);

        face=Misc::getIntQueryVal(url, KFI_KIO_FACE, -1);

        if(name.isEmpty() && path.isEmpty())
        {
            // OK, no useable info in the query - stat fonts:/ to get the required info...
            KIO::UDSEntry udsEntry;

            if(KIO::NetAccess::stat(url, udsEntry, NULL))
            {
                if(udsEntry.numberValue(KIO::UDS_HIDDEN, 0))
                {
                    path=udsEntry.stringValue(UDS_EXTRA_FILE_NAME);
                    face=Misc::getIntQueryVal(url, KFI_KIO_FACE, 0);
                }
                else
                {
                    name=udsEntry.stringValue(KIO::UDS_NAME);
                    styleInfo=FC::styleValFromStr(udsEntry.stringValue(UDS_EXTRA_FC_STYLE));
                }
                mime=udsEntry.stringValue(KIO::UDS_MIME_TYPE);
            }
        }

        if(path.isNull())
        {
            // Enabled font...
            if(!name.isNull())
            {
                int weight, width, slant;

                FC::decomposeStyleVal(styleInfo, weight, width, slant);
                name=getFamily(name);

                FcObjectSet *os  = FcObjectSetBuild(FC_SPACING, FC_FOUNDRY, FC_FONTVERSION, (void *)0);
                FcPattern   *pat = FcPatternBuild(NULL,
                                                    FC_FAMILY, FcTypeString,
                                                    (const FcChar8 *)(name.toUtf8().data()),
                                                    FC_WEIGHT, FcTypeInteger, weight,
                                                    FC_SLANT, FcTypeInteger, slant,
#ifndef KFI_FC_NO_WIDTHS
                                                    FC_WIDTH, FcTypeInteger, width,
#endif
                                                    NULL);

                FcFontSet   *set = FcFontList(0, pat, os);

                FcPatternDestroy(pat);
                FcObjectSetDestroy(os);

                if(set && set->nfont)
                {
                    int     spacing,
                            version;
                    QString foundry(FC::getFcString(set->fonts[0], FC_FOUNDRY, 0)),
                            versionStr;

                    FcPatternGetInteger(set->fonts[0], FC_SPACING, 0, &spacing);
                    FcPatternGetInteger(set->fonts[0], FC_FONTVERSION, 0, &version);

                    if(version)
                        versionStr.setNum(CFontEngine::decodeFixed(version));

                    result(name, CFontEngine::fixFoundry(foundry),
                           KFI::FC::weightStr(weight, false), KFI::FC::widthStr(width, false),
                           KFI::FC::spacingStr(spacing), KFI::FC::slantStr(slant, false),
                           versionStr, mime);
                }
            }
        }
        else // Its a disabled font, so read file...
        {
            CFontEngine fe;
            QByteArray  data;

            if(fe.openFont(CFontEngine::TYPE_UNKNOWN, data,
                           QFile::encodeName(path).constData(), face) &&
               !fe.getFamilyName().isEmpty())
                result(fe.getFamilyName(), fe.getFoundry(),
                       KFI::FC::weightStr(fe.getWeight(), false),
                       KFI::FC::widthStr(fe.getWidth(), false),
                       KFI::FC::spacingStr(fe.getSpacing()),
                       KFI::FC::slantStr(fe.getItalic(), false), fe.getVersion(), mime);
        }
    }

    if(fileProt)
    {
        CFontEngine::EType type(CFontEngine::getType(analysisResult->path().c_str(), in));

        if(CFontEngine::TYPE_UNKNOWN!=type)
        {
            // For some reason, when called vie KFileMetaInfo in->size() is 0. So, set a maximum
            // size that we want to read in...
            static const int constMaxFileSize=30*1024*1024;

            int        size=in->size()>0 ? in->size() : constMaxFileSize;
            const char *d;
            int        n=in->read(d, size, -1);

            in->reset(0);
            if(-2==n)
                return in;

            CFontEngine fe;
            QByteArray  data=QByteArray::fromRawData(d, n);

            face=Misc::getIntQueryVal(url, KFI_KIO_FACE, 0);

            if(fe.openFont(type, data, analysisResult->path().c_str(), face) &&
               !fe.getFamilyName().isEmpty())
                result(fe.getFamilyName(), fe.getFoundry(),
                       KFI::FC::weightStr(fe.getWeight(), false),
                       KFI::FC::widthStr(fe.getWidth(), false),
                       KFI::FC::spacingStr(fe.getSpacing()),
                       KFI::FC::slantStr(fe.getItalic(), false),
                       fe.getVersion(), toMime(type));
        }
    }

    return in;
}

void FontThroughAnalyzer::result(const QString &family, const QString &foundry, const QString &weight,
                                 const QString &width, const QString &spacing, const QString &slant,
                                 const QString &version, const QString &mime)
{
    analysisResult->addValue(factory->constFamilyNameField, (const char *)family.toUtf8());
    analysisResult->addValue(factory->constWeightField, (const char *)weight.toUtf8());
    analysisResult->addValue(factory->constSlantField, (const char *)slant.toUtf8());
    analysisResult->addValue(factory->constWidthField, (const char *)width.toUtf8());
    analysisResult->addValue(factory->constSpacingField, (const char *)spacing.toUtf8());
    analysisResult->addValue(factory->constFoundryField, foundry.isEmpty()
                                    ? (const char *)i18n(KFI_UNKNOWN_FOUNDRY).toUtf8()
                                    : (const char *)foundry.toUtf8());
    if(!version.isEmpty())
        analysisResult->addValue(factory->constVersionField, (const char *)version.toUtf8());

    analysisResult->setMimeType((const char *)mime.toUtf8());
}

