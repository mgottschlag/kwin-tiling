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
#include <list>

static const int constMaxFaces(8); // Max number of faces supported in a TTC...

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

    const char * getName() const
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
    constFamilyNameField=reg.registerField(constFamilyName, FieldRegister::stringType, constMaxFaces, 0);
    constFoundryField=reg.registerField(constFoundry, FieldRegister::stringType, constMaxFaces, 0);
    constWeightField=reg.registerField(constWeight, FieldRegister::stringType, constMaxFaces, 0);
    constWidthField=reg.registerField(constWidth, FieldRegister::stringType, constMaxFaces, 0);
    constSpacingField=reg.registerField(constSpacing, FieldRegister::stringType, constMaxFaces, 0);
    constSlantField=reg.registerField(constSlant, FieldRegister::stringType, constMaxFaces, 0);
    constVersionField=reg.registerField(constVersion, FieldRegister::stringType, constMaxFaces, 0);
}

class Factory : public AnalyzerFactoryFactory
{
    public:

    list<StreamThroughAnalyzerFactory *> getStreamThroughAnalyzerFactories() const
    {
        list<StreamThroughAnalyzerFactory *> af;

        af.push_back(new FontThroughAnalyzerFactory());
        return af;
    }
};

// macro that initializes the Factory when the plugin is loaded
STRIGI_ANALYZER_FACTORY(Factory)

FontThroughAnalyzer::FontThroughAnalyzer(const FontThroughAnalyzerFactory *f)
                   : factory(f)
{
}

jstreams::InputStream * FontThroughAnalyzer::connectInputStream(jstreams::InputStream *in)
{
    //
    // For some reason, when called vie KFileMetaInfo in->getSize() is 0. So, set a maximum size that
    // we want to read in...
    static const int constMaxFileSize=30*1024*1024;

    int                size=in->getSize()>0 ? in->getSize() : constMaxFileSize;
    KUrl               url(analysisResult->path().c_str());
    QString            fName;
    bool               fontsProt  = KFI_KIO_FONTS_PROTOCOL == url.protocol(),
                       fileProt   = "file"                 == url.protocol() || url.protocol().isEmpty(),
                       status     = false;
    CFontEngine::EType type(CFontEngine::getType(analysisResult->path().c_str(), in));

    if( (fontsProt || fileProt) && CFontEngine::TYPE_UNKNOWN!=type )
    {
        CFontEngine fe;
        int         faceFrom(0),
                    faceTo(constMaxFaces);

        const char *d;
        int         n=in->read(d, size, -1);

        in->reset(0);
        if(-2==n)
            return in;

        QByteArray data=QByteArray::fromRawData(d, n);

        for(int face=faceFrom; face<faceTo; ++face)
        {
            if(fe.openFont(type, data, analysisResult->path().c_str(), face))
            {
                add(fe.getFamilyName(), fe.getFoundry(),
                    KFI::FC::weightStr(fe.getWeight(), false), KFI::FC::widthStr(fe.getWidth(), false),
                    KFI::FC::spacingStr(fe.getSpacing()), KFI::FC::slantStr(fe.getItalic(), false), fe.getVersion());
                status=true;
                if(CFontEngine::TYPE_TTC!=type || face>=fe.getNumFaces())
                    break;
            }
            else
                break;
        }

        if(status)
            switch(type)
            {
                case CFontEngine::TYPE_OTF:
                    analysisResult->setMimeType("application/x-font-otf");
                    break;
                case CFontEngine::TYPE_TTF:
                case CFontEngine::TYPE_TTC:
                    analysisResult->setMimeType("application/x-font-ttf");
                    break;
                case CFontEngine::TYPE_TYPE1:
                    analysisResult->setMimeType("application/x-font-type1");
                    break;
                case CFontEngine::TYPE_PCF:
                    analysisResult->setMimeType("application/x-font-pcf");
                    break;
                case CFontEngine::TYPE_BDF:
                    analysisResult->setMimeType("application/x-font-bdf");
                    break;
                case CFontEngine::TYPE_AFM:
                    analysisResult->setMimeType("application/x-font-afm");
                default:
                    break;
            }
    }

    return in;
}

void FontThroughAnalyzer::add(const QString &family, const QString &foundry, const QString &weight, const QString &width,
                              const QString &spacing, const QString &slant,  const QString &version)
{
    analysisResult->setField(factory->constFoundryField, foundry.isEmpty()
                                    ? (const char *)i18n(KFI_UNKNOWN_FOUNDRY).toUtf8()
                                    : (const char *)foundry.toUtf8());
    analysisResult->setField(factory->constFamilyNameField, (const char *)family.toUtf8());
    analysisResult->setField(factory->constWeightField, (const char *)weight.toUtf8());
    analysisResult->setField(factory->constWidthField, (const char *)width.toUtf8());
    analysisResult->setField(factory->constSpacingField, (const char *)spacing.toUtf8());
    analysisResult->setField(factory->constSlantField, (const char *)slant.toUtf8());
    if(!version.isEmpty())
        analysisResult->setField(factory->constVersionField, (const char *)version.toUtf8());
}

